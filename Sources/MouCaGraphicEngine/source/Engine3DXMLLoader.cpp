#include "Dependencies.h"

#include "MouCaGraphicEngine/include/Engine3DXMLLoader.h"
#include "MouCaGraphicEngine/include/Engine3DXMLHelper.h"

#include <LibCore/include/CoreFileTracker.h>

#include <LibRT/include/RTImage.h>
#include <LibRT/include/RTMonitor.h>
#include <LibRT/include/RTRenderDialog.h>
#include <LibRT/include/RTShaderFile.h>

#include <LibGLFW/include/GLFWWindow.h>

#include <LibVR/include/VRPlatform.h>

#include <LibVulkan/include/VKAccelerationStructure.h>
#include <LibVulkan/include/VKAccelerationStructureGeometry.h>

#include <LibVulkan/include/VKCommand.h>
#include <LibVulkan/include/VKCommandBufferSurface.h>
#include <LibVulkan/include/VKContextDevice.h>
#include <LibVulkan/include/VKContextWindow.h>
#include <LibVulkan/include/VKDescriptorSet.h>
#include <LibVulkan/include/VKFence.h>
#include <LibVulkan/include/VKFrameBuffer.h>
#include <LibVulkan/include/VKGraphicsPipeline.h>
#include <LibVulkan/include/VKImage.h>
#include <LibVulkan/include/VKPipelineLayout.h>
#include <LibVulkan/include/VKPipelineStates.h>
#include <LibVulkan/include/VKRayTracingPipeline.h>
#include <LibVulkan/include/VKSampler.h>
#include <LibVulkan/include/VKSemaphore.h>
#include <LibVulkan/include/VKSequence.h>
#include <LibVulkan/include/VKSubmitInfo.h>
#include <LibVulkan/include/VKSwapChain.h>
#include <LibVulkan/include/VKTracingRay.h>

#include <LibXML/include/XML.h>

#include "MouCaGraphicEngine/include/GraphicEngine.h"
#include "MouCaGraphicEngine/include/VulkanEnum.h"
#include "MouCaGraphicEngine/include/VulkanManager.h"

namespace MouCaGraphic
{
    
Engine3DXMLLoader::ContextLoading::ContextLoading(GraphicEngine& engine, XML::Parser& parser, MouCaCore::ResourceManager& resources):
_engine(engine), _parser(parser), _resources(resources), _xmlFileName(Core::convertToU8(parser.getFilename()))
{}

//-----------------------------------------------------------------------------------------
//                                  Load / Save engine
//-----------------------------------------------------------------------------------------

Engine3DXMLLoader::~Engine3DXMLLoader()
{}

void Engine3DXMLLoader::load(ContextLoading& context)
{
    MOUCA_PRE_CONDITION(context._parser.isLoaded()); //DEV Issue: Need a valid xml.

    //Read Engine part
    auto result = context._parser.getNode(u8"Engine3D");
    if (result->getNbElements() == 0)
    {
        MOUCA_THROW_ERROR_2(u8"Engine3D", u8"LoadingXMLCorruptError", context.getFileName(), u8"Engine3D");
    }

    XML::NodeUPtr engineNode = result->getNode(0);
    float version;
    engineNode->getAttribute(u8"version", version);

    // Check version before continue
    if (VulkanManager::_version < version)
    {
        MOUCA_THROW_ERROR_2(u8"Engine3D", u8"LoadingXMLVersionError", context.getFileName(), std::to_string(version));
    }

    //Go to node
    // cppcheck-suppress unreadVariable // false positive
    auto aPush = context._parser.autoPushNode(*engineNode);

    // Check if we need to load environment
    if (_manager.getEnvironment().isNull())
    {
        loadEnvironment(context);
    }

    // Prepare global data node
    auto datas = context._parser.getNode(u8"GlobalData");
    if (datas->getNbElements() > 0)
    {
        MOUCA_ASSERT(datas->getNbElements() == 1);
        context._globalData = datas->getNode(0);
    }

    // Prepare all surfaces
    // Build new window
    loadWindows(context);

    // Build new VR
    loadVR(context);

    // Create device if possible
    loadDevices(context);
}

void Engine3DXMLLoader::loadWindows(ContextLoading& context)
{
    MOUCA_PRE_CONDITION(_dialogs.empty());

    auto result = context._parser.getNode(u8"Window");
    for (size_t idWindow = 0; idWindow < result->getNbElements(); ++idWindow)
    {
        auto window = result->getNode(idWindow);
        bool existing;
        const uint32_t id = LoaderHelper::getIdentifiant(window, u8"Window", _dialogs, context, existing);
        if (existing)
            continue;

        // Title
        Core::String title;
        window->getAttribute(u8"title", title);

        // Mode
        bool state = false;
        window->getAttribute(u8"visible", state);
        auto mode = (state ? RT::Window::Visible : RT::Window::Unknown);

        // Viewport
        if (window->hasAttribute(u8"monitor"))
        {
            size_t idMonitor = 0;
            window->getAttribute(u8"monitor", idMonitor);
            if (idMonitor >= context._engine.getMonitors().size())
            {
                MOUCA_THROW_ERROR_2(u8"Engine3D", u8"WindowUnknownMonitorError", context.getFileName(), std::to_string(idMonitor));
            }

            window->getAttribute(u8"fullscreen", state);
            mode = static_cast<RT::Window::Mode>(mode | (state ? RT::Window::Resizable : RT::Window::Unknown));

            // Build dialog
            _dialogs[id] = context._engine.getRTPlatform().createWindow(context._engine.getMonitors()[idMonitor], title, mode);
        }
        else
        {
            int32_t posX = 0, posY = 0;
            RT::ViewportInt32 viewport;
            window->getAttribute(u8"positionX", posX);
            window->getAttribute(u8"positionY", posY);
            viewport.setOffset(posX, posY);

            LoaderHelper::readAttribute(window, u8"width",  posX, context);
            LoaderHelper::readAttribute(window, u8"height", posY, context);
            viewport.setSize(posX, posY);

            // Validate window
            bool valid = false;
            for (const RT::Monitor& screen : context._engine.getMonitors())
            {
                valid = screen.getMonitorArea().isInside(viewport.getOffset());
                if (valid)
                    break;
            }

            if (!valid)
            {
                MOUCA_THROW_ERROR_2(u8"Engine3D", u8"WindowCreationOutsideError", context.getFileName(), u8"La position de la fenêtre n'est pas dans un écran.");
            }

            // Mode
            window->getAttribute(u8"resizable", state);
            mode = static_cast<RT::Window::Mode>(mode | (state ? RT::Window::Resizable : RT::Window::Unknown));
            window->getAttribute(u8"border", state);
            mode = static_cast<RT::Window::Mode>(mode | (state ? RT::Window::Border : RT::Window::Unknown));

            // Build dialog
            _dialogs[id] = context._engine.getRTPlatform().createWindow(viewport, title, mode);
        }

        MOUCA_ASSERT(!_dialogs[id].expired());

        // Register dialog
        _manager.addRenderDialog(_dialogs[id]);
    }
}

void Engine3DXMLLoader::loadVR(ContextLoading& context)
{
    auto result = context._parser.getNode(u8"VRHeadset");

    // Initialize VR
    if(result->getNbElements()==1)
    {
        auto& vrPlatform = context._engine.getVRPlatform();
        if (vrPlatform.isNull())
        {
            vrPlatform.initialize();
        }
    }
}

void Engine3DXMLLoader::loadSurfaces(ContextLoading& context, Vulkan::ContextDeviceWPtr device)
{
    MOUCA_PRE_CONDITION(!device.expired()); //DEV Issue: Bad device !

    // Read surfaces
    auto result = context._parser.getNode(u8"Surfaces");
    if (result->getNbElements() > 0)
    {
        MOUCA_ASSERT(!_dialogs.empty());            //DEV Issue: Create surface/window without window ?
        MOUCA_ASSERT(result->getNbElements() == 1); //DEV Issue: please clean xml ?

        auto aPushS = context._parser.autoPushNode(*result->getNode(0));

        auto allSurfaces = context._parser.getNode(u8"Surface");
        for (size_t idSurface = 0; idSurface < allSurfaces->getNbElements(); ++idSurface)
        {
            auto surface = allSurfaces->getNode(idSurface);

            // Mandatory attribute
            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(surface, u8"Surface", _surfaces, context, existing);
            if (existing)
            {
                continue;
            }

            uint32_t windowId = 0;
            surface->getAttribute(u8"windowId", windowId);
            if (_dialogs.find(windowId) == _dialogs.cend()) //DEV Issue: Must exist
            {
                MOUCA_THROW_ERROR_1(u8"Engine3D", u8"UnknownSurfaceError", context.getFileName());
            }

            // Read user preferences
            Vulkan::SurfaceFormat::Configuration configuration;

            {
                auto aPush = context._parser.autoPushNode(*surface);

                auto allPreferences = context._parser.getNode(u8"UserPreferences");                
                for(size_t idPreference = 0; idPreference < allPreferences->getNbElements(); ++idPreference)
                {
                    auto preferences = allPreferences->getNode(idPreference);
                    // Read current config but keep only is OS is matching
                    if (preferences->hasAttribute("os"))
                    {
                        Core::String os;
                        preferences->getAttribute("os", os);

                        if( os != VulkanManager::_osName)
                            continue;
                    }
                    
                    if(preferences->hasAttribute(u8"presentationMode"))
                    {   
                        configuration._presentMode = LoaderHelper::readValue(preferences, u8"presentationMode", presentModes, false, context);
                    }
                    if(preferences->hasAttribute(u8"usage"))
                    {
                        configuration._usage = LoaderHelper::readValue(preferences, u8"usage", imageUsages, true, context);
                    }
                    if (preferences->hasAttribute(u8"transform"))
                    {
                        configuration._transform = LoaderHelper::readValue(preferences, u8"transform", surfaceTransforms, true, context);
                    }
                    if (preferences->hasAttribute(u8"format"))
                    {
                        configuration._format.format = LoaderHelper::readValue(preferences, u8"format", formats, false, context);
                    }
                    if (preferences->hasAttribute(u8"colorSpace"))
                    {
                        configuration._format.colorSpace = LoaderHelper::readValue(preferences, u8"colorSpace", colorSpaces, false, context);
                    }
                }
            }

            // Build surface
            _surfaces[id] = _manager.createWindowSurface(device, windowId, configuration);
        }
    }
}

void Engine3DXMLLoader::loadSemaphores(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak)
{
    MOUCA_PRE_CONDITION(!deviceWeak.expired()); //DEV Issue: Bad device !

    // Read semaphores
    auto result = context._parser.getNode(u8"Semaphores");
    if (result->getNbElements() > 0)
    {
        MOUCA_ASSERT(result->getNbElements() == 1); //DEV Issue: please clean xml ?

        // cppcheck-suppress unreadVariable // false positive
        auto aPush = context._parser.autoPushNode(*result->getNode(0));

        auto device = deviceWeak.lock();

        auto allSemaphores = context._parser.getNode(u8"Semaphore");
        for (size_t idSemaphore = 0; idSemaphore < allSemaphores->getNbElements(); ++idSemaphore)
        {
            auto semaphoreNode = allSemaphores->getNode(idSemaphore);
            // Mandatory attribute
            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(semaphoreNode, u8"Semaphore", _semaphores, context, existing);

            auto semaphore = std::make_shared<Vulkan::Semaphore>();
            semaphore->initialize(device->getDevice());

            // Register + ownership
            device->insertSemaphore(semaphore);

            _semaphores[id] = semaphore;
        }
    }
}

void Engine3DXMLLoader::loadFences(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak)
{
    MOUCA_PRE_CONDITION(!deviceWeak.expired()); //DEV Issue: Bad device !

    // Read semaphores
    auto result = context._parser.getNode(u8"Fences");
    if (result->getNbElements() > 0)
    {
        MOUCA_ASSERT(result->getNbElements() == 1); //DEV Issue: please clean xml ?

        // cppcheck-suppress unreadVariable // false positive
        auto aPush = context._parser.autoPushNode(*result->getNode(0));

        auto device = deviceWeak.lock();

        auto allFences = context._parser.getNode(u8"Fence");
        for (size_t idFence = 0; idFence < allFences->getNbElements(); ++idFence)
        {
            auto semaphoreNode = allFences->getNode(idFence);
            // Mandatory attribute
            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(semaphoreNode, u8"Fence", _fences, context, existing);

            const VkFenceCreateFlags flag = LoaderHelper::readValue(semaphoreNode, u8"flag", fenceCreates, true, context);

            auto fence = std::make_shared<Vulkan::Fence>();
            fence->initialize(device->getDevice(), flag);

            // Register + ownership
            device->insertFence(fence);

            _fences[id] = fence;
        }
    }
}

void Engine3DXMLLoader::loadFrameBuffers(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak)
{
    MOUCA_PRE_CONDITION(!deviceWeak.expired()); //DEV Issue: Bad device !

    auto result = context._parser.getNode(u8"FrameBuffers");
    if (result->getNbElements() > 0)
    {
        MOUCA_ASSERT(result->getNbElements() == 1); //DEV Issue: please clean xml ?

        auto device = deviceWeak.lock();

        auto aPush = context._parser.autoPushNode(*result->getNode(0));

        auto allFrameBuffers = context._parser.getNode(u8"FrameBuffer");
        for (size_t idFrameBuffer = 0; idFrameBuffer < allFrameBuffers->getNbElements(); ++idFrameBuffer)
        {
            auto frameBufferNode = allFrameBuffers->getNode(idFrameBuffer);
            // Search if FrameBuffer is existing before try to load
            uint32_t id=0;
            if (frameBufferNode->hasAttribute(u8"id") || frameBufferNode->hasAttribute(u8"externalId"))
            {
                bool existing;
                id = LoaderHelper::getIdentifiant(frameBufferNode, u8"FrameBuffer", _frameBuffers, context, existing);
                if (existing)
                {
                    continue;
                }
            }

            // Read size from image
            VkExtent2D resolution = {};
            Vulkan::ContextWindow::ViewAttachments attachments;
            // Load Attachments
            {
                auto aPushA = context._parser.autoPushNode(*frameBufferNode);

                auto allAttachments = context._parser.getNode(u8"Attachment");
                for (size_t idAttachment = 0; idAttachment < allAttachments->getNbElements(); ++idAttachment)
                {
                    auto attachmentNode = allAttachments->getNode(idAttachment);
                    if (attachmentNode->hasAttribute(u8"viewImageId"))
                    {
                        const uint32_t viewImageId = LoaderHelper::getLinkedIdentifiant(attachmentNode, u8"viewImageId", _view, context);

                        // Read view information
                        auto view = _view[viewImageId].lock();
                        attachments.emplace_back(view);
                        resolution =
                        {
                            view->getImage().getExtent().width,
                            view->getImage().getExtent().height
                        };
                    }
                    else // Add empty view
                    {
                        attachments.emplace_back(Vulkan::ImageViewWPtr());
                    }
                }
            }

            if (attachments.empty())
            {
                MOUCA_THROW_ERROR_1(u8"Engine3D", u8"XMLFrameBufferMissAttachmentError", context.getFileName());
            }

            // Mandatory attribute
            const uint32_t renderPassId = LoaderHelper::getLinkedIdentifiant(frameBufferNode, u8"renderPassId", _renderPasses, context);
            
            // Build SwapChain framebuffer
            if (frameBufferNode->hasAttribute(u8"surfaceId"))
            {
                MOUCA_ASSERT(!frameBufferNode->hasAttribute(u8"id")); //DEV Issue: Not supported id !

                // Get attached surface
                const uint32_t surfaceId = LoaderHelper::getLinkedIdentifiant(frameBufferNode, u8"surfaceId", _renderPasses, context);

                // Build FrameBuffer inside surface
                _surfaces[surfaceId].lock()->createFrameBuffer(_renderPasses[renderPassId], attachments);
            }
            else // Build basic Framebuffer
            {
                MOUCA_ASSERT(frameBufferNode->hasAttribute(u8"id")); //DEV Issue: New case ?

                Vulkan::FrameBuffer::Attachments allAttachments;
                allAttachments.resize(attachments.size());
                std::transform(attachments.begin(), attachments.end(), allAttachments.begin(),
                               [](const auto& attachment) { return attachment.lock()->getInstance(); });
                
                // Build Frame Buffer
                auto frameBuffer = std::make_shared<Vulkan::FrameBuffer>();
                frameBuffer->initialize(device->getDevice(), _renderPasses[renderPassId], resolution, allAttachments);

                // Register + ownership
                device->insertFrameBuffer(frameBuffer);
                _frameBuffers[id] = frameBuffer;
            }
        }
    }
}


void Engine3DXMLLoader::loadImagesAndView(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak)
{
    MOUCA_PRE_CONDITION(!deviceWeak.expired()); //DEV Issue: Bad device !

    // Search Images
    auto result = context._parser.getNode(u8"Images");
    if (result->getNbElements() > 0)
    {
        MOUCA_ASSERT(result->getNbElements() == 1); //DEV Issue: please clean xml ?
        auto device = deviceWeak.lock();

        auto aPushI = context._parser.autoPushNode(*result->getNode(0));

        // Parsing all images
        auto allImages = context._parser.getNode(u8"Image");
        for (size_t idImages = 0; idImages < allImages->getNbElements(); ++idImages)
        {
            auto imageNode = allImages->getNode(idImages);

            // Mandatory attribute
            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(imageNode, u8"Image", _images, context, existing);

            Vulkan::Image::Size size;
            VkFormat format = VK_FORMAT_UNDEFINED;
            VkImageType type = VK_IMAGE_TYPE_MAX_ENUM;
            VkImageUsageFlags usage = LoaderHelper::readValue(imageNode, u8"usage", imageUsages , true, context);

            // Build image from surface/swapchain size/format
            if (imageNode->hasAttribute("fromSurfaceId"))
            {
                const uint32_t idSC = LoaderHelper::getLinkedIdentifiant(imageNode, u8"fromSurfaceId", _surfaces, context);

                auto surface = _surfaces[idSC].lock();
                const auto& config = surface->getFormat().getConfiguration();

                size._extent.width  = config._extent.width;
                size._extent.height = config._extent.height;

                // Compute default format
                format = (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
                       ? device->getDevice().getDepthFormat() // Best depth/Stencil format
                       : config._format.format;
            }
            else if (imageNode->hasAttribute("fromVRId"))
            {
                const auto& vrPlatform = context._engine.getVRPlatform();
                MOUCA_ASSERT(!vrPlatform.isNull());
                //const uint32_t idSC = LoaderHelper::getLinkedIdentifiant(imageNode, u8"fromVRId", _surfaces, context);

//                 auto surface = _surfaces[idSC].lock();
//                 const auto& config = surface->getFormat().getConfiguration();

                const auto renderSize = vrPlatform.getRenderSize();
                size._extent.width  = renderSize[0];
                size._extent.height = renderSize[1];

                // Compute default format
                format = (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
                    ? device->getDevice().getDepthFormat() // Best depth/Stencil format
                    : VK_FORMAT_R8G8B8A8_SRGB;
            }
            else if (imageNode->hasAttribute("external"))
            {
                const uint32_t idSC = LoaderHelper::getLinkedIdentifiant(imageNode, u8"external", _cpuImages, context);
                auto image = _cpuImages[idSC].lock();

                size._extent =
                {
                    image->getExtents(0).x,
                    image->getExtents(0).y,
                    image->getExtents(0).z
                };
                size._arrayLayers = image->getLayers();
                size._mipLevels   = image->getLevels();
                switch (image->getTarget())
                {
                    case RT::Image::Target::Type1D:
                    case RT::Image::Target::Type1DArray:
                    {
                        type = VK_IMAGE_TYPE_1D;
                    }
                    break;
                    case RT::Image::Target::Type2D:
                    case RT::Image::Target::Type2DArray:
                    {
                        type = VK_IMAGE_TYPE_2D;
                    }
                    break;
                    case RT::Image::Target::Type3D:
                    {
                        type = VK_IMAGE_TYPE_3D;
                    }
                    break;
                    default: MOUCA_ASSERT(false); // DEV Issue: Not implemented
                }
            }

            // Search format/size if not properly define or override
            if( imageNode->hasAttribute(u8"extent") || !size.isValid() )
            {
                MOUCA_ASSERT(context._globalData != nullptr);
                // Read name
                Core::String name;
                imageNode->getAttribute(u8"extent", name);

                // Extract data
                auto resultExt = context._parser.searchNodeFrom(*context._globalData, u8"Data", u8"name", name.substr(1));
                LoaderHelper::readData(resultExt, size._extent);
            }
            if (imageNode->hasAttribute(u8"format") || format == VK_FORMAT_UNDEFINED)
            {
                format = LoaderHelper::readValue(imageNode, u8"format", formats, false, context);
            }
            if (imageNode->hasAttribute(u8"imageType") || type == VK_IMAGE_TYPE_MAX_ENUM)
            {
                type = LoaderHelper::readValue(imageNode, u8"imageType", imageTypes, false, context);
            }
            
            MOUCA_ASSERT(format != VK_FORMAT_UNDEFINED);   // DEV Issue: Need a valid format.
            MOUCA_ASSERT(size.isValid());                  // DEV Issue: Need a valid size.

            auto image = std::make_shared<Vulkan::Image>();
            image->initialize(device->getDevice(),
                              size, type, format,
                              LoaderHelper::readValue(imageNode, u8"samples",       samples, false, context),
                              LoaderHelper::readValue(imageNode, u8"tiling",        tilings, false, context),
                              usage,
                              LoaderHelper::readValue(imageNode, u8"sharingMode",   sharingModes, false, context),
                              LoaderHelper::readValue(imageNode, u8"initialLayout", imageLayouts, false, context),
                              LoaderHelper::readValue(imageNode, u8"memoryProperty", memoryProperties, true, context)
                             );
            // Register + ownership
            device->insertImage(image);
            _images[id] = image;
            MOUCA_DEBUG("Image: id=" << id << ", handle=" << image->getImage());

            // Build view
            auto aPush = context._parser.autoPushNode(*imageNode);

            auto allViews = context._parser.getNode(u8"View");
            for(size_t idView = 0; idView <allViews->getNbElements(); ++idView)
            {
                auto viewNode = allViews->getNode(idView);

                // Mandatory attribute
                const uint32_t idV = LoaderHelper::getIdentifiant(viewNode, u8"View", _view, context, existing);

                // Read parameter
                const VkImageViewType typeV   = LoaderHelper::readValue(viewNode, u8"viewType", viewTypes, false, context);
                const VkFormat        formatV = LoaderHelper::readValue(viewNode, u8"format",   formats, false, context);

                const VkComponentMapping mapping
                {
                    LoaderHelper::readValue(viewNode, u8"componentSwizzleRed",   componentSwizzles, false, context),
                    LoaderHelper::readValue(viewNode, u8"componentSwizzleGreen", componentSwizzles, false, context),
                    LoaderHelper::readValue(viewNode, u8"componentSwizzleBlue",  componentSwizzles, false, context),
                    LoaderHelper::readValue(viewNode, u8"componentSwizzleAlpha", componentSwizzles, false, context)
                };
                VkImageSubresourceRange subRessourceRange;
                subRessourceRange.aspectMask = LoaderHelper::readValue(viewNode, u8"aspectMask", aspectFlags, true, context);
                viewNode->getAttribute(u8"baseMipLevel",   subRessourceRange.baseMipLevel);
                viewNode->getAttribute(u8"levelCount",     subRessourceRange.levelCount);
                viewNode->getAttribute(u8"baseArrayLayer", subRessourceRange.baseArrayLayer);
                viewNode->getAttribute(u8"layerCount",     subRessourceRange.layerCount);

                // Build View
                auto view = image->createView(device->getDevice(), typeV, formatV, mapping, subRessourceRange);
                _view[idV] = view;

                MOUCA_DEBUG("View: id=" << idV << ", handle=" << view.lock()->getInstance());
            }
        }
    }
}

void Engine3DXMLLoader::loadSamplers(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak)
{
    MOUCA_PRE_CONDITION(!deviceWeak.expired()); //DEV Issue: Bad device !

    // Search Buffers
    auto result = context._parser.getNode(u8"Samplers");
    if (result->getNbElements() > 0)
    {
        MOUCA_ASSERT(result->getNbElements() == 1); //DEV Issue: please clean xml ?
        auto device = deviceWeak.lock();

        auto aPushS = context._parser.autoPushNode(*result->getNode(0));

        // Parsing all samplers
        auto allSamplers = context._parser.getNode(u8"Sampler");
        for (size_t idSampler = 0; idSampler < allSamplers->getNbElements(); ++idSampler)
        {
            auto samplerNode = allSamplers->getNode(idSampler);

            // Mandatory attribute
            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(samplerNode, u8"Sampler", _samplers, context, existing);
            if (existing)
            {
                continue;
            }

            // Read max LOD from image
            float maxLod = 0.0f;
            if (samplerNode->hasAttribute(u8"imageId"))
            {
                const uint32_t idI = LoaderHelper::getLinkedIdentifiant(samplerNode, u8"imageId", _images, context);
                maxLod = static_cast<float>(_images[idI].lock()->getMaxMipLevels());
            }

            float mipLodBias;
            samplerNode->getAttribute(u8"mipLodBias", mipLodBias);
            float minLod = 0.0f;
            samplerNode->getAttribute(u8"minLod", minLod);
            if(samplerNode->hasAttribute(u8"maxLod"))
            {
                samplerNode->getAttribute(u8"maxLod", maxLod);
            }            
            bool unnormalizedCoordinates;
            samplerNode->getAttribute(u8"unnormalizedCoordinates", unnormalizedCoordinates);

            // Read compare
            bool compareEnable = false;
            VkCompareOp compareOp = VK_COMPARE_OP_NEVER;
            if (samplerNode->hasAttribute(u8"compareOp"))
            {
                compareEnable = true;
                compareOp = LoaderHelper::readValue(samplerNode, u8"compareOp", compareOperations, false, context);
            }            
            
            //Read anisotropy
            bool anisotropyEnable = false;
            float maxAnisotropy = 1.0f;
            if (samplerNode->hasAttribute(u8"maxAnisotropy"))
            {
                anisotropyEnable = true;

                Core::String isDevice;
                samplerNode->getAttribute(u8"maxAnisotropy", isDevice);
                if (isDevice == "device")
                {
                    // Use device limits
                    maxAnisotropy = device->getDevice().getPhysicalDeviceProperties().limits.maxSamplerAnisotropy;
                }
                else
                {
                    samplerNode->getAttribute(u8"maxAnisotropy", maxAnisotropy);
                }
            }

            auto sampler = std::make_shared<Vulkan::Sampler>();
            sampler->initialize(device->getDevice(),
                LoaderHelper::readValue(samplerNode, u8"magFilter", filters, false, context),
                LoaderHelper::readValue(samplerNode, u8"minFilter", filters, false, context),
                LoaderHelper::readValue(samplerNode, u8"mipmapMode", samplerMipmaps, false, context),
                LoaderHelper::readValue(samplerNode, u8"addressModeU", samplerAdresses, false, context),
                LoaderHelper::readValue(samplerNode, u8"addressModeV", samplerAdresses, false, context),
                LoaderHelper::readValue(samplerNode, u8"addressModeW", samplerAdresses, false, context),
                mipLodBias, anisotropyEnable, maxAnisotropy, compareEnable, compareOp,
                minLod, maxLod,
                LoaderHelper::readValue(samplerNode, u8"borderColor", borderColors, false, context),
                unnormalizedCoordinates
                );

            // Register
            device->insertSampler(sampler);
            _samplers[id] = sampler;
        }
    }
}

void Engine3DXMLLoader::loadBuffers(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak)
{
    MOUCA_PRE_CONDITION(!deviceWeak.expired()); //DEV Issue: Bad device !

    // Search Buffers
    auto result = context._parser.getNode(u8"Buffers");
    if (result->getNbElements() > 0)
    {
        MOUCA_ASSERT(result->getNbElements() == 1); //DEV Issue: please clean xml ?
        auto device = deviceWeak.lock();

        auto aPushB = context._parser.autoPushNode(*result->getNode(0));

        // Parsing all buffers
        auto allBuffers = context._parser.getNode(u8"Buffer");
        for (size_t idBuffer = 0; idBuffer < allBuffers->getNbElements(); ++idBuffer)
        {
            auto bufferNode = allBuffers->getNode(idBuffer);
            auto aPushM = context._parser.autoPushNode(*bufferNode);

            // Mandatory attribute
            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(bufferNode, u8"Buffer", _buffers, context, existing);
            if (existing)
            {
                continue;
            }

            const auto usage          = LoaderHelper::readValue(bufferNode, u8"usage", bufferUsages, true, context);
            VkDeviceSize size=0;
            if(bufferNode->hasAttribute(u8"size"))
            {
                bufferNode->getAttribute(u8"size", size);
            }
            else if(bufferNode->hasAttribute(u8"external"))
            {
                const uint32_t idE = LoaderHelper::getLinkedIdentifiant(bufferNode, u8"external", _cpuBuffers, context);
                size = _cpuBuffers[idE].lock()->getByteSize();
            }
            MOUCA_ASSERT(size > 0);

            VkBufferCreateFlags createFlags = 0;
            if (bufferNode->hasAttribute(u8"create"))
            {
                createFlags = LoaderHelper::readValue(bufferNode, u8"create", bufferCreates, true, context);
            }

            Vulkan::MemoryBufferUPtr memoryBuffer;
            loadMemoryBuffer(context, deviceWeak, memoryBuffer);

            auto buffer = std::make_shared<Vulkan::Buffer>(std::move(memoryBuffer));
            buffer->initialize(device->getDevice(), createFlags, usage, size);
            // Register
            device->insertBuffer(buffer);
            _buffers[id] = buffer;
        }
    }
}
// <Buffers>
// <Buffer id = "0" usage = "VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT" size = "192" / >
// < / Buffers>

void Engine3DXMLLoader::loadMemoryBuffer(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak, Vulkan::MemoryBufferUPtr& memoryBuffer)
{
    auto result = context._parser.getNode(u8"MemoryBuffer");
    if (result->getNbElements() > 0)
    {
        auto memoryNode = result->getNode(0);
        const auto memoryProperty = LoaderHelper::readValue(memoryNode, u8"property", memoryProperties, true, context);
        memoryBuffer = std::make_unique<Vulkan::MemoryBuffer>(memoryProperty);
    }
    else
    {
        auto resultAllo = context._parser.getNode(u8"MemoryBufferAllocate");
        if (resultAllo->getNbElements() > 0)
        {
            auto memoryNode = resultAllo->getNode(0);
            const auto memoryProperty = LoaderHelper::readValue(memoryNode, u8"property", memoryProperties, true, context);
            const auto memoryAllocate = LoaderHelper::readValue(memoryNode, u8"allocate", memoryAllocates, true, context);
            memoryBuffer = std::make_unique<Vulkan::MemoryBufferAllocate>(memoryProperty, memoryAllocate);
        }
        else
        {
            MOUCA_THROW_ERROR(u8"Engine3D", u8"UnknownMemoryError");
        }
    }
}


void Engine3DXMLLoader::loadGraphicsPipelines(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak)
{
    // Search Graphics Pipeline
    auto result = context._parser.getNode(u8"GraphicsPipelines");
    if (result->getNbElements() > 0)
    {
        MOUCA_ASSERT(result->getNbElements() == 1); //DEV Issue: please clean xml ?
        auto device = deviceWeak.lock();

        // cppcheck-suppress unreadVariable // false positive
        auto aPush = context._parser.autoPushNode(*result->getNode(0));

        // Parsing all Graphics pipeline
        auto allGraphicsPipelines = context._parser.getNode(u8"GraphicsPipeline");
        for (size_t idGraphicsPipeline = 0; idGraphicsPipeline < allGraphicsPipelines->getNbElements(); ++idGraphicsPipeline)
        {
            auto graphicsPipelineNode = allGraphicsPipelines->getNode(idGraphicsPipeline);

            // Mandatory attribute
            bool existing;
            const uint32_t id  = LoaderHelper::getIdentifiant(graphicsPipelineNode, u8"GraphicsPipeline", _graphicsPipelines, context, existing);

            const uint32_t idR = LoaderHelper::getLinkedIdentifiant(graphicsPipelineNode, u8"renderPassId", _renderPasses, context);
            const uint32_t idL = LoaderHelper::getLinkedIdentifiant(graphicsPipelineNode, u8"pipelineLayoutId", _pipelineLayouts, context);

            auto aPushG = context._parser.autoPushNode(*graphicsPipelineNode);


            auto graphicsPipeline = std::make_shared<Vulkan::GraphicsPipeline>();
            
            // Configure all States
            loadPipelineStateCreate(context, graphicsPipeline->getInfo(), id, idR);

            // Build new PipelineGraphic
            graphicsPipeline->initialize(device->getDevice(), _renderPasses[idR], _pipelineLayouts[idL], Vulkan::PipelineCacheWPtr());
            deviceWeak.lock()->insertGraphicsPipeline(graphicsPipeline);
            _graphicsPipelines[id] = graphicsPipeline;
        }
    }
}

void Engine3DXMLLoader::loadQueueSequences(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak)
{
    MOUCA_PRE_CONDITION(!deviceWeak.expired()); //DEV Issue: Bad device !

    // Search QueueSequences
    auto result = context._parser.getNode(u8"QueueSequences");
    if (result->getNbElements() > 0)
    {
        MOUCA_ASSERT(result->getNbElements() == 1); //DEV Issue: please clean xml ?
        auto device = deviceWeak.lock();

        // cppcheck-suppress unreadVariable // false positive
        auto aPush = context._parser.autoPushNode(*result->getNode(0));

        // Parsing all images
        auto allQueueSequences = context._parser.getNode(u8"QueueSequence");
        for (size_t idQueueSequences = 0; idQueueSequences < allQueueSequences->getNbElements(); ++idQueueSequences)
        {
            auto queueSequenceNode = allQueueSequences->getNode(idQueueSequences);

            // Mandatory attribute
            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(queueSequenceNode, u8"View", _queueSequences, context, existing);

            auto aPushQ = context._parser.autoPushNode(*queueSequenceNode);

            auto sequences = std::make_shared<Vulkan::QueueSequence>();
            loadSequences(context, deviceWeak, sequences);

            // Save data
            device->insertSequence(sequences);

            _queueSequences[id] = sequences;
        }
    }
}

void Engine3DXMLLoader::loadSequences(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak, Vulkan::QueueSequenceWPtr queueSequenceWeak)
{
    MOUCA_PRE_CONDITION(!deviceWeak.expired());        //DEV Issue: Bad device !
    MOUCA_PRE_CONDITION(!queueSequenceWeak.expired()); //DEV Issue: Bad container !

    auto queueSequence = queueSequenceWeak.lock();
    // Parsing all images
    auto allSequences = context._parser.getNode(u8"Sequence");
    for (size_t idSequences = 0; idSequences < allSequences->getNbElements(); ++idSequences)
    {
        auto sequenceNode = allSequences->getNode(idSequences);

        // Get type of Sequence
        Core::String type;
        sequenceNode->getAttribute(u8"type", type);

        // Build and initialize each sequence
        if(type == u8"acquire")
        {
            Vulkan::FenceWPtr     fence;
            Vulkan::SemaphoreWPtr semaphore;
            uint64_t timeout = std::numeric_limits<uint64_t>::max();

            // Read optional data
            if( sequenceNode->hasAttribute(u8"fenceId") )
            {
                const uint32_t idF = LoaderHelper::getLinkedIdentifiant(sequenceNode, u8"fenceId", _fences, context);
                fence = _fences[idF];
            }
            if(sequenceNode->hasAttribute(u8"timeout")) { sequenceNode->getAttribute(u8"timeout", timeout); }
            if(sequenceNode->hasAttribute(u8"semaphoreId"))
            {
                const uint32_t idSem = LoaderHelper::getLinkedIdentifiant(sequenceNode, u8"semaphoreId", _semaphores, context);
                semaphore = _semaphores[idSem];
            }

            // Build Sequence
            const uint32_t idS = LoaderHelper::getLinkedIdentifiant(sequenceNode, u8"surfaceId", _surfaces, context);
            queueSequence->emplace_back(std::make_shared<Vulkan::SequenceAcquire>(_surfaces[idS].lock()->getEditSwapChain(), semaphore, fence, timeout));
        }
        else if (type == u8"waitFence")
        {
            uint64_t timeout = std::numeric_limits<uint64_t>::max();

            // Read optional data
            if(sequenceNode->hasAttribute(u8"timeout")) { sequenceNode->getAttribute(u8"timeout", timeout); }
            
            bool waitAll;
            sequenceNode->getAttribute(u8"waitAll", waitAll);

            // Read all fences associated
            std::vector<Vulkan::FenceWPtr> fences;
            auto aPush     = context._parser.autoPushNode(*sequenceNode);
            auto allFences = context._parser.getNode(u8"Fence");
            for (size_t idFence = 0; idFence < allFences->getNbElements(); ++idFence)
            {
                auto fenceNode = allFences->getNode(idFence);
                const uint32_t idF = LoaderHelper::getLinkedIdentifiant(fenceNode, u8"fenceId", _fences, context);
                fences.emplace_back(_fences[idF]);
            }
            if (fences.empty())
            {
                MOUCA_THROW_ERROR_3(u8"Engine3D", u8"XMLMissingNodeError", context.getFileName(), u8"Sequence type=waitFence", u8"Fence");
            }

            // Build Sequence
            queueSequence->emplace_back(std::make_shared<Vulkan::SequenceWaitFence>(fences, timeout, (waitAll ? VK_TRUE : VK_FALSE)));
        }
        else if (type == u8"resetFence")
        {
            // Read all fences associated
            std::vector<Vulkan::FenceWPtr> fences;
            auto aPush = context._parser.autoPushNode(*sequenceNode);
            auto allFences = context._parser.getNode(u8"Fence");
            for (size_t idFence = 0; idFence < allFences->getNbElements(); ++idFence)
            {
                auto fenceNode = allFences->getNode(idFence);
                const uint32_t idF = LoaderHelper::getLinkedIdentifiant(fenceNode, u8"fenceId", _fences, context);
                fences.emplace_back(_fences[idF]);
            }
            if (fences.empty())
            {
                MOUCA_THROW_ERROR_3(u8"Engine3D", u8"XMLMissingNodeError", context.getFileName(), u8"Sequence type=waitFence", u8"Fence");
            }

            // Build Sequence
            queueSequence->emplace_back(std::make_shared<Vulkan::SequenceResetFence>(fences));
        }
        else if (type == u8"submit")
        {
            Vulkan::FenceWPtr fence;
            if (sequenceNode->hasAttribute(u8"fenceId"))
            {
                const uint32_t idF = LoaderHelper::getLinkedIdentifiant(sequenceNode, u8"fenceId", _fences, context);
                fence = _fences[idF];
            }

            // Build SubmitInfo
            Vulkan::SubmitInfos submitInfos;
            auto aPush = context._parser.autoPushNode(*sequenceNode);
            loadSubmitInfo(context, deviceWeak, submitInfos);

            // Build Sequence
            queueSequence->emplace_back(std::make_shared<Vulkan::SequenceSubmit>(std::move(submitInfos), fence));
        }
        else if (type == u8"submitVR")
        {
            auto& vrPlatform = context._engine.getVRPlatform();
            if (vrPlatform.isNull())
            {
                MOUCA_THROW_ERROR_1(u8"Engine3D", u8"VRNotReadyError", u8"Sequence");
            }
            // Read parameters
            Core::String eye;
            sequenceNode->getAttribute(u8"eye", eye);
            const bool isLeftEye = (eye == u8"left");

            const uint32_t idI = LoaderHelper::getLinkedIdentifiant(sequenceNode, u8"imageId", _images, context);
            auto image = _images[idI].lock();

            // Build Sequence
            const auto& device = deviceWeak.lock()->getDevice();
            queueSequence->emplace_back(std::make_shared <MouCaVR::Platform::SequenceSubmitVR>(vrPlatform, isLeftEye, _manager.getEnvironment(), device.getInstance(), device.getPhysicalDevice(), device.getQueue(), device.getQueueFamilyGraphicId(), image->getImage(), image->getSamples()));
        }
        else if (type == u8"presentKHR")
        {
            Vulkan::FenceWPtr fence;
            if (sequenceNode->hasAttribute(u8"fenceId"))
            {
                const uint32_t idF = LoaderHelper::getLinkedIdentifiant(sequenceNode, u8"fenceId", _fences, context);
                fence = _fences[idF];
            }

            auto aPush = context._parser.autoPushNode(*sequenceNode);

            std::vector<Vulkan::SemaphoreWPtr> semaphores;
            std::vector<Vulkan::SwapChainWPtr> swapChains;

            // Parsing all Swapchain
            auto allSwapchains = context._parser.getNode(u8"Swapchain");
            for (size_t idSwapchain = 0; idSwapchain < allSwapchains->getNbElements(); ++idSwapchain)
            {
                auto swapChainNode = allSwapchains->getNode(idSwapchain);

                // Read info
                const uint32_t idS = LoaderHelper::getLinkedIdentifiant(swapChainNode, u8"surfaceId", _surfaces, context);
                swapChains.emplace_back(_surfaces[idS].lock()->getEditSwapChain());
            }

            if( swapChains.empty() )
            {
                MOUCA_THROW_ERROR_3(u8"Engine3D", u8"XMLMissingNodeError", context.getFileName(), u8"Sequence type=presentKHR", u8"Swapchain");
            }

            // Parsing all Signal Semaphore
            auto allSemaphores = context._parser.getNode(u8"Semaphore");
            for (size_t idSemaphore = 0; idSemaphore < allSemaphores->getNbElements(); ++idSemaphore)
            {
                auto semaphoreNode = allSemaphores->getNode(idSemaphore);
                // Read info
                const uint32_t idSem = LoaderHelper::getLinkedIdentifiant(semaphoreNode, u8"semaphoreId", _semaphores, context);
                // Store
                semaphores.emplace_back(_semaphores[idSem]);
            }
            // Build Sequence
            auto sequence = std::make_shared<Vulkan::SequencePresentKHR>(semaphores, swapChains);
            sequence->registerSwapChain(); //Sniff: best into constructor
            queueSequence->emplace_back(sequence);
        }
        else
        {
            MOUCA_THROW_ERROR_2(u8"Engine3D", u8"XMLUnknownSequenceError", context.getFileName(), type);
        }
    }
}

void Engine3DXMLLoader::loadSubmitInfo(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak, Vulkan::SubmitInfos& submitInfos)
{
    MOUCA_PRE_CONDITION(!deviceWeak.expired());        //DEV Issue: Bad device !

    std::vector<Vulkan::WaitSemaphore>      waitSemaphores;
    std::vector<Vulkan::SemaphoreWPtr>      signalSemaphores;
    std::vector<Vulkan::ICommandBufferWPtr> commandBuffers;

    // Parsing all WaitSync
    auto allWaitSyncs = context._parser.getNode(u8"WaitSync");
    for (size_t idWaitSync = 0; idWaitSync < allWaitSyncs->getNbElements(); ++idWaitSync)
    {
        auto waitSyncNode = allWaitSyncs->getNode(idWaitSync);

        // Read info
        const uint32_t idSem  = LoaderHelper::getLinkedIdentifiant(waitSyncNode, u8"semaphoreId", _semaphores, context);
        const auto pipelineFlag = LoaderHelper::readValue(waitSyncNode, u8"pipelineFlag", pipelineStageFlags, true, context);
        // Store
        waitSemaphores.emplace_back(Vulkan::WaitSemaphore(_semaphores[idSem], pipelineFlag));
    }

    // Parsing all Signal Semaphore
    auto allSignalSyncs = context._parser.getNode(u8"SignalSync");
    for (size_t idSignalSync = 0; idSignalSync < allSignalSyncs->getNbElements(); ++idSignalSync)
    {
        auto signalSyncNode = allSignalSyncs->getNode(idSignalSync);

        // Read info
        const uint32_t idSem  = LoaderHelper::getLinkedIdentifiant(signalSyncNode, u8"semaphoreId", _semaphores, context);
        // Store
        signalSemaphores.emplace_back(_semaphores[idSem]);
    }

    // Parsing all Signal Semaphore
    auto allCommandBuffers = context._parser.getNode(u8"CommandBuffer");
    for (size_t idCommandBuffer = 0; idCommandBuffer < allCommandBuffers->getNbElements(); ++idCommandBuffer)
    {
        auto commandBufferNode = allCommandBuffers->getNode(idCommandBuffer);

        if(commandBufferNode->hasAttribute(u8"surfaceId"))
        {
            // Get surface associated
            const uint32_t idS = LoaderHelper::getLinkedIdentifiant(commandBufferNode, u8"surfaceId", _surfaces, context);
            
            // Prepare data
            auto surface = _surfaces[idS].lock();
            commandBuffers.emplace_back(surface->getICommandBuffer());
        }
        else
        {
            // Get surface associated
            const uint32_t idC = LoaderHelper::getLinkedIdentifiant(commandBufferNode, u8"id", _commandBuffers, context);
            commandBuffers.emplace_back(_commandBuffers[idC]);
        }
    }

    auto submitInfo = std::make_unique<Vulkan::SubmitInfo>();
    submitInfo->addSynchronization(waitSemaphores, signalSemaphores);
    submitInfo->initialize(std::move(commandBuffers));
    //submitInfo->buildSubmitInfo();

    submitInfos.emplace_back(std::move(submitInfo));
}

void Engine3DXMLLoader::loadDescriptorSetLayouts(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak)
{
    // Search DescriptorSetLayouts
    auto result = context._parser.getNode(u8"DescriptorSetLayouts");
    if (result->getNbElements() > 0)
    {
        MOUCA_ASSERT(result->getNbElements() == 1); //DEV Issue: please clean xml ?
        auto device = deviceWeak.lock();

        auto aPush = context._parser.autoPushNode(*result->getNode(0));

        // Parsing all DescriptorSetLayout
        auto allDescriptorSetLayouts = context._parser.getNode(u8"DescriptorSetLayout");
        for (size_t idDescriptorSetLayout = 0; idDescriptorSetLayout < allDescriptorSetLayouts->getNbElements(); ++idDescriptorSetLayout)
        {
            auto descriptorSetLayoutNode = allDescriptorSetLayouts->getNode(idDescriptorSetLayout);

            // Load mandatory 
            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(descriptorSetLayoutNode, u8"DescriptorSetLayout", _descriptorSetLayouts, context, existing);
            if (existing)
            {
                continue;
            }

            auto descriptorSetLayout = std::make_shared<Vulkan::DescriptorSetLayout>();

            // Parsing binding
            auto aPushD = context._parser.autoPushNode(*descriptorSetLayoutNode);
            auto allBindings = context._parser.getNode(u8"Binding");
            for (size_t idBinding = 0; idBinding < allBindings->getNbElements(); ++idBinding)
            {
                auto bindingNode = allBindings->getNode(idBinding);
                
                uint32_t count;
                bindingNode->getAttribute(u8"count", count);
                const auto flags = LoaderHelper::readValue(bindingNode, u8"shaderStageFlags", shaderStages, true, context);
                const auto type  = LoaderHelper::readValue(bindingNode, u8"type", descriptorTypes, false, context);
                descriptorSetLayout->addBinding(type, count, flags);
            }
            
            // Build
            descriptorSetLayout->initialize(device->getDevice());

            // Register
            device->insertDescriptorSetLayout(descriptorSetLayout);
            _descriptorSetLayouts[id] = descriptorSetLayout;
        }
    }
}

void Engine3DXMLLoader::loadDescriptorSetPools(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak)
{
    // Search DescriptorSetPool
    auto result = context._parser.getNode(u8"DescriptorPools");
    if (result->getNbElements() > 0)
    {
        MOUCA_ASSERT(result->getNbElements() == 1); //DEV Issue: please clean xml ?
        auto device = deviceWeak.lock();

        auto aPush = context._parser.autoPushNode(*result->getNode(0));

        // Parsing all DescriptorSetLayout
        auto allDescriptorPools = context._parser.getNode(u8"DescriptorPool");
        for (size_t idDescriptorPool = 0; idDescriptorPool < allDescriptorPools->getNbElements(); ++idDescriptorPool)
        {
            auto descriptorPoolNode = allDescriptorPools->getNode(idDescriptorPool);

            // Mandatory data
            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(descriptorPoolNode, u8"DescriptorPool", _descriptorPools, context, existing);
            if(existing)
            {
                continue;
            }
            uint32_t maxSets;
            descriptorPoolNode->getAttribute(u8"maxSets", maxSets);

            // Parse Size
            auto aPushS = context._parser.autoPushNode(*descriptorPoolNode);
            auto allSizes = context._parser.getNode(u8"Size");

            std::vector<VkDescriptorPoolSize> sizes;
            sizes.reserve(allSizes->getNbElements());
            for (size_t idSize = 0; idSize < allSizes->getNbElements(); ++idSize)
            {
                auto sizeNode = allSizes->getNode(idSize);
                VkDescriptorPoolSize poolSize;
                
                poolSize.type = LoaderHelper::readValue(sizeNode, u8"type", descriptorTypes, false, context);
                sizeNode->getAttribute(u8"descriptorCount", poolSize.descriptorCount);

                sizes.emplace_back(poolSize);
            }

            // Build
            auto descriptorPool = std::make_shared<Vulkan::DescriptorPool>();
            descriptorPool->initialize(device->getDevice(), sizes, maxSets);

            // Register
            device->insertDescriptorPool(descriptorPool);
            _descriptorPools[id] = descriptorPool;
        }
    }
}

void Engine3DXMLLoader::loadDescriptorSets(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak)
{
    // Search DescriptorSets
    auto result = context._parser.getNode(u8"DescriptorSets");
    if (result->getNbElements() > 0)
    {
        MOUCA_ASSERT(result->getNbElements() == 1); //DEV Issue: please clean xml ?
        auto device = deviceWeak.lock();

        auto aPush = context._parser.autoPushNode(*result->getNode(0));

        // Parsing all DescriptorSetLayout
        auto allDescriptorSets = context._parser.getNode(u8"DescriptorSet");
        for (size_t idDescriptorSet = 0; idDescriptorSet < allDescriptorSets->getNbElements(); ++idDescriptorSet)
        {
            auto descriptorSetNode = allDescriptorSets->getNode(idDescriptorSet);

            // Load mandatory 
            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(descriptorSetNode, u8"DescriptorSet", _descriptorSets, context, existing);
            if (existing)
            {
                continue;
            }

            const uint32_t idPool = LoaderHelper::getLinkedIdentifiant(descriptorSetNode, u8"descriptorPoolId", _descriptorPools, context);
            
            // Parse SetLayout
            auto aPushS = context._parser.autoPushNode(*descriptorSetNode);
            auto allSetLayouts = context._parser.getNode(u8"SetLayout");

            // Allocate array
            std::vector<VkDescriptorSetLayout> dSetLayouts;
            std::vector<std::vector<Vulkan::WriteDescriptorSet>> writeDescriptors;
            writeDescriptors.resize(allSetLayouts->getNbElements());
            auto itWriteDescriptor = writeDescriptors.begin();

            dSetLayouts.reserve(allSetLayouts->getNbElements());
            // Fill
            for (size_t idDSetLayout = 0; idDSetLayout < allSetLayouts->getNbElements(); ++idDSetLayout)
            {
                auto dSetLayoutNode = allSetLayouts->getNode(idDSetLayout);

                const uint32_t idS = LoaderHelper::getLinkedIdentifiant(dSetLayoutNode, u8"descriptorSetLayoutId", _descriptorSetLayouts, context);
                dSetLayouts.emplace_back(_descriptorSetLayouts[idS].lock()->getInstance());

                // Parse WriteDescriptor
                auto aPushSL = context._parser.autoPushNode(*dSetLayoutNode);
                
                auto allWriteDescriptors = context._parser.getNode(u8"WriteDescriptor");
                for (size_t idWriteDescriptor = 0; idWriteDescriptor < allWriteDescriptors->getNbElements(); ++idWriteDescriptor)
                {
                    auto writeDescriptorNode = allWriteDescriptors->getNode(idWriteDescriptor);

                    // Read attribute
                    const auto     type    = LoaderHelper::readValue(writeDescriptorNode, u8"descriptorType", descriptorTypes, false, context);
                    uint32_t binding;
                    writeDescriptorNode->getAttribute(u8"binding", binding);

                    // Parse WriteDescriptor
                    auto aPushW = context._parser.autoPushNode(*writeDescriptorNode);

                    // Build Buffer info
                    auto allBufferInfos = context._parser.getNode(u8"BufferInfo");
                    if(allBufferInfos->getNbElements() > 0)
                    {
                        Vulkan::DescriptorBufferInfos buffers;
                        for (size_t idBufferInfo = 0; idBufferInfo < allBufferInfos->getNbElements(); ++idBufferInfo)
                        {
                            auto bufferInfoNode = allBufferInfos->getNode(idBufferInfo);

                            const auto idBuffer = LoaderHelper::getLinkedIdentifiant(bufferInfoNode, u8"bufferId", _buffers, context);

                            if (bufferInfoNode->hasAttribute(u8"offset"))
                            {
                                VkDeviceSize offset;
                                bufferInfoNode->getAttribute(u8"offset", offset);
                                VkDeviceSize range;
                                bufferInfoNode->getAttribute(u8"range", range);

                                // Copy full descriptor
                                Vulkan::DescriptorBufferInfo info
                                { _buffers[idBuffer], offset, range };
                                buffers.emplace_back(std::move(info));
                            }
                            else // Full buffer
                            {
                                const auto& descriptor = _buffers[idBuffer].lock()->getDescriptor();
                                Vulkan::DescriptorBufferInfo info
                                { _buffers[idBuffer], descriptor.offset, descriptor.range };
                                buffers.emplace_back(std::move(info));
                            }
                        }

                        itWriteDescriptor->emplace_back(Vulkan::WriteDescriptorSet(binding, type, std::move(buffers)));
                    }
                    
                    auto allImageInfos = context._parser.getNode(u8"ImageInfo");
                    if(allImageInfos->getNbElements() > 0)
                    {
                        Vulkan::DescriptorImageInfos images;
                        for (size_t idImageInfo = 0; idImageInfo < allImageInfos->getNbElements(); ++idImageInfo)
                        {
                            auto imageInfoNode = allImageInfos->getNode(idImageInfo);

                            const auto idSampler = LoaderHelper::getLinkedIdentifiant(imageInfoNode, u8"samplerId", _samplers, context);
                            const auto idView    = LoaderHelper::getLinkedIdentifiant(imageInfoNode, u8"viewId",    _view, context);

                            Vulkan::DescriptorImageInfo info
                            {
                                _samplers[idSampler].lock(),
                                _view[idView].lock(),
                                LoaderHelper::readValue(imageInfoNode, u8"layout", imageLayouts, false, context)
                            };
                            images.emplace_back(std::move(info));
                        }
                        itWriteDescriptor->emplace_back(Vulkan::WriteDescriptorSet(binding, type, std::move(images)));
                    }
                }
                // Next Set
                ++itWriteDescriptor;
            }

            auto descriptorSet = std::make_shared<Vulkan::DescriptorSet>();
            descriptorSet->initialize(device->getDevice(), _descriptorPools[idPool], dSetLayouts);

            // Update descriptor set
            uint32_t set = 0;
            for (auto& writeDescriptor : writeDescriptors)
            {
                descriptorSet->update(device->getDevice(), set, std::move(writeDescriptor));
                ++set;
            }

            device->insertDescriptorSet(descriptorSet);
            _descriptorSets[id] = descriptorSet;
        }
    }
}

void Engine3DXMLLoader::loadShaderModules(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak)
{
    // Search DescriptorSetLayouts
    auto result = context._parser.getNode(u8"ShaderModules");
    if (result->getNbElements() > 0)
    {
        MOUCA_ASSERT(result->getNbElements() == 1); //DEV Issue: please clean xml ?
        auto device = deviceWeak.lock();

        // cppcheck-suppress unreadVariable // false positive
        auto aPush = context._parser.autoPushNode(*result->getNode(0));

        // Parsing all ShaderModule
        auto allShaderModules = context._parser.getNode(u8"ShaderModule");
        for (size_t idShaderModule = 0; idShaderModule < allShaderModules->getNbElements(); ++idShaderModule)
        {
            auto shaderModuleNode = allShaderModules->getNode(idShaderModule);

            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(shaderModuleNode, u8"ShaderModule", _shaderModules, context, existing);

            // Read info
            Core::String filename;
            shaderModuleNode->getAttribute(u8"spirv", filename);

            Core::String source;
            if(shaderModuleNode->hasAttribute(u8"code"))
            {
                shaderModuleNode->getAttribute(u8"code", source);
            }

            // Create shader resources
            auto& resourceManager = context._resources;
            const auto stage = LoaderHelper::readValue(shaderModuleNode, u8"stage", shaderStages, false, context);

            auto shaderFile = resourceManager.openShader(Core::convertToOS(filename), shaderKinds[stage], Core::convertToOS(source));

            // If code source: enable tracking
            if(!source.empty())
            {
                // Register manage to file tracker
                resourceManager.getTracker().registerResource(shaderFile);
            }     

            // Build shader
            shaderFile->open();
            MOUCA_ASSERT(shaderFile->isLoaded());

            auto shaderModule = std::make_shared<Vulkan::ShaderModule>();
            shaderModule->initialize(device->getDevice(), shaderFile->extractString(), u8"main", static_cast<VkShaderStageFlagBits>(stage));
            
            // Register
            device->insertShaderModule(shaderModule);
            _shaderModules[id] = shaderModule;
            
            // Clean resource after use (TEMP)
            shaderFile->close();

            // Release resource if not tracked
            if(source.empty())
            {
                resourceManager.releaseResource(shaderFile);
            }
            else // Or register file/module
            {
                _manager.registerShader(shaderFile, { device, shaderModule });
            }
        }
    }
}

void Engine3DXMLLoader::loadRayTracingPipelines(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak)
{
    // Search DescriptorSetLayouts
    auto result = context._parser.getNode(u8"RayTracingPipelines");
    if (result->getNbElements() > 0)
    {
        MOUCA_ASSERT(result->getNbElements() == 1); //DEV Issue: please clean xml ?
        auto device = deviceWeak.lock();

        // cppcheck-suppress unreadVariable // false positive
        auto aPush = context._parser.autoPushNode(*result->getNode(0));

        // Parsing all ShaderModule
        auto allRayPipelines = context._parser.getNode(u8"RayTracingPipeline");
        for (size_t idPipeline = 0; idPipeline < allRayPipelines->getNbElements(); ++idPipeline)
        {
            auto pipelineNode = allRayPipelines->getNode(idPipeline);

            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(pipelineNode, u8"RayTracingPipeline", _rayTracingPipelines, context, existing);
            if(existing)
            {
                continue;
            }

            auto pipeline = std::make_shared<Vulkan::RayTracingPipeline>();

            auto aPushP = context._parser.autoPushNode(*pipelineNode);            
            // Stages
            loadPipelineStages(context, pipeline->getShadersStage());
            if (pipeline->getShadersStage().getNbShaders() == 0)
            {
                MOUCA_THROW_ERROR_3(u8"Engine3D", u8"XMLMissingNodeError", context.getFileName(), u8"RayTracingPipeline", u8"Stages/Stage");
            }

            // Groups
            loadRayTracingShaderGroup(context, deviceWeak, *pipeline);

            const uint32_t idL = LoaderHelper::getLinkedIdentifiant(pipelineNode, u8"pipelineLayoutId", _pipelineLayouts, context);
            uint32_t maxRecursive = 0;
            pipelineNode->getAttribute(u8"maxPipelineRayRecursionDepth", maxRecursive);
            
            pipeline->initialize(device->getDevice(), _pipelineLayouts[idL], maxRecursive);

            device->insertRayTracingPipeline(pipeline);
            _rayTracingPipelines[id] = pipeline;
        }
    }
}

void Engine3DXMLLoader::loadRayTracingShaderGroup(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak, Vulkan::RayTracingPipeline& pipeline)
{
    auto groups = context._parser.getNode(u8"Groups");
    if (groups->getNbElements() > 0)
    {
        MOUCA_ASSERT(groups->getNbElements() == 1); //DEV Issue: Need to clean xml !

        // cppcheck-suppress unreadVariable // false positive
        auto aPush = context._parser.autoPushNode(*groups->getNode(0));

        // Parsing all Graphics pipeline
        auto allGroups = context._parser.getNode(u8"Group");
        for (size_t idGroup = 0; idGroup < allGroups->getNbElements(); ++idGroup)
        {
            auto groupNode = allGroups->getNode(idGroup);

            VkRayTracingShaderGroupCreateInfoKHR group
            {
                VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR ,    //VkStructureType                   sType;
                nullptr,                                                        //const void* pNext;
                VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR,                   //VkRayTracingShaderGroupTypeKHR    type;
                VK_SHADER_UNUSED_KHR,                                           //uint32_t                          generalShader;
                VK_SHADER_UNUSED_KHR,                                           //uint32_t                          closestHitShader;
                VK_SHADER_UNUSED_KHR,                                           //uint32_t                          anyHitShader;
                VK_SHADER_UNUSED_KHR,                                           //uint32_t                          intersectionShader;
                nullptr,                                                        //const void* pShaderGroupCaptureReplayHandle;
            };

            group.type = LoaderHelper::readValue(groupNode, u8"type", rayTracingGroupTypes, false, context);

            if(groupNode->hasAttribute(u8"generalShader"))
            {
                groupNode->getAttribute(u8"generalShader", group.generalShader);
            }
            if (groupNode->hasAttribute(u8"closestHitShader"))
            {
                groupNode->getAttribute(u8"closestHitShader", group.closestHitShader);
            }
            if (groupNode->hasAttribute(u8"anyHitShader"))
            {
                groupNode->getAttribute(u8"anyHitShader", group.anyHitShader);
            }
            if (groupNode->hasAttribute(u8"intersectionShader"))
            {
                groupNode->getAttribute(u8"intersectionShader", group.intersectionShader);
            }

            pipeline.addGroup(std::move(group));
        }
    }
}

void Engine3DXMLLoader::loadTracingRay(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak)
{
    auto tracingRays = context._parser.getNode(u8"TracingRays");
    if (tracingRays->getNbElements() > 0)
    {
        MOUCA_ASSERT(tracingRays->getNbElements() == 1); //DEV Issue: Need to clean xml !
        auto device = deviceWeak.lock();

        // cppcheck-suppress unreadVariable // false positive
        auto aPush = context._parser.autoPushNode(*tracingRays->getNode(0));

        // Parsing all Graphics pipeline
        auto allTracingRays = context._parser.getNode(u8"TracingRay");
        for (size_t idTracingRay = 0; idTracingRay < allTracingRays->getNbElements(); ++idTracingRay)
        {
            auto tracingRayNode = allTracingRays->getNode(idTracingRay);

            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(tracingRayNode, u8"RayTracingPipeline", _tracingRays, context, existing);
            if (existing)
            {
                continue;
            }

            const uint32_t idPipeline = LoaderHelper::getLinkedIdentifiant(tracingRayNode, u8"rayTracingPipelines", _rayTracingPipelines, context);
            Vulkan::TracingRay::BufferSizes sizes{ 0, 0, 0, 0 };
            
            tracingRayNode->getAttribute(u8"raygenSize",   sizes[0]);
            tracingRayNode->getAttribute(u8"missSize",     sizes[1]);
            tracingRayNode->getAttribute(u8"hitSize",      sizes[2]);
            tracingRayNode->getAttribute(u8"callableSize", sizes[3]);

            auto tracingRay = std::make_shared<Vulkan::TracingRay>();
            tracingRay->initialize(device->getDevice(), _rayTracingPipelines[idPipeline], 0, sizes);

            device->insertTracingRay(tracingRay);
            _tracingRays[id] = tracingRay;
        }
    }
}

void Engine3DXMLLoader::loadAccelerationStructures(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak)
{
    auto accelerationStructures = context._parser.getNode(u8"AccelerationStructures");
    if (accelerationStructures->getNbElements() > 0)
    {
        MOUCA_ASSERT(accelerationStructures->getNbElements() == 1); //DEV Issue: Need to clean xml !
        auto device = deviceWeak.lock();

        // cppcheck-suppress unreadVariable // false positive
        auto aPush = context._parser.autoPushNode(*accelerationStructures->getNode(0));

        // Parsing all Graphics pipeline
        auto allAccelerationStructures = context._parser.getNode(u8"AccelerationStructure");
        for (size_t idAccelerationStructure = 0; idAccelerationStructure < allAccelerationStructures->getNbElements(); ++idAccelerationStructure)
        {
            auto accelerationStructureNode = allAccelerationStructures->getNode(idAccelerationStructure);

            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(accelerationStructureNode, u8"AccelerationStructure", _accelerationStructures, context, existing);
            if (existing)
            {
                continue;
            }

            auto as = std::make_shared<Vulkan::AccelerationStructure>();

            const auto type = LoaderHelper::readValue(accelerationStructureNode, u8"type", accelerationStructureTypes, false, context);

            auto aPushA = context._parser.autoPushNode(*accelerationStructureNode);

            {
                auto allGeometries = context._parser.getNode(u8"Geometry");
                for (size_t idGeometry = 0; idGeometry < allGeometries->getNbElements(); ++idGeometry)
                {
                    auto geometryNode = allGeometries->getNode(idGeometry);

                    const auto flag = LoaderHelper::readValue(geometryNode, u8"flag", geometryFlags, true, context );
                    Core::String typeG;
                    geometryNode->getAttribute("type", typeG);
                    if(typeG == u8"triangle")
                    {
                        const uint32_t idMesh = LoaderHelper::getLinkedIdentifiant(geometryNode, u8"meshId", _cpuMesh, context);

                        const uint32_t idVBO = LoaderHelper::getLinkedIdentifiant(geometryNode, u8"vboBufferId", _buffers, context);
                        const uint32_t idIBO = LoaderHelper::getLinkedIdentifiant(geometryNode, u8"iboBufferId", _buffers, context);
                        
                        auto triangle = std::make_unique<Vulkan::AccelerationStructureGeometryTriangles>();
                        triangle->initialize(_cpuMesh[idMesh], _buffers[idIBO], _buffers[idVBO], flag);

                        as->addGeometry(std::move(triangle));
                    }
                    else if (typeG == u8"instance")
                    {
                        auto aPushGI = context._parser.autoPushNode(*geometryNode);

                        auto allInstances = std::make_unique<Vulkan::AccelerationStructureGeometryInstance>();

                        auto allInstancesNode = context._parser.getNode(u8"Instance");
                        for (size_t idInstance = 0; idInstance < allInstancesNode->getNbElements(); ++idInstance)
                        {
                            auto instanceNode = allInstancesNode->getNode(idInstance);
                            const uint32_t idAS = LoaderHelper::getLinkedIdentifiant(instanceNode, u8"accelerationStructureId", _accelerationStructures, context);

                            const auto flagI = LoaderHelper::readValue(instanceNode, u8"flag", geometryInstanceFlags, true, context );

                            VkTransformMatrixKHR transformMatrix =
                            {
                                1.0f, 0.0f, 0.0f, 0.0f,
                                0.0f, 1.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 1.0f, 0.0f
                            };

                            auto instance = Vulkan::AccelerationStructureGeometryInstance::Instance();
                            instance.initialize(_accelerationStructures[idAS], flagI, std::move(transformMatrix));

                            allInstances->addInstance(std::move(instance));
                        }
                        
                        allInstances->initialize(flag);

                        as->addGeometry(std::move(allInstances));
                    }
                    else
                    {
                        MOUCA_THROW_ERROR_3(u8"Engine3D", u8"UnknownASGeometryError", context.getFileName(), u8"Geometry", typeG);
                    }
                }
                //bgs[0]->initialize(*device);
            }

            // Prepare AS
            as->setCreateInfo(0, type);

            // Need to update as when data ready: not here !
            
            device->insertAccelerationStructure(as);
            _accelerationStructures[id] = as;
        }
    }
}

}