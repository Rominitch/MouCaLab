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

Core::ErrorData Engine3DXMLLoader::makeLoaderError(const ContextLoading& context, const Core::StringView& idError) const
{
    return Core::ErrorData("Engine3D", idError) << context.getFileName().string();
}

template<>
static void LoaderHelper::readData(const XML::NodeUPtr& node, VkExtent3D& extent)
{
    node->getAttribute("width", extent.width);
    node->getAttribute("height", extent.height);
    node->getAttribute("depth", extent.depth);
}
    
Engine3DXMLLoader::ContextLoading::ContextLoading(GraphicEngine& engine, XML::Parser& parser, MouCaCore::ResourceManager& resources):
_engine(engine), _parser(parser), _resources(resources), _xmlFileName(parser.getFilename())
{}

//-----------------------------------------------------------------------------------------
//                                  Load / Save engine
//-----------------------------------------------------------------------------------------

Engine3DXMLLoader::~Engine3DXMLLoader()
{}

void Engine3DXMLLoader::load(ContextLoading& context)
{
    MouCa::preCondition(context._parser.isLoaded()); //DEV Issue: Need a valid xml.

    //Read Engine part
    auto result = context._parser.getNode("Engine3D");
    if (result->getNbElements() == 0)
    {
        throw Core::Exception(makeLoaderError(context, "LoadingXMLCorruptError") << "Engine3D");
    }

    XML::NodeUPtr engineNode = result->getNode(0);
    float version;
    engineNode->getAttribute("version", version);

    // Check version before continue
    if (VulkanManager::_version < version)
    {
        throw Core::Exception(makeLoaderError(context, "LoadingXMLVersionError") << std::to_string(version));
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
    auto datas = context._parser.getNode("GlobalData");
    if (datas->getNbElements() > 0)
    {
        MouCa::assertion(datas->getNbElements() == 1);
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
    MouCa::preCondition(_dialogs.empty());

    auto result = context._parser.getNode("Window");
    for (size_t idWindow = 0; idWindow < result->getNbElements(); ++idWindow)
    {
        auto window = result->getNode(idWindow);
        bool existing;
        const uint32_t id = LoaderHelper::getIdentifiant(window, "Window", _dialogs, context, existing);
        if (existing)
            continue;

        // Title
        Core::String title;
        window->getAttribute("title", title);

        // Mode
        bool state = false;
        window->getAttribute("visible", state);
        auto mode = (state ? RT::Window::Visible : RT::Window::Unknown);

        // Viewport
        if (window->hasAttribute("monitor"))
        {
            size_t idMonitor = 0;
            window->getAttribute("monitor", idMonitor);
            if (idMonitor >= context._engine.getMonitors().size())
            {
                throw Core::Exception(makeLoaderError(context, "WindowUnknownMonitorError") << std::to_string(idMonitor));
            }

            window->getAttribute("fullscreen", state);
            mode = static_cast<RT::Window::Mode>(mode | (state ? RT::Window::Resizable : RT::Window::Unknown));

            // Build dialog
            _dialogs[id] = context._engine.getRTPlatform().createWindow(context._engine.getMonitors()[idMonitor], title, mode);
        }
        else
        {
            int32_t posX = 0, posY = 0;
            RT::ViewportInt32 viewport;
            window->getAttribute("positionX", posX);
            window->getAttribute("positionY", posY);
            viewport.setOffset(posX, posY);

            LoaderHelper::readAttribute(window, "width",  posX, context);
            LoaderHelper::readAttribute(window, "height", posY, context);
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
                throw Core::Exception(makeLoaderError(context, "WindowCreationOutsideError") << "Current windows is not on a screen.");
            }

            // Mode
            window->getAttribute("resizable", state);
            mode = static_cast<RT::Window::Mode>(mode | (state ? RT::Window::Resizable : RT::Window::Unknown));
            window->getAttribute("border", state);
            mode = static_cast<RT::Window::Mode>(mode | (state ? RT::Window::Border : RT::Window::Unknown));

            // Build dialog
            _dialogs[id] = context._engine.getRTPlatform().createWindow(viewport, title, mode);
        }

        MouCa::assertion(!_dialogs[id].expired());

        // Register dialog
        _manager.addRenderDialog(_dialogs[id]);
    }
}

void Engine3DXMLLoader::loadVR(ContextLoading& context)
{
    auto result = context._parser.getNode("VRHeadset");

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
    MouCa::preCondition(!device.expired()); //DEV Issue: Bad device !

    // Read surfaces
    auto result = context._parser.getNode("Surfaces");
    if (result->getNbElements() > 0)
    {
        MouCa::assertion(!_dialogs.empty());            //DEV Issue: Create surface/window without window ?
        MouCa::assertion(result->getNbElements() == 1); //DEV Issue: please clean xml ?

        auto aPushS = context._parser.autoPushNode(*result->getNode(0));

        auto allSurfaces = context._parser.getNode("Surface");
        for (size_t idSurface = 0; idSurface < allSurfaces->getNbElements(); ++idSurface)
        {
            auto surface = allSurfaces->getNode(idSurface);

            // Mandatory attribute
            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(surface, "Surface", _surfaces, context, existing);
            if (existing)
            {
                continue;
            }

            uint32_t windowId = 0;
            surface->getAttribute("windowId", windowId);
            if (_dialogs.find(windowId) == _dialogs.cend()) //DEV Issue: Must exist
            {
                throw Core::Exception(makeLoaderError(context, "UnknownSurfaceError"));
            }

            // Read user preferences
            Vulkan::SurfaceFormat::Configuration configuration;

            {
                auto aPush = context._parser.autoPushNode(*surface);

                auto allPreferences = context._parser.getNode("UserPreferences");                
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
                    
                    if(preferences->hasAttribute("presentationMode"))
                    {   
                        configuration._presentMode = LoaderHelper::readValue(preferences, "presentationMode", presentModes, false, context);
                    }
                    if(preferences->hasAttribute("usage"))
                    {
                        configuration._usage = LoaderHelper::readValue(preferences, "usage", imageUsages, true, context);
                    }
                    if (preferences->hasAttribute("transform"))
                    {
                        configuration._transform = LoaderHelper::readValue(preferences, "transform", surfaceTransforms, true, context);
                    }
                    if (preferences->hasAttribute("format"))
                    {
                        configuration._format.format = LoaderHelper::readValue(preferences, "format", formats, false, context);
                    }
                    if (preferences->hasAttribute("colorSpace"))
                    {
                        configuration._format.colorSpace = LoaderHelper::readValue(preferences, "colorSpace", colorSpaces, false, context);
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
    MouCa::preCondition(!deviceWeak.expired()); //DEV Issue: Bad device !

    // Read semaphores
    auto result = context._parser.getNode("Semaphores");
    if (result->getNbElements() > 0)
    {
        MouCa::assertion(result->getNbElements() == 1); //DEV Issue: please clean xml ?

        // cppcheck-suppress unreadVariable // false positive
        auto aPush = context._parser.autoPushNode(*result->getNode(0));

        auto device = deviceWeak.lock();

        auto allSemaphores = context._parser.getNode("Semaphore");
        for (size_t idSemaphore = 0; idSemaphore < allSemaphores->getNbElements(); ++idSemaphore)
        {
            auto semaphoreNode = allSemaphores->getNode(idSemaphore);
            // Mandatory attribute
            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(semaphoreNode, "Semaphore", _semaphores, context, existing);

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
    MouCa::preCondition(!deviceWeak.expired()); //DEV Issue: Bad device !

    // Read semaphores
    auto result = context._parser.getNode("Fences");
    if (result->getNbElements() > 0)
    {
        MouCa::assertion(result->getNbElements() == 1); //DEV Issue: please clean xml ?

        // cppcheck-suppress unreadVariable // false positive
        auto aPush = context._parser.autoPushNode(*result->getNode(0));

        auto device = deviceWeak.lock();

        auto allFences = context._parser.getNode("Fence");
        for (size_t idFence = 0; idFence < allFences->getNbElements(); ++idFence)
        {
            auto semaphoreNode = allFences->getNode(idFence);
            // Mandatory attribute
            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(semaphoreNode, "Fence", _fences, context, existing);

            const VkFenceCreateFlags flag = LoaderHelper::readValue(semaphoreNode, "flag", fenceCreates, true, context);

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
    MouCa::preCondition(!deviceWeak.expired()); //DEV Issue: Bad device !

    auto result = context._parser.getNode("FrameBuffers");
    if (result->getNbElements() > 0)
    {
        MouCa::assertion(result->getNbElements() == 1); //DEV Issue: please clean xml ?

        auto device = deviceWeak.lock();

        auto aPush = context._parser.autoPushNode(*result->getNode(0));

        auto allFrameBuffers = context._parser.getNode("FrameBuffer");
        for (size_t idFrameBuffer = 0; idFrameBuffer < allFrameBuffers->getNbElements(); ++idFrameBuffer)
        {
            auto frameBufferNode = allFrameBuffers->getNode(idFrameBuffer);
            // Search if FrameBuffer is existing before try to load
            uint32_t id=0;
            if (frameBufferNode->hasAttribute("id") || frameBufferNode->hasAttribute("externalId"))
            {
                bool existing;
                id = LoaderHelper::getIdentifiant(frameBufferNode, "FrameBuffer", _frameBuffers, context, existing);
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

                auto allAttachments = context._parser.getNode("Attachment");
                for (size_t idAttachment = 0; idAttachment < allAttachments->getNbElements(); ++idAttachment)
                {
                    auto attachmentNode = allAttachments->getNode(idAttachment);
                    if (attachmentNode->hasAttribute("viewImageId"))
                    {
                        const uint32_t viewImageId = LoaderHelper::getLinkedIdentifiant(attachmentNode, "viewImageId", _view, context);

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
                throw Core::Exception(makeLoaderError(context, "XMLFrameBufferMissAttachmentError"));
            }

            // Mandatory attribute
            const uint32_t renderPassId = LoaderHelper::getLinkedIdentifiant(frameBufferNode, "renderPassId", _renderPasses, context);
            
            // Build SwapChain framebuffer
            if (frameBufferNode->hasAttribute("surfaceId"))
            {
                MouCa::assertion(!frameBufferNode->hasAttribute("id")); //DEV Issue: Not supported id !

                // Get attached surface
                const uint32_t surfaceId = LoaderHelper::getLinkedIdentifiant(frameBufferNode, "surfaceId", _renderPasses, context);

                // Build FrameBuffer inside surface
                _surfaces[surfaceId].lock()->createFrameBuffer(_renderPasses[renderPassId], attachments);
            }
            else // Build basic Framebuffer
            {
                MouCa::assertion(frameBufferNode->hasAttribute("id")); //DEV Issue: New case ?

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
    MouCa::preCondition(!deviceWeak.expired()); //DEV Issue: Bad device !

    // Search Images
    auto result = context._parser.getNode("Images");
    if (result->getNbElements() > 0)
    {
        MouCa::assertion(result->getNbElements() == 1); //DEV Issue: please clean xml ?
        auto device = deviceWeak.lock();

        auto aPushI = context._parser.autoPushNode(*result->getNode(0));

        // Parsing all images
        auto allImages = context._parser.getNode("Image");
        for (size_t idImages = 0; idImages < allImages->getNbElements(); ++idImages)
        {
            auto imageNode = allImages->getNode(idImages);

            // Mandatory attribute
            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(imageNode, "Image", _images, context, existing);

            Vulkan::Image::Size size;
            VkFormat format = VK_FORMAT_UNDEFINED;
            VkImageType type = VK_IMAGE_TYPE_MAX_ENUM;
            VkImageUsageFlags usage = LoaderHelper::readValue(imageNode, "usage", imageUsages , true, context);

            // Build image from surface/swapchain size/format
            if (imageNode->hasAttribute("fromSurfaceId"))
            {
                const uint32_t idSC = LoaderHelper::getLinkedIdentifiant(imageNode, "fromSurfaceId", _surfaces, context);

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
                MouCa::assertion(!vrPlatform.isNull());
                //const uint32_t idSC = LoaderHelper::getLinkedIdentifiant(imageNode, "fromVRId", _surfaces, context);

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
                const uint32_t idSC = LoaderHelper::getLinkedIdentifiant(imageNode, "external", _cpuImages, context);
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
                    default: MouCa::assertion(false); // DEV Issue: Not implemented
                }
            }

            // Search format/size if not properly define or override
            if( imageNode->hasAttribute("extent") || !size.isValid() )
            {
                MouCa::assertion(context._globalData != nullptr);
                // Read name
                Core::String name;
                imageNode->getAttribute("extent", name);

                // Extract data
                auto resultExt = context._parser.searchNodeFrom(*context._globalData, "Data", "name", name.substr(1));
                LoaderHelper::readData(resultExt, size._extent);
            }
            if (imageNode->hasAttribute("format") || format == VK_FORMAT_UNDEFINED)
            {
                format = LoaderHelper::readValue(imageNode, "format", formats, false, context);
            }
            if (imageNode->hasAttribute("imageType") || type == VK_IMAGE_TYPE_MAX_ENUM)
            {
                type = LoaderHelper::readValue(imageNode, "imageType", imageTypes, false, context);
            }
            
            MouCa::assertion(format != VK_FORMAT_UNDEFINED);   // DEV Issue: Need a valid format.
            MouCa::assertion(size.isValid());                  // DEV Issue: Need a valid size.

            auto image = std::make_shared<Vulkan::Image>();
            image->initialize(device->getDevice(),
                              size, type, format,
                              LoaderHelper::readValue(imageNode, "samples",       samples, false, context),
                              LoaderHelper::readValue(imageNode, "tiling",        tilings, false, context),
                              usage,
                              LoaderHelper::readValue(imageNode, "sharingMode",   sharingModes, false, context),
                              LoaderHelper::readValue(imageNode, "initialLayout", imageLayouts, false, context),
                              LoaderHelper::readValue(imageNode, "memoryProperty", memoryProperties, true, context)
                             );
            // Register + ownership
            device->insertImage(image);
            _images[id] = image;
            MouCa::logConsole(std::format("Image: id={}, handle={:#08x}", id, reinterpret_cast<size_t>(image->getImage())));

            // Build view
            auto aPush = context._parser.autoPushNode(*imageNode);

            auto allViews = context._parser.getNode("View");
            for(size_t idView = 0; idView <allViews->getNbElements(); ++idView)
            {
                auto viewNode = allViews->getNode(idView);

                // Mandatory attribute
                const uint32_t idV = LoaderHelper::getIdentifiant(viewNode, "View", _view, context, existing);

                // Read parameter
                const VkImageViewType typeV   = LoaderHelper::readValue(viewNode, "viewType", viewTypes, false, context);
                const VkFormat        formatV = LoaderHelper::readValue(viewNode, "format",   formats, false, context);

                const VkComponentMapping mapping
                {
                    LoaderHelper::readValue(viewNode, "componentSwizzleRed",   componentSwizzles, false, context),
                    LoaderHelper::readValue(viewNode, "componentSwizzleGreen", componentSwizzles, false, context),
                    LoaderHelper::readValue(viewNode, "componentSwizzleBlue",  componentSwizzles, false, context),
                    LoaderHelper::readValue(viewNode, "componentSwizzleAlpha", componentSwizzles, false, context)
                };
                VkImageSubresourceRange subRessourceRange;
                subRessourceRange.aspectMask = LoaderHelper::readValue(viewNode, "aspectMask", aspectFlags, true, context);
                viewNode->getAttribute("baseMipLevel",   subRessourceRange.baseMipLevel);
                viewNode->getAttribute("levelCount",     subRessourceRange.levelCount);
                viewNode->getAttribute("baseArrayLayer", subRessourceRange.baseArrayLayer);
                viewNode->getAttribute("layerCount",     subRessourceRange.layerCount);

                // Build View
                auto view = image->createView(device->getDevice(), typeV, formatV, mapping, subRessourceRange);
                _view[idV] = view;

                MouCa::logConsole(std::format("View: id={}, handle={:#08x}", idV, reinterpret_cast<size_t>(view.lock()->getInstance())));
            }
        }
    }
}

void Engine3DXMLLoader::loadSamplers(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak)
{
    MouCa::preCondition(!deviceWeak.expired()); //DEV Issue: Bad device !

    // Search Buffers
    auto result = context._parser.getNode("Samplers");
    if (result->getNbElements() > 0)
    {
        MouCa::assertion(result->getNbElements() == 1); //DEV Issue: please clean xml ?
        auto device = deviceWeak.lock();

        auto aPushS = context._parser.autoPushNode(*result->getNode(0));

        // Parsing all samplers
        auto allSamplers = context._parser.getNode("Sampler");
        for (size_t idSampler = 0; idSampler < allSamplers->getNbElements(); ++idSampler)
        {
            auto samplerNode = allSamplers->getNode(idSampler);

            // Mandatory attribute
            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(samplerNode, "Sampler", _samplers, context, existing);
            if (existing)
            {
                continue;
            }

            // Read max LOD from image
            float maxLod = 0.0f;
            if (samplerNode->hasAttribute("imageId"))
            {
                const uint32_t idI = LoaderHelper::getLinkedIdentifiant(samplerNode, "imageId", _images, context);
                maxLod = static_cast<float>(_images[idI].lock()->getMaxMipLevels());
            }

            float mipLodBias;
            samplerNode->getAttribute("mipLodBias", mipLodBias);
            float minLod = 0.0f;
            samplerNode->getAttribute("minLod", minLod);
            if(samplerNode->hasAttribute("maxLod"))
            {
                samplerNode->getAttribute("maxLod", maxLod);
            }            
            bool unnormalizedCoordinates;
            samplerNode->getAttribute("unnormalizedCoordinates", unnormalizedCoordinates);

            // Read compare
            bool compareEnable = false;
            VkCompareOp compareOp = VK_COMPARE_OP_NEVER;
            if (samplerNode->hasAttribute("compareOp"))
            {
                compareEnable = true;
                compareOp = LoaderHelper::readValue(samplerNode, "compareOp", compareOperations, false, context);
            }            
            
            //Read anisotropy
            bool anisotropyEnable = false;
            float maxAnisotropy = 1.0f;
            if (samplerNode->hasAttribute("maxAnisotropy"))
            {
                anisotropyEnable = true;

                Core::String isDevice;
                samplerNode->getAttribute("maxAnisotropy", isDevice);
                if (isDevice == "device")
                {
                    // Use device limits
                    maxAnisotropy = device->getDevice().getPhysicalDeviceProperties().limits.maxSamplerAnisotropy;
                }
                else
                {
                    samplerNode->getAttribute("maxAnisotropy", maxAnisotropy);
                }
            }

            auto sampler = std::make_shared<Vulkan::Sampler>();
            sampler->initialize(device->getDevice(),
                LoaderHelper::readValue(samplerNode, "magFilter", filters, false, context),
                LoaderHelper::readValue(samplerNode, "minFilter", filters, false, context),
                LoaderHelper::readValue(samplerNode, "mipmapMode", samplerMipmaps, false, context),
                LoaderHelper::readValue(samplerNode, "addressModeU", samplerAdresses, false, context),
                LoaderHelper::readValue(samplerNode, "addressModeV", samplerAdresses, false, context),
                LoaderHelper::readValue(samplerNode, "addressModeW", samplerAdresses, false, context),
                mipLodBias, anisotropyEnable, maxAnisotropy, compareEnable, compareOp,
                minLod, maxLod,
                LoaderHelper::readValue(samplerNode, "borderColor", borderColors, false, context),
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
    MouCa::preCondition(!deviceWeak.expired()); //DEV Issue: Bad device !

    // Search Buffers
    auto result = context._parser.getNode("Buffers");
    if (result->getNbElements() > 0)
    {
        MouCa::assertion(result->getNbElements() == 1); //DEV Issue: please clean xml ?
        auto device = deviceWeak.lock();

        auto aPushB = context._parser.autoPushNode(*result->getNode(0));

        // Parsing all buffers
        auto allBuffers = context._parser.getNode("Buffer");
        for (size_t idBuffer = 0; idBuffer < allBuffers->getNbElements(); ++idBuffer)
        {
            auto bufferNode = allBuffers->getNode(idBuffer);
            auto aPushM = context._parser.autoPushNode(*bufferNode);

            // Mandatory attribute
            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(bufferNode, "Buffer", _buffers, context, existing);
            if (existing)
            {
                continue;
            }

            const auto usage          = LoaderHelper::readValue(bufferNode, "usage", bufferUsages, true, context);
            VkDeviceSize size=0;
            if(bufferNode->hasAttribute("size"))
            {
                bufferNode->getAttribute("size", size);
            }
            else if(bufferNode->hasAttribute("external"))
            {
                const uint32_t idE = LoaderHelper::getLinkedIdentifiant(bufferNode, "external", _cpuBuffers, context);
                size = _cpuBuffers[idE].lock()->getByteSize();
            }
            MouCa::assertion(size > 0);

            VkBufferCreateFlags createFlags = 0;
            if (bufferNode->hasAttribute("create"))
            {
                createFlags = LoaderHelper::readValue(bufferNode, "create", bufferCreates, true, context);
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
    auto result = context._parser.getNode("MemoryBuffer");
    if (result->getNbElements() > 0)
    {
        auto memoryNode = result->getNode(0);
        const auto memoryProperty = LoaderHelper::readValue(memoryNode, "property", memoryProperties, true, context);
        memoryBuffer = std::make_unique<Vulkan::MemoryBuffer>(memoryProperty);
    }
    else
    {
        auto resultAllo = context._parser.getNode("MemoryBufferAllocate");
        if (resultAllo->getNbElements() > 0)
        {
            auto memoryNode = resultAllo->getNode(0);
            const auto memoryProperty = LoaderHelper::readValue(memoryNode, "property", memoryProperties, true, context);
            const auto memoryAllocate = LoaderHelper::readValue(memoryNode, "allocate", memoryAllocates, true, context);
            memoryBuffer = std::make_unique<Vulkan::MemoryBufferAllocate>(memoryProperty, memoryAllocate);
        }
        else
        {
            throw Core::Exception(Core::ErrorData("Engine3D", "UnknownMemoryError"));
        }
    }
}


void Engine3DXMLLoader::loadGraphicsPipelines(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak)
{
    // Search Graphics Pipeline
    auto result = context._parser.getNode("GraphicsPipelines");
    if (result->getNbElements() > 0)
    {
        MouCa::assertion(result->getNbElements() == 1); //DEV Issue: please clean xml ?
        auto device = deviceWeak.lock();

        // cppcheck-suppress unreadVariable // false positive
        auto aPush = context._parser.autoPushNode(*result->getNode(0));

        // Parsing all Graphics pipeline
        auto allGraphicsPipelines = context._parser.getNode("GraphicsPipeline");
        for (size_t idGraphicsPipeline = 0; idGraphicsPipeline < allGraphicsPipelines->getNbElements(); ++idGraphicsPipeline)
        {
            auto graphicsPipelineNode = allGraphicsPipelines->getNode(idGraphicsPipeline);

            // Mandatory attribute
            bool existing;
            const uint32_t id  = LoaderHelper::getIdentifiant(graphicsPipelineNode, "GraphicsPipeline", _graphicsPipelines, context, existing);

            const uint32_t idR = LoaderHelper::getLinkedIdentifiant(graphicsPipelineNode, "renderPassId", _renderPasses, context);
            const uint32_t idL = LoaderHelper::getLinkedIdentifiant(graphicsPipelineNode, "pipelineLayoutId", _pipelineLayouts, context);

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
    MouCa::preCondition(!deviceWeak.expired()); //DEV Issue: Bad device !

    // Search QueueSequences
    auto result = context._parser.getNode("QueueSequences");
    if (result->getNbElements() > 0)
    {
        MouCa::assertion(result->getNbElements() == 1); //DEV Issue: please clean xml ?
        auto device = deviceWeak.lock();

        // cppcheck-suppress unreadVariable // false positive
        auto aPush = context._parser.autoPushNode(*result->getNode(0));

        // Parsing all images
        auto allQueueSequences = context._parser.getNode("QueueSequence");
        for (size_t idQueueSequences = 0; idQueueSequences < allQueueSequences->getNbElements(); ++idQueueSequences)
        {
            auto queueSequenceNode = allQueueSequences->getNode(idQueueSequences);

            // Mandatory attribute
            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(queueSequenceNode, "View", _queueSequences, context, existing);

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
    MouCa::preCondition(!deviceWeak.expired());        //DEV Issue: Bad device !
    MouCa::preCondition(!queueSequenceWeak.expired()); //DEV Issue: Bad container !

    auto queueSequence = queueSequenceWeak.lock();
    // Parsing all images
    auto allSequences = context._parser.getNode("Sequence");
    for (size_t idSequences = 0; idSequences < allSequences->getNbElements(); ++idSequences)
    {
        auto sequenceNode = allSequences->getNode(idSequences);

        // Get type of Sequence
        Core::String type;
        sequenceNode->getAttribute("type", type);

        // Build and initialize each sequence
        if(type == "acquire")
        {
            Vulkan::FenceWPtr     fence;
            Vulkan::SemaphoreWPtr semaphore;
            uint64_t timeout = std::numeric_limits<uint64_t>::max();

            // Read optional data
            if( sequenceNode->hasAttribute("fenceId") )
            {
                const uint32_t idF = LoaderHelper::getLinkedIdentifiant(sequenceNode, "fenceId", _fences, context);
                fence = _fences[idF];
            }
            if(sequenceNode->hasAttribute("timeout")) { sequenceNode->getAttribute("timeout", timeout); }
            if(sequenceNode->hasAttribute("semaphoreId"))
            {
                const uint32_t idSem = LoaderHelper::getLinkedIdentifiant(sequenceNode, "semaphoreId", _semaphores, context);
                semaphore = _semaphores[idSem];
            }

            // Build Sequence
            const uint32_t idS = LoaderHelper::getLinkedIdentifiant(sequenceNode, "surfaceId", _surfaces, context);
            queueSequence->emplace_back(std::make_shared<Vulkan::SequenceAcquire>(_surfaces[idS].lock()->getEditSwapChain(), semaphore, fence, timeout));
        }
        else if (type == "waitFence")
        {
            uint64_t timeout = std::numeric_limits<uint64_t>::max();

            // Read optional data
            if(sequenceNode->hasAttribute("timeout")) { sequenceNode->getAttribute("timeout", timeout); }
            
            bool waitAll;
            sequenceNode->getAttribute("waitAll", waitAll);

            // Read all fences associated
            std::vector<Vulkan::FenceWPtr> fences;
            auto aPush     = context._parser.autoPushNode(*sequenceNode);
            auto allFences = context._parser.getNode("Fence");
            for (size_t idFence = 0; idFence < allFences->getNbElements(); ++idFence)
            {
                auto fenceNode = allFences->getNode(idFence);
                const uint32_t idF = LoaderHelper::getLinkedIdentifiant(fenceNode, "fenceId", _fences, context);
                fences.emplace_back(_fences[idF]);
            }
            if (fences.empty())
            {
                throw Core::Exception(makeLoaderError(context, "XMLMissingNodeError") << "Sequence type=waitFence" << "Fence");
            }

            // Build Sequence
            queueSequence->emplace_back(std::make_shared<Vulkan::SequenceWaitFence>(fences, timeout, (waitAll ? VK_TRUE : VK_FALSE)));
        }
        else if (type == "resetFence")
        {
            // Read all fences associated
            std::vector<Vulkan::FenceWPtr> fences;
            auto aPush = context._parser.autoPushNode(*sequenceNode);
            auto allFences = context._parser.getNode("Fence");
            for (size_t idFence = 0; idFence < allFences->getNbElements(); ++idFence)
            {
                auto fenceNode = allFences->getNode(idFence);
                const uint32_t idF = LoaderHelper::getLinkedIdentifiant(fenceNode, "fenceId", _fences, context);
                fences.emplace_back(_fences[idF]);
            }
            if (fences.empty())
            {
                throw Core::Exception(makeLoaderError(context, "XMLMissingNodeError") << "Sequence type=waitFence" << "Fence");
            }

            // Build Sequence
            queueSequence->emplace_back(std::make_shared<Vulkan::SequenceResetFence>(fences));
        }
        else if (type == "submit")
        {
            Vulkan::FenceWPtr fence;
            if (sequenceNode->hasAttribute("fenceId"))
            {
                const uint32_t idF = LoaderHelper::getLinkedIdentifiant(sequenceNode, "fenceId", _fences, context);
                fence = _fences[idF];
            }

            // Build SubmitInfo
            Vulkan::SubmitInfos submitInfos;
            auto aPush = context._parser.autoPushNode(*sequenceNode);
            loadSubmitInfo(context, deviceWeak, submitInfos);

            // Build Sequence
            queueSequence->emplace_back(std::make_shared<Vulkan::SequenceSubmit>(std::move(submitInfos), fence));
        }
        else if (type == "submitVR")
        {
            auto& vrPlatform = context._engine.getVRPlatform();
            if (vrPlatform.isNull())
            {
                throw Core::Exception(Core::ErrorData("Engine3D", "VRNotReadyError") << "Sequence");
            }
            // Read parameters
            Core::String eye;
            sequenceNode->getAttribute("eye", eye);
            const bool isLeftEye = (eye == "left");

            const uint32_t idI = LoaderHelper::getLinkedIdentifiant(sequenceNode, "imageId", _images, context);
            auto image = _images[idI].lock();

            // Build Sequence
            const auto& device = deviceWeak.lock()->getDevice();
            queueSequence->emplace_back(std::make_shared <MouCaVR::Platform::SequenceSubmitVR>(vrPlatform, isLeftEye, _manager.getEnvironment(), device.getInstance(), device.getPhysicalDevice(), device.getQueue(), device.getQueueFamilyGraphicId(), image->getImage(), image->getSamples()));
        }
        else if (type == "presentKHR")
        {
            Vulkan::FenceWPtr fence;
            if (sequenceNode->hasAttribute("fenceId"))
            {
                const uint32_t idF = LoaderHelper::getLinkedIdentifiant(sequenceNode, "fenceId", _fences, context);
                fence = _fences[idF];
            }

            auto aPush = context._parser.autoPushNode(*sequenceNode);

            std::vector<Vulkan::SemaphoreWPtr> semaphores;
            std::vector<Vulkan::SwapChainWPtr> swapChains;

            // Parsing all Swapchain
            auto allSwapchains = context._parser.getNode("Swapchain");
            for (size_t idSwapchain = 0; idSwapchain < allSwapchains->getNbElements(); ++idSwapchain)
            {
                auto swapChainNode = allSwapchains->getNode(idSwapchain);

                // Read info
                const uint32_t idS = LoaderHelper::getLinkedIdentifiant(swapChainNode, "surfaceId", _surfaces, context);
                swapChains.emplace_back(_surfaces[idS].lock()->getEditSwapChain());
            }

            if( swapChains.empty() )
            {
                throw Core::Exception(makeLoaderError(context, "XMLMissingNodeError") << "Sequence type=presentKHR" << "Swapchain");
            }

            // Parsing all Signal Semaphore
            auto allSemaphores = context._parser.getNode("Semaphore");
            for (size_t idSemaphore = 0; idSemaphore < allSemaphores->getNbElements(); ++idSemaphore)
            {
                auto semaphoreNode = allSemaphores->getNode(idSemaphore);
                // Read info
                const uint32_t idSem = LoaderHelper::getLinkedIdentifiant(semaphoreNode, "semaphoreId", _semaphores, context);
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
            throw Core::Exception(makeLoaderError(context, "XMLUnknownSequenceError") << type);
        }
    }
}

void Engine3DXMLLoader::loadSubmitInfo(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak, Vulkan::SubmitInfos& submitInfos)
{
    MouCa::preCondition(!deviceWeak.expired());        //DEV Issue: Bad device !

    std::vector<Vulkan::WaitSemaphore>      waitSemaphores;
    std::vector<Vulkan::SemaphoreWPtr>      signalSemaphores;
    std::vector<Vulkan::ICommandBufferWPtr> commandBuffers;

    // Parsing all WaitSync
    auto allWaitSyncs = context._parser.getNode("WaitSync");
    for (size_t idWaitSync = 0; idWaitSync < allWaitSyncs->getNbElements(); ++idWaitSync)
    {
        auto waitSyncNode = allWaitSyncs->getNode(idWaitSync);

        // Read info
        const uint32_t idSem  = LoaderHelper::getLinkedIdentifiant(waitSyncNode, "semaphoreId", _semaphores, context);
        const auto pipelineFlag = LoaderHelper::readValue(waitSyncNode, "pipelineFlag", pipelineStageFlags, true, context);
        // Store
        waitSemaphores.emplace_back(Vulkan::WaitSemaphore(_semaphores[idSem], pipelineFlag));
    }

    // Parsing all Signal Semaphore
    auto allSignalSyncs = context._parser.getNode("SignalSync");
    for (size_t idSignalSync = 0; idSignalSync < allSignalSyncs->getNbElements(); ++idSignalSync)
    {
        auto signalSyncNode = allSignalSyncs->getNode(idSignalSync);

        // Read info
        const uint32_t idSem  = LoaderHelper::getLinkedIdentifiant(signalSyncNode, "semaphoreId", _semaphores, context);
        // Store
        signalSemaphores.emplace_back(_semaphores[idSem]);
    }

    // Parsing all Signal Semaphore
    auto allCommandBuffers = context._parser.getNode("CommandBuffer");
    for (size_t idCommandBuffer = 0; idCommandBuffer < allCommandBuffers->getNbElements(); ++idCommandBuffer)
    {
        auto commandBufferNode = allCommandBuffers->getNode(idCommandBuffer);

        if(commandBufferNode->hasAttribute("surfaceId"))
        {
            // Get surface associated
            const uint32_t idS = LoaderHelper::getLinkedIdentifiant(commandBufferNode, "surfaceId", _surfaces, context);
            
            // Prepare data
            auto surface = _surfaces[idS].lock();
            commandBuffers.emplace_back(surface->getICommandBuffer());
        }
        else
        {
            // Get surface associated
            const uint32_t idC = LoaderHelper::getLinkedIdentifiant(commandBufferNode, "id", _commandBuffers, context);
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
    auto result = context._parser.getNode("DescriptorSetLayouts");
    if (result->getNbElements() > 0)
    {
        MouCa::assertion(result->getNbElements() == 1); //DEV Issue: please clean xml ?
        auto device = deviceWeak.lock();

        auto aPush = context._parser.autoPushNode(*result->getNode(0));

        // Parsing all DescriptorSetLayout
        auto allDescriptorSetLayouts = context._parser.getNode("DescriptorSetLayout");
        for (size_t idDescriptorSetLayout = 0; idDescriptorSetLayout < allDescriptorSetLayouts->getNbElements(); ++idDescriptorSetLayout)
        {
            auto descriptorSetLayoutNode = allDescriptorSetLayouts->getNode(idDescriptorSetLayout);

            // Load mandatory 
            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(descriptorSetLayoutNode, "DescriptorSetLayout", _descriptorSetLayouts, context, existing);
            if (existing)
            {
                continue;
            }

            auto descriptorSetLayout = std::make_shared<Vulkan::DescriptorSetLayout>();

            // Parsing binding
            auto aPushD = context._parser.autoPushNode(*descriptorSetLayoutNode);
            auto allBindings = context._parser.getNode("Binding");
            for (size_t idBinding = 0; idBinding < allBindings->getNbElements(); ++idBinding)
            {
                auto bindingNode = allBindings->getNode(idBinding);
                
                uint32_t count;
                bindingNode->getAttribute("count", count);
                const auto flags = LoaderHelper::readValue(bindingNode, "shaderStageFlags", shaderStages, true, context);
                const auto type  = LoaderHelper::readValue(bindingNode, "type", descriptorTypes, false, context);
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
    auto result = context._parser.getNode("DescriptorPools");
    if (result->getNbElements() > 0)
    {
        MouCa::assertion(result->getNbElements() == 1); //DEV Issue: please clean xml ?
        auto device = deviceWeak.lock();

        auto aPush = context._parser.autoPushNode(*result->getNode(0));

        // Parsing all DescriptorSetLayout
        auto allDescriptorPools = context._parser.getNode("DescriptorPool");
        for (size_t idDescriptorPool = 0; idDescriptorPool < allDescriptorPools->getNbElements(); ++idDescriptorPool)
        {
            auto descriptorPoolNode = allDescriptorPools->getNode(idDescriptorPool);

            // Mandatory data
            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(descriptorPoolNode, "DescriptorPool", _descriptorPools, context, existing);
            if(existing)
            {
                continue;
            }
            uint32_t maxSets;
            descriptorPoolNode->getAttribute("maxSets", maxSets);

            // Parse Size
            auto aPushS = context._parser.autoPushNode(*descriptorPoolNode);
            auto allSizes = context._parser.getNode("Size");

            std::vector<VkDescriptorPoolSize> sizes;
            sizes.reserve(allSizes->getNbElements());
            for (size_t idSize = 0; idSize < allSizes->getNbElements(); ++idSize)
            {
                auto sizeNode = allSizes->getNode(idSize);
                VkDescriptorPoolSize poolSize;
                
                poolSize.type = LoaderHelper::readValue(sizeNode, "type", descriptorTypes, false, context);
                sizeNode->getAttribute("descriptorCount", poolSize.descriptorCount);

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
    auto result = context._parser.getNode("DescriptorSets");
    if (result->getNbElements() > 0)
    {
        MouCa::assertion(result->getNbElements() == 1); //DEV Issue: please clean xml ?
        auto device = deviceWeak.lock();

        auto aPush = context._parser.autoPushNode(*result->getNode(0));

        // Parsing all DescriptorSetLayout
        auto allDescriptorSets = context._parser.getNode("DescriptorSet");
        for (size_t idDescriptorSet = 0; idDescriptorSet < allDescriptorSets->getNbElements(); ++idDescriptorSet)
        {
            auto descriptorSetNode = allDescriptorSets->getNode(idDescriptorSet);

            // Load mandatory 
            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(descriptorSetNode, "DescriptorSet", _descriptorSets, context, existing);
            if (existing)
            {
                continue;
            }

            const uint32_t idPool = LoaderHelper::getLinkedIdentifiant(descriptorSetNode, "descriptorPoolId", _descriptorPools, context);
            
            // Parse SetLayout
            auto aPushS = context._parser.autoPushNode(*descriptorSetNode);
            auto allSetLayouts = context._parser.getNode("SetLayout");

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

                const uint32_t idS = LoaderHelper::getLinkedIdentifiant(dSetLayoutNode, "descriptorSetLayoutId", _descriptorSetLayouts, context);
                dSetLayouts.emplace_back(_descriptorSetLayouts[idS].lock()->getInstance());

                // Parse WriteDescriptor
                auto aPushSL = context._parser.autoPushNode(*dSetLayoutNode);
                
                auto allWriteDescriptors = context._parser.getNode("WriteDescriptor");
                for (size_t idWriteDescriptor = 0; idWriteDescriptor < allWriteDescriptors->getNbElements(); ++idWriteDescriptor)
                {
                    auto writeDescriptorNode = allWriteDescriptors->getNode(idWriteDescriptor);

                    // Read attribute
                    const auto     type    = LoaderHelper::readValue(writeDescriptorNode, "descriptorType", descriptorTypes, false, context);
                    uint32_t binding;
                    writeDescriptorNode->getAttribute("binding", binding);

                    // Parse WriteDescriptor
                    auto aPushW = context._parser.autoPushNode(*writeDescriptorNode);

                    // Build Buffer info
                    auto allBufferInfos = context._parser.getNode("BufferInfo");
                    if(allBufferInfos->getNbElements() > 0)
                    {
                        Vulkan::DescriptorBufferInfos buffers;
                        for (size_t idBufferInfo = 0; idBufferInfo < allBufferInfos->getNbElements(); ++idBufferInfo)
                        {
                            auto bufferInfoNode = allBufferInfos->getNode(idBufferInfo);

                            const auto idBuffer = LoaderHelper::getLinkedIdentifiant(bufferInfoNode, "bufferId", _buffers, context);

                            if (bufferInfoNode->hasAttribute("offset"))
                            {
                                VkDeviceSize offset;
                                bufferInfoNode->getAttribute("offset", offset);
                                VkDeviceSize range;
                                bufferInfoNode->getAttribute("range", range);

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
                    
                    auto allImageInfos = context._parser.getNode("ImageInfo");
                    if(allImageInfos->getNbElements() > 0)
                    {
                        Vulkan::DescriptorImageInfos images;
                        for (size_t idImageInfo = 0; idImageInfo < allImageInfos->getNbElements(); ++idImageInfo)
                        {
                            auto imageInfoNode = allImageInfos->getNode(idImageInfo);

                            const auto idSampler = LoaderHelper::getLinkedIdentifiant(imageInfoNode, "samplerId", _samplers, context);
                            const auto idView    = LoaderHelper::getLinkedIdentifiant(imageInfoNode, "viewId",    _view, context);

                            Vulkan::DescriptorImageInfo info
                            {
                                _samplers[idSampler].lock(),
                                _view[idView].lock(),
                                LoaderHelper::readValue(imageInfoNode, "layout", imageLayouts, false, context)
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
    auto result = context._parser.getNode("ShaderModules");
    if (result->getNbElements() > 0)
    {
        MouCa::assertion(result->getNbElements() == 1); //DEV Issue: please clean xml ?
        auto device = deviceWeak.lock();

        // cppcheck-suppress unreadVariable // false positive
        auto aPush = context._parser.autoPushNode(*result->getNode(0));

        // Parsing all ShaderModule
        auto allShaderModules = context._parser.getNode("ShaderModule");
        for (size_t idShaderModule = 0; idShaderModule < allShaderModules->getNbElements(); ++idShaderModule)
        {
            auto shaderModuleNode = allShaderModules->getNode(idShaderModule);

            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(shaderModuleNode, "ShaderModule", _shaderModules, context, existing);

            // Read info
            Core::String filename;
            shaderModuleNode->getAttribute("spirv", filename);

            Core::String source;
            if(shaderModuleNode->hasAttribute("code"))
            {
                shaderModuleNode->getAttribute("code", source);
            }

            // Create shader resources
            auto& resourceManager = context._resources;
            const auto stage = LoaderHelper::readValue(shaderModuleNode, "stage", shaderStages, false, context);

            auto shaderFile = resourceManager.openShader(filename, shaderKinds[stage], source);

            // If code source: enable tracking
            if(!source.empty())
            {
                // Register manage to file tracker
                resourceManager.getTracker().registerResource(shaderFile);
            }     

            // Build shader
            shaderFile->open();
            MouCa::assertion(shaderFile->isLoaded());

            auto shaderModule = std::make_shared<Vulkan::ShaderModule>();
            shaderModule->initialize(device->getDevice(), shaderFile->extractString(), "main", static_cast<VkShaderStageFlagBits>(stage));
            
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
    auto result = context._parser.getNode("RayTracingPipelines");
    if (result->getNbElements() > 0)
    {
        MouCa::assertion(result->getNbElements() == 1); //DEV Issue: please clean xml ?
        auto device = deviceWeak.lock();

        // cppcheck-suppress unreadVariable // false positive
        auto aPush = context._parser.autoPushNode(*result->getNode(0));

        // Parsing all ShaderModule
        auto allRayPipelines = context._parser.getNode("RayTracingPipeline");
        for (size_t idPipeline = 0; idPipeline < allRayPipelines->getNbElements(); ++idPipeline)
        {
            auto pipelineNode = allRayPipelines->getNode(idPipeline);

            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(pipelineNode, "RayTracingPipeline", _rayTracingPipelines, context, existing);
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
                throw Core::Exception(makeLoaderError(context, "XMLMissingNodeError") << "RayTracingPipeline" << "Stages/Stage");
            }

            // Groups
            loadRayTracingShaderGroup(context, deviceWeak, *pipeline);

            const uint32_t idL = LoaderHelper::getLinkedIdentifiant(pipelineNode, "pipelineLayoutId", _pipelineLayouts, context);
            uint32_t maxRecursive = 0;
            pipelineNode->getAttribute("maxPipelineRayRecursionDepth", maxRecursive);
            
            pipeline->initialize(device->getDevice(), _pipelineLayouts[idL], maxRecursive);

            device->insertRayTracingPipeline(pipeline);
            _rayTracingPipelines[id] = pipeline;
        }
    }
}

void Engine3DXMLLoader::loadRayTracingShaderGroup(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak, Vulkan::RayTracingPipeline& pipeline)
{
    auto groups = context._parser.getNode("Groups");
    if (groups->getNbElements() > 0)
    {
        MouCa::assertion(groups->getNbElements() == 1); //DEV Issue: Need to clean xml !

        // cppcheck-suppress unreadVariable // false positive
        auto aPush = context._parser.autoPushNode(*groups->getNode(0));

        // Parsing all Graphics pipeline
        auto allGroups = context._parser.getNode("Group");
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

            group.type = LoaderHelper::readValue(groupNode, "type", rayTracingGroupTypes, false, context);

            if(groupNode->hasAttribute("generalShader"))
            {
                groupNode->getAttribute("generalShader", group.generalShader);
            }
            if (groupNode->hasAttribute("closestHitShader"))
            {
                groupNode->getAttribute("closestHitShader", group.closestHitShader);
            }
            if (groupNode->hasAttribute("anyHitShader"))
            {
                groupNode->getAttribute("anyHitShader", group.anyHitShader);
            }
            if (groupNode->hasAttribute("intersectionShader"))
            {
                groupNode->getAttribute("intersectionShader", group.intersectionShader);
            }

            pipeline.addGroup(std::move(group));
        }
    }
}

void Engine3DXMLLoader::loadTracingRay(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak)
{
    auto tracingRays = context._parser.getNode("TracingRays");
    if (tracingRays->getNbElements() > 0)
    {
        MouCa::assertion(tracingRays->getNbElements() == 1); //DEV Issue: Need to clean xml !
        auto device = deviceWeak.lock();

        // cppcheck-suppress unreadVariable // false positive
        auto aPush = context._parser.autoPushNode(*tracingRays->getNode(0));

        // Parsing all Graphics pipeline
        auto allTracingRays = context._parser.getNode("TracingRay");
        for (size_t idTracingRay = 0; idTracingRay < allTracingRays->getNbElements(); ++idTracingRay)
        {
            auto tracingRayNode = allTracingRays->getNode(idTracingRay);

            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(tracingRayNode, "RayTracingPipeline", _tracingRays, context, existing);
            if (existing)
            {
                continue;
            }

            const uint32_t idPipeline = LoaderHelper::getLinkedIdentifiant(tracingRayNode, "rayTracingPipelines", _rayTracingPipelines, context);
            Vulkan::TracingRay::BufferSizes sizes{ 0, 0, 0, 0 };
            
            tracingRayNode->getAttribute("raygenSize",   sizes[0]);
            tracingRayNode->getAttribute("missSize",     sizes[1]);
            tracingRayNode->getAttribute("hitSize",      sizes[2]);
            tracingRayNode->getAttribute("callableSize", sizes[3]);

            auto tracingRay = std::make_shared<Vulkan::TracingRay>();
            tracingRay->initialize(device->getDevice(), _rayTracingPipelines[idPipeline], 0, sizes);

            device->insertTracingRay(tracingRay);
            _tracingRays[id] = tracingRay;
        }
    }
}

void Engine3DXMLLoader::loadAccelerationStructures(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak)
{
    auto accelerationStructures = context._parser.getNode("AccelerationStructures");
    if (accelerationStructures->getNbElements() > 0)
    {
        MouCa::assertion(accelerationStructures->getNbElements() == 1); //DEV Issue: Need to clean xml !
        auto device = deviceWeak.lock();

        // cppcheck-suppress unreadVariable // false positive
        auto aPush = context._parser.autoPushNode(*accelerationStructures->getNode(0));

        // Parsing all Graphics pipeline
        auto allAccelerationStructures = context._parser.getNode("AccelerationStructure");
        for (size_t idAccelerationStructure = 0; idAccelerationStructure < allAccelerationStructures->getNbElements(); ++idAccelerationStructure)
        {
            auto accelerationStructureNode = allAccelerationStructures->getNode(idAccelerationStructure);

            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(accelerationStructureNode, "AccelerationStructure", _accelerationStructures, context, existing);
            if (existing)
            {
                continue;
            }

            auto as = std::make_shared<Vulkan::AccelerationStructure>();

            const auto type = LoaderHelper::readValue(accelerationStructureNode, "type", accelerationStructureTypes, false, context);

            auto aPushA = context._parser.autoPushNode(*accelerationStructureNode);

            {
                auto allGeometries = context._parser.getNode("Geometry");
                for (size_t idGeometry = 0; idGeometry < allGeometries->getNbElements(); ++idGeometry)
                {
                    auto geometryNode = allGeometries->getNode(idGeometry);

                    const auto flag = LoaderHelper::readValue(geometryNode, "flag", geometryFlags, true, context );
                    Core::String typeG;
                    geometryNode->getAttribute("type", typeG);
                    if(typeG == "triangle")
                    {
                        const uint32_t idMesh = LoaderHelper::getLinkedIdentifiant(geometryNode, "meshId", _cpuMesh, context);

                        const uint32_t idVBO = LoaderHelper::getLinkedIdentifiant(geometryNode, "vboBufferId", _buffers, context);
                        const uint32_t idIBO = LoaderHelper::getLinkedIdentifiant(geometryNode, "iboBufferId", _buffers, context);
                        
                        auto triangle = std::make_unique<Vulkan::AccelerationStructureGeometryTriangles>();
                        triangle->initialize(_cpuMesh[idMesh], _buffers[idIBO], _buffers[idVBO], flag);

                        as->addGeometry(std::move(triangle));
                    }
                    else if (typeG == "instance")
                    {
                        auto aPushGI = context._parser.autoPushNode(*geometryNode);

                        auto allInstances = std::make_unique<Vulkan::AccelerationStructureGeometryInstance>();

                        auto allInstancesNode = context._parser.getNode("Instance");
                        for (size_t idInstance = 0; idInstance < allInstancesNode->getNbElements(); ++idInstance)
                        {
                            auto instanceNode = allInstancesNode->getNode(idInstance);
                            const uint32_t idAS = LoaderHelper::getLinkedIdentifiant(instanceNode, "accelerationStructureId", _accelerationStructures, context);

                            const auto flagI = LoaderHelper::readValue(instanceNode, "flag", geometryInstanceFlags, true, context );

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
                        throw Core::Exception(makeLoaderError(context, "UnknownASGeometryError") << "Geometry" << typeG);
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