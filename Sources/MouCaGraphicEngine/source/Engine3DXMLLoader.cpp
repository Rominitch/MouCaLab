#include "Dependencies.h"

#include "MouCaGraphicEngine/include/Engine3DXMLLoader.h"

#include <LibCore/include/CoreFileTracker.h>

#include <LibRT/include/RTImage.h>
#include <LibRT/include/RTMonitor.h>
#include <LibRT/include/RTRenderDialog.h>
#include <LibRT/include/RTShaderFile.h>

#include <LibGLFW/include/GLFWWindow.h>

#include <LibVR/include/VRPlatform.h>

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
#include <LibVulkan/include/VKRenderPass.h>
#include <LibVulkan/include/VKSampler.h>
#include <LibVulkan/include/VKSemaphore.h>
#include <LibVulkan/include/VKSequence.h>
#include <LibVulkan/include/VKSubmitInfo.h>
#include <LibVulkan/include/VKSwapChain.h>

#include <LibXML/include/XML.h>

#include "MouCaGraphicEngine/include/GraphicEngine.h"
#include "MouCaGraphicEngine/include/VulkanEnum.h"
#include "MouCaGraphicEngine/include/VulkanManager.h"

namespace MouCaGraphic
{

Engine3DXMLLoader::ContextLoading::ContextLoading(GraphicEngine& engine, XML::Parser& parser, MouCaCore::ResourceManager& resources):
_engine(engine), _parser(parser), _resources(resources), _xmlFileName(Core::convertToU8(parser.getFilename()))
{}

// Convert string to enum based on real integer or custom string or many using |
// Supported Format:
//   Integer number: aspectMask = "6"
//   LiteralFormat:  aspectMask = "VK_IMAGE_ASPECT_DEPTH_BIT"
//         or with | aspectMask = "VK_IMAGE_ASPECT_DEPTH_BIT|VK_IMAGE_ASPECT_STENCIL_BIT"
template<typename VulkanEnum>
VulkanEnum readValue(XML::NodeUPtr& node, const Core::String& attribute, const std::map<Core::String, VulkanEnum>& literalEnum, bool bitMode, Engine3DXMLLoader::ContextLoading& context)
{
    Core::String value;
    node->getAttribute(attribute, value);

    if (value.empty())
    {
        MOUCA_THROW_ERROR_2(u8"Engine3D", u8"XMLEmptyAttributeStringError", context.getFileName(), attribute);
    }

    auto data = static_cast<VulkanEnum>(0); // Set to 0 to make | operator
    try
    {
        // Here we don't verify if integer exists: can be a combination or future value.
        // Note: Vulkan enum class would be fine (when possible).
        data = static_cast<VulkanEnum>(std::stoll(value));
    }
    catch (...)
    {
        // Search if variable
        if (value[0] == '$')
        {
            if (context._globalData == nullptr)
            {
                MOUCA_THROW_ERROR_1(u8"Engine3D", u8"XMLMissingGlobalData", context.getFileName());
            }

            // Search global data
            auto result = context._parser.searchNodeFrom(*context._globalData, u8"Data", u8"name", value.substr(1));
            // Make same job on another node
            return readValue(result, attribute, literalEnum, bitMode, context);
        }
        else
        {
            std::vector<Core::String> results;
            // Possibly | inside
            if (bitMode)
            {
                boost::split(results, value, [](const auto& c) {return c == u8'|'; });
            }
            else
            {
                results.emplace_back(value);
            }
            for (const auto& part : results)
            {
                auto itLiteral = literalEnum.find(part);
                if (itLiteral == literalEnum.cend())
                {
                    MOUCA_THROW_ERROR_3(u8"Engine3D", u8"XMLUnknownAttributeStringError", context.getFileName(), attribute, part);
                }
                data = static_cast<VulkanEnum>(data | itLiteral->second);
            }
        }
    }
    return data;
}

template<typename DataType>
void readAttribute(const XML::NodeUPtr& node, const Core::String& attribute, DataType& value, Engine3DXMLLoader::ContextLoading& context)
{
    Core::String data;
    node->getAttribute(attribute, data);

    if (data[0] == '$')
    {
        if (context._globalData == nullptr)
        {
            MOUCA_THROW_ERROR_1(u8"Engine3D", u8"XMLMissingGlobalData", context.getFileName());
        }

        // Search global data
        auto result = context._parser.searchNodeFrom(*context._globalData, u8"Data", u8"name", data.substr(1));
        // Make same job on another node
        return result->getAttribute(attribute, value);
    }
    else
        node->getAttribute(attribute, value);
}

template<typename DataType>
void readData(const XML::NodeUPtr& node, DataType& value)
{
    MOUCA_THROW_ERROR(u8"BasicError", u8"DevImplementationMissing");
}

template<>
void readData(const XML::NodeUPtr& node, VkExtent3D& extent)
{
    node->getAttribute(u8"width",  extent.width);
    node->getAttribute(u8"height", extent.height);
    node->getAttribute(u8"depth",  extent.depth);
}

template<typename VulkanTypeWeak>
uint32_t getIdentifiant(const XML::NodeUPtr& node, const Core::String& nodeName, const std::map<uint32_t, VulkanTypeWeak>& container, Engine3DXMLLoader::ContextLoading& context, bool& existing)
{
    uint32_t id = 0;
    if (node->hasAttribute(u8"externalId"))
    {
        existing = true;
        node->getAttribute(u8"externalId", id);
        if (container.find(id) == container.cend()) //DEV Issue: Must not exist
        {
            MOUCA_THROW_ERROR_3(u8"Engine3D", u8"XMLUnknownIdentifiantError", context.getFileName(), nodeName + u8":externalId", std::to_string(id));
        }
    }
    else
    {
        existing = false;
        node->getAttribute(u8"id", id);
        if (container.find(id) != container.cend()) //DEV Issue: Must not exist
        {
            MOUCA_THROW_ERROR_3(u8"Engine3D", u8"XMLAlreadyExistingIdError", context.getFileName(), std::to_string(id), u8"id");
        }
    }
    return id;
}

template<typename VulkanTypeWeak>
uint32_t getLinkedIdentifiant(const XML::NodeUPtr& node, const Core::String& idName, const std::map<uint32_t, VulkanTypeWeak>& container, Engine3DXMLLoader::ContextLoading& context)
{
    uint32_t id = 0;
    node->getAttribute(idName, id);
    if (container.find(id) == container.cend()) //DEV Issue: Must exist
    {
        MOUCA_THROW_ERROR_3(u8"Engine3D", u8"XMLUnknownIdentifiantError", context.getFileName(), idName, std::to_string(id));
    }
    return id;
}

void createCharExtensions(std::vector<Core::String>& extensions, std::vector<const char*>& cExtensions)
{
    // Need to convert list (MUST keep old to get valid memory)
    cExtensions.resize(extensions.size());
    auto cExt = cExtensions.begin();
    for (auto& ext : extensions)
    {
        *cExt = ext.c_str();
        ++cExt;
    }
}

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

    // Prepare all surfaces
    // Build new window
    loadWindows(context);

    // Build new VR
    loadVR(context);

    // Check if we need to load environment
    if (_manager.getEnvironment().isNull())
    {
        loadEnvironment(context);
    }

    // Prepare global data node
    auto datas = context._parser.getNode(u8"GlobalData");
    if (datas->getNbElements() > 0 )
    {
        MOUCA_ASSERT(datas->getNbElements() == 1);
        context._globalData = datas->getNode(0);
    }

    // Create device if possible
    loadDevices(context);
}

void Engine3DXMLLoader::loadEnvironment(ContextLoading& context)
{
    MOUCA_PRE_CONDITION(context._parser.isLoaded());           //DEV Issue: Need a valid xml.
    MOUCA_PRE_CONDITION(_manager.getEnvironment().isNull());    //DEV Issue: Environment MUST be not configured.

    // Check node exists
    auto result = context._parser.getNode(u8"Environment");
    if (result->getNbElements() == 0)
    {
        MOUCA_THROW_ERROR_2(u8"Engine3D", u8"LoadingXMLCorruptError", context.getFileName(), u8"Environment");
    }

    // Go into
    XML::NodeUPtr environmentNode = result->getNode(0);
    // cppcheck-suppress unreadVariable // false positive
    auto aPush = context._parser.autoPushNode(*environmentNode);

    // Get extensions: WARNING extensions must exists to validate cExtensions;
    std::vector<Core::String> extensions;

    // Check if we need to load VR support
    auto& vrPlatform = context._engine.getVRPlatform();
    if (!vrPlatform.isNull())
    {
        extensions = vrPlatform.getInstanceExtensions();
    }

    loadExtensions(context, extensions);

    std::vector<const char*> cExtensions;
    createCharExtensions(extensions, cExtensions);

    // Build environment
    _manager.getEnvironment().initialize(context._info, cExtensions);

    MOUCA_POST_CONDITION(!_manager.getEnvironment().isNull());  //DEV Issue: Environment MUST be configured.
}

void Engine3DXMLLoader::loadExtensions(ContextLoading& context, std::vector<Core::String>& extensions)
{
    // Read extensions
    auto result = context._parser.getNode(u8"Extension");

    // Allocate more space
    extensions.reserve(extensions.size() + result->getNbElements());

    for (size_t idExtension = 0; idExtension < result->getNbElements(); ++idExtension)
    {
        Core::String osName;
        auto extension = result->getNode(idExtension);
        if (extension->hasAttribute(u8"os"))
        {
            extension->getAttribute(u8"os", osName);
        }
        else // When attribute is not present consider like current os to add it.
        {
            osName = VulkanManager::_osName;
        }

        if (osName == VulkanManager::_osName)
        {
            Core::String extensionValue;
            extension->getValue(extensionValue);
            extensions.emplace_back(std::move(extensionValue));
        }
    }
}

void Engine3DXMLLoader::loadDevices(ContextLoading& context)
{
    MOUCA_PRE_CONDITION(context._parser.isLoaded()); //DEV Issue: Need a valid xml.

    auto result = context._parser.getNode(u8"Device");
    for (size_t idDevice = 0; idDevice < result->getNbElements(); ++idDevice)
    {
        auto device = result->getNode(idDevice);

        Vulkan::ContextDeviceWPtr ctxDevice;
        bool existing = false;
        const uint32_t id = getIdentifiant(device, u8"Device", _devices, context, existing);
        if (!existing)
        {
            Core::String mode;
            device->getAttribute(u8"mode", mode);
            if (mode == u8"render")
            {
                uint32_t idSurface = 0;
                device->getAttribute(u8"compatibleWindowId", idSurface);

                if (idSurface >= _manager.getSurfaces().size())
                {
                    MOUCA_THROW_ERROR_1(u8"Engine3D", u8"UnknownSurfaceError", context.getFileName());
                }

                auto aPush = context._parser.autoPushNode(*device);

                // Get extensions: WARNING extensions must exists to validate cExtensions;
                std::vector<Core::String> extensions;
                auto& vrPlatform = context._engine.getVRPlatform();
                if (!vrPlatform.isNull())
                {
                    extensions = vrPlatform.getPhysicalDeviceExtensions(vrPlatform.getVulkanPhysicalDevice(_manager.getEnvironment()));
                }
                loadExtensions(context, extensions);
                std::vector<const char*> cExtensions;
                createCharExtensions(extensions, cExtensions);

                // Check mandatory features
                VkPhysicalDeviceFeatures mandatory = {};

                // Build device
                ctxDevice = _manager.createRenderingDevice(cExtensions, mandatory, *_manager.getSurfaces().at(idSurface));
                // Register
                _devices[id] = ctxDevice; 

                // Load all device's data (order is important)
                loadSurfaces(context, ctxDevice);
            }
            else
            {
                MOUCA_THROW_ERROR_2(u8"Engine3D", u8"DeviceModeError", context.getFileName(), mode);
            }
        }
        else
        {
            ctxDevice = _devices[id];
        }
        MOUCA_ASSERT(!ctxDevice.expired()); // Bad creation/ bad transfer ?

        // Load all device's data (order is important)
        loadShaderModules(context, ctxDevice);

        loadSemaphores(context, ctxDevice);

        loadFences(context, ctxDevice);

        loadImagesAndView(context, ctxDevice);

        loadSamplers(context, ctxDevice);

        loadBuffers(context, ctxDevice);

        loadRenderPasses(context, ctxDevice);

        loadFrameBuffers(context, ctxDevice);

        loadDescriptorSetPools(context, ctxDevice);

        loadDescriptorSetLayouts(context, ctxDevice);

        loadDescriptorSets(context, ctxDevice);

        loadPipelineLayouts(context, ctxDevice);

        loadGraphicsPipelines(context, ctxDevice);

        loadCommandPools(context, ctxDevice);

        loadCommandsGroup(context, ctxDevice);

        loadCommandBuffers(context, ctxDevice);

        loadQueueSequences(context, ctxDevice);
    }
}

void Engine3DXMLLoader::loadWindows(ContextLoading& context)
{
    MOUCA_PRE_CONDITION(_dialogs.empty());

    auto result = context._parser.getNode(u8"Window");
    for (size_t idWindow = 0; idWindow < result->getNbElements(); ++idWindow)
    {
        auto window = result->getNode(idWindow);
        bool existing;
        const uint32_t id = getIdentifiant(window, u8"Window", _dialogs, context, existing);
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

            window->getAttribute(u8"width", posX);
            window->getAttribute(u8"height", posY);
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
            const uint32_t id = getIdentifiant(surface, u8"Surface", _surfaces, context, existing);
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
                        configuration._presentMode = readValue(preferences, u8"presentationMode", presentModes, false, context);
                    }
                    if(preferences->hasAttribute(u8"usage"))
                    {
                        configuration._usage = readValue(preferences, u8"usage", imageUsages, true, context);
                    }
                    if (preferences->hasAttribute(u8"transform"))
                    {
                        configuration._transform = readValue(preferences, u8"transform", surfaceTransforms, true, context);
                    }
                    if (preferences->hasAttribute(u8"format"))
                    {
                        configuration._format.format = readValue(preferences, u8"format", formats, false, context);
                    }
                    if (preferences->hasAttribute(u8"colorSpace"))
                    {
                        configuration._format.colorSpace = readValue(preferences, u8"colorSpace", colorSpaces, false, context);
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
            const uint32_t id = getIdentifiant(semaphoreNode, u8"Semaphore", _semaphores, context, existing);

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
            const uint32_t id = getIdentifiant(semaphoreNode, u8"Fence", _fences, context, existing);

            const VkFenceCreateFlags flag = readValue(semaphoreNode, u8"flag", fenceCreates, true, context);

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
                id = getIdentifiant(frameBufferNode, u8"FrameBuffer", _frameBuffers, context, existing);
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
                        const uint32_t viewImageId = getLinkedIdentifiant(attachmentNode, u8"viewImageId", _view, context);

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
            const uint32_t renderPassId = getLinkedIdentifiant(frameBufferNode, u8"renderPassId", _renderPasses, context);
            
            // Build SwapChain framebuffer
            if (frameBufferNode->hasAttribute(u8"surfaceId"))
            {
                MOUCA_ASSERT(!frameBufferNode->hasAttribute(u8"id")); //DEV Issue: Not supported id !

                // Get attached surface
                const uint32_t surfaceId = getLinkedIdentifiant(frameBufferNode, u8"surfaceId", _renderPasses, context);

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

struct SubPassAttachment
{
    void buildColorAttachments(size_t size)
    {
        colorAttachmentReferences.resize(size);
    }

    void buildAttachments(std::vector<VkAttachmentReference> & attachmentReferences)
    {
        MOUCA_PRE_CONDITION(!colorAttachmentReferences.empty());
        if (attachmentReferences.empty())
        {
            attachmentReferences.resize(colorAttachmentReferences.size());

            const VkAttachmentReference empty{ VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_UNDEFINED };
            std::fill(attachmentReferences.begin(), attachmentReferences.end(), empty);
        }
    }

    void buildDepthAttachments()
    {
        buildAttachments(depthAttachmentReferences);
    }

    void buildResolveAttachments()
    {
        buildAttachments(resolveAttachmentReferences);
    }

    std::vector<VkAttachmentReference> colorAttachmentReferences;
    std::vector<VkAttachmentReference> depthAttachmentReferences;
    std::vector<VkAttachmentReference> resolveAttachmentReferences;

    std::vector<VkAttachmentReference> inputAttachmentReferences;

    std::vector<uint32_t> preserveAttachmentReferences;
};

void Engine3DXMLLoader::loadRenderPasses(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak)
{
    MOUCA_PRE_CONDITION(!deviceWeak.expired()); //DEV Issue: Bad device !

    auto result = context._parser.getNode(u8"RenderPasses");
    if (result->getNbElements() > 0)
    {
        MOUCA_ASSERT(result->getNbElements() == 1); //DEV Issue: please clean xml ?

        auto aPushR = context._parser.autoPushNode(*result->getNode(0));

        auto device = deviceWeak.lock();

        auto allRenderPasses = context._parser.getNode(u8"RenderPass");
        for (size_t idRenderPass = 0; idRenderPass < allRenderPasses->getNbElements(); ++idRenderPass)
        {
            auto renderPassNode = allRenderPasses->getNode(idRenderPass);
            // Mandatory attribute
            bool existing;
            const uint32_t id = getIdentifiant(renderPassNode, u8"RenderPass", _renderPasses, context, existing);
            if (existing)
                continue;

            auto aPush = context._parser.autoPushNode(*renderPassNode);

            // Load data (WARNING: vector MUST be valid/allocated until renderPass->initialize)
            std::vector<VkAttachmentDescription> attachments;
            loadRenderPassAttachment(context, id, attachments);

            std::vector<SubPassAttachment> attachmentReferences;
            std::vector<VkSubpassDescription> subpasses;
            loadRenderPassSubPass(context, id, subpasses, attachmentReferences);

            std::vector<VkSubpassDependency> dependencies;
            loadRenderPassDependency(context, id, dependencies, subpasses);

            // Build render pass
            auto renderPass = std::make_shared<Vulkan::RenderPass>();
            renderPass->initialize(device->getDevice(), std::move(attachments), std::move(subpasses), std::move(dependencies));

            // Register + ownership
            device->insertRenderPass(renderPass);

            _renderPasses[id] = renderPass;
        }
    }
}

void Engine3DXMLLoader::loadRenderPassAttachment(ContextLoading& context, const uint32_t id, std::vector<VkAttachmentDescription>& attachments)
{
    auto allAttachments = context._parser.getNode(u8"Attachment");

    attachments.resize(allAttachments->getNbElements());
    for (size_t idAttachment = 0; idAttachment < allAttachments->getNbElements(); ++idAttachment)
    {
        auto attachmentNode = allAttachments->getNode(idAttachment);
        
        auto& attachment = attachments[idAttachment];
        // Extract VKImage format (from swapchain image or image)
        if (attachmentNode->hasAttribute(u8"surfaceId"))
        {
            const uint32_t surfaceId = getLinkedIdentifiant(attachmentNode, u8"surfaceId", _surfaces, context);

            // Use format of swapchain
            attachment.format = _surfaces[surfaceId].lock()->getFormat().getConfiguration()._format.format;
        }
        else if (attachmentNode->hasAttribute(u8"imageId"))
        {
            const uint32_t imageId = getLinkedIdentifiant(attachmentNode, u8"imageId", _images, context);

            // Use format of image
            attachment.format = _images[imageId].lock()->getFormat();
        }
        else
        {
            MOUCA_THROW_ERROR_2(u8"Engine3D", u8"XMLRenderPassFormatError", context.getFileName(), std::to_string(id));
        }
        MOUCA_ASSERT(attachment.format != VK_FORMAT_UNDEFINED);

        // Read sampler
        attachment.samples = readValue(attachmentNode, u8"samples", samples, false, context);
        MOUCA_ASSERT(Core::Maths::isPowerOfTwo(static_cast<uint32_t>(attachment.samples)));

        // Read load/store
        attachment.loadOp         = readValue(attachmentNode, u8"loadOp",        attachmentLoads,  false, context);
        attachment.storeOp        = readValue(attachmentNode, u8"storeOp",       attachmentStores, false, context);
        attachment.stencilLoadOp  = readValue(attachmentNode, u8"stencilLoadOp", attachmentLoads,  false, context);
        attachment.stencilStoreOp = readValue(attachmentNode, u8"stencilSaveOp", attachmentStores, false, context);

        // Read layout
        attachment.initialLayout = readValue(attachmentNode, u8"initialLayout",  imageLayouts, false, context);
        attachment.finalLayout   = readValue(attachmentNode, u8"finalLayout",    imageLayouts, false, context);
    }

    // Manage errors
    if (attachments.empty())
    {
        MOUCA_THROW_ERROR_2(u8"Engine3D", u8"XMLRenderPassMissAttachmentError", context.getFileName(), std::to_string(id));
    }
}

void Engine3DXMLLoader::loadRenderPassSubPass(ContextLoading& context, const uint32_t id, std::vector<VkSubpassDescription>& subPasses, std::vector<SubPassAttachment>& attachmentReferences)
{
    auto allSubPasses = context._parser.getNode(u8"SubPass");

    // Allocated memory
    subPasses.resize(allSubPasses->getNbElements());
    attachmentReferences.resize(allSubPasses->getNbElements());

    for (size_t idSubPass = 0; idSubPass < allSubPasses->getNbElements(); ++idSubPass)
    {
        auto subPassNode       = allSubPasses->getNode(idSubPass);
        auto& subPass          = subPasses[idSubPass];
        auto& attachments      = attachmentReferences[idSubPass];

        // Read data
        subPass.pipelineBindPoint = readValue(subPassNode, u8"bindPoint", bindPoints, false, context);

        auto aPush = context._parser.autoPushNode(*subPassNode);

        // Read color attachment
        auto allColorAttachments = context._parser.getNode(u8"ColorAttachment");
        attachments.buildColorAttachments(allColorAttachments->getNbElements());
        for (size_t idAttachment = 0; idAttachment < allColorAttachments->getNbElements(); ++idAttachment)
        {
            auto colorAttachmentNode = allColorAttachments->getNode(idAttachment);

            // Read color Attachment
            attachments.colorAttachmentReferences[idAttachment] =
            {
                readValue(colorAttachmentNode, u8"colorAttachment", subPassAttachmentHelper, false, context),
                readValue(colorAttachmentNode, u8"colorLayout", imageLayouts, false, context)
            };

            // Read depth/stencil Attachment
            if (colorAttachmentNode->hasAttribute(u8"depthStencilAttachment"))
            {
                attachments.buildDepthAttachments();

                attachments.depthAttachmentReferences[idAttachment] =
                {
                    readValue(colorAttachmentNode, u8"depthStencilAttachment", subPassAttachmentHelper, false, context),
                    readValue(colorAttachmentNode, u8"depthStencilLayout", imageLayouts, false, context)
                };
            }
            // Read resolve Attachment (optional)
            if (colorAttachmentNode->hasAttribute(u8"resolveAttachment"))
            {
                // Allocate now buffer (re-entrance do nothing)
                attachments.buildResolveAttachments();

                attachments.colorAttachmentReferences[idAttachment] =
                {
                    readValue(colorAttachmentNode, u8"resolveAttachment", subPassAttachmentHelper, false, context),
                    readValue(colorAttachmentNode, u8"resolveLayout", imageLayouts, false, context)
                };
            }
        }

        if (attachments.colorAttachmentReferences.empty())
        {
            MOUCA_THROW_ERROR_2(u8"Engine3D", u8"XMLRenderPassMissColorAttachmentError", context.getFileName(), std::to_string(idSubPass));
        }

        // Read color attachment
        auto allInputAttachments = context._parser.getNode(u8"InputAttachment");
        attachments.inputAttachmentReferences.resize(allInputAttachments->getNbElements());
        for (size_t idAttachment = 0; idAttachment < allInputAttachments->getNbElements(); ++idAttachment)
        {
            auto inputAttachmentNode = allInputAttachments->getNode(idAttachment);

            // Read depth/stencil Attachment
            attachments.inputAttachmentReferences[idAttachment] =
            {
                readValue(inputAttachmentNode, u8"attachment", subPassAttachmentHelper, false, context),
                readValue(inputAttachmentNode, u8"layout",     imageLayouts, false, context)
            };
        }

        // Read preserve attachment
        auto allPreserveAttachments = context._parser.getNode(u8"PreserveAttachment");
        attachments.preserveAttachmentReferences.resize(allPreserveAttachments->getNbElements());
        for (size_t idAttachment = 0; idAttachment < allPreserveAttachments->getNbElements(); ++idAttachment)
        {
            auto preserveAttachmentNode = allPreserveAttachments->getNode(idAttachment);
            attachments.preserveAttachmentReferences[idAttachment] = readValue(preserveAttachmentNode, u8"attachment", subPassAttachmentHelper, false, context);
        }

        // Link array to data
        subPass.colorAttachmentCount = static_cast<uint32_t>(attachments.colorAttachmentReferences.size());
        subPass.pColorAttachments       = attachments.colorAttachmentReferences.data();
        subPass.pDepthStencilAttachment = attachments.depthAttachmentReferences.empty()
                                        ? VK_NULL_HANDLE
                                        : attachments.depthAttachmentReferences.data();
        subPass.pResolveAttachments     = attachments.resolveAttachmentReferences.empty()
                                        ? VK_NULL_HANDLE
                                        : attachments.resolveAttachmentReferences.data();

        subPass.inputAttachmentCount = static_cast<uint32_t>(attachments.inputAttachmentReferences.size());
        subPass.pInputAttachments    = attachments.inputAttachmentReferences.data();

        subPass.preserveAttachmentCount = static_cast<uint32_t>(attachments.preserveAttachmentReferences.size());
        subPass.pPreserveAttachments    = attachments.preserveAttachmentReferences.data();
    }

    if (subPasses.empty())
    {
        MOUCA_THROW_ERROR_2(u8"Engine3D", u8"XMLRenderPassMissSubpassError", context.getFileName(), std::to_string(id));
    }
}

void Engine3DXMLLoader::loadRenderPassDependency(ContextLoading& context, const uint32_t id, std::vector<VkSubpassDependency>& dependencies, const std::vector<VkSubpassDescription>& subPass)
{
    auto allDependencies = context._parser.getNode(u8"Dependency");

    dependencies.reserve(allDependencies->getNbElements());
    for (size_t idDependency = 0; idDependency < allDependencies->getNbElements(); ++idDependency)
    {
        auto dependencyNode = allDependencies->getNode(idDependency);
        const VkSubpassDependency subDependency
        {
            readValue(dependencyNode, u8"srcSubpass",      subPassHelper,      false, context),
            readValue(dependencyNode, u8"dstSubpass",      subPassHelper,      false, context),
            readValue(dependencyNode, u8"srcStageMask",    pipelineStageFlags, true,  context),
            readValue(dependencyNode, u8"dstStageMask",    pipelineStageFlags, true,  context),
            readValue(dependencyNode, u8"srcAccessMask",   accessFlags,        true,  context),
            readValue(dependencyNode, u8"dstAccessMask",   accessFlags,        true,  context),
            readValue(dependencyNode, u8"dependencyFlags", dependencyFlags,    true,  context),
        };

        if(subDependency.dstSubpass != VK_SUBPASS_EXTERNAL && subDependency.dstSubpass >= subPass.size())
        {
            MOUCA_THROW_ERROR_4(u8"Engine3D", u8"XMLRenderPassUnknownSubpassError", context.getFileName(), std::to_string(id), u8"dstSubpass", std::to_string(subDependency.dstSubpass));
        }
        if(subDependency.srcSubpass != VK_SUBPASS_EXTERNAL && subDependency.srcSubpass >= subPass.size())
        {
            MOUCA_THROW_ERROR_4(u8"Engine3D", u8"XMLRenderPassUnknownSubpassError", context.getFileName(), std::to_string(id), u8"srcSubpass", std::to_string(subDependency.srcSubpass));
        }
        dependencies.emplace_back(subDependency);
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
            const uint32_t id = getIdentifiant(imageNode, u8"Image", _images, context, existing);

            Vulkan::Image::Size size;
            VkFormat format = VK_FORMAT_UNDEFINED;
            VkImageType type = VK_IMAGE_TYPE_MAX_ENUM;
            VkImageUsageFlags usage = readValue(imageNode, u8"usage", imageUsages , true, context);

            // Build image from surface/swapchain size/format
            if (imageNode->hasAttribute("fromSurfaceId"))
            {
                const uint32_t idSC = getLinkedIdentifiant(imageNode, u8"fromSurfaceId", _surfaces, context);

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
                //const uint32_t idSC = getLinkedIdentifiant(imageNode, u8"fromVRId", _surfaces, context);

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
                const uint32_t idSC = getLinkedIdentifiant(imageNode, u8"external", _cpuImages, context);
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
                readData(resultExt, size._extent);
            }
            if (imageNode->hasAttribute(u8"format") || format == VK_FORMAT_UNDEFINED)
            {
                format = readValue(imageNode, u8"format", formats, false, context);
            }
            if (imageNode->hasAttribute(u8"imageType") || type == VK_IMAGE_TYPE_MAX_ENUM)
            {
                type = readValue(imageNode, u8"imageType", imageTypes, false, context);
            }
            
            MOUCA_ASSERT(format != VK_FORMAT_UNDEFINED);   // DEV Issue: Need a valid format.
            MOUCA_ASSERT(size.isValid());                  // DEV Issue: Need a valid size.

            auto image = std::make_shared<Vulkan::Image>();
            image->initialize(device->getDevice(),
                              size, type, format,
                              readValue(imageNode, u8"samples",       samples, false, context),
                              readValue(imageNode, u8"tiling",        tilings, false, context),
                              usage,
                              readValue(imageNode, u8"sharingMode",   sharingModes, false, context),
                              readValue(imageNode, u8"initialLayout", imageLayouts, false, context),
                              readValue(imageNode, u8"memoryProperty", memoryProperties, true, context)
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
                const uint32_t idV = getIdentifiant(viewNode, u8"View", _view, context, existing);

                // Read parameter
                const VkImageViewType typeV   = readValue(viewNode, u8"viewType", viewTypes, false, context);
                const VkFormat        formatV = readValue(viewNode, u8"format",   formats, false, context);

                const VkComponentMapping mapping
                {
                    readValue(viewNode, u8"componentSwizzleRed",   componentSwizzles, false, context),
                    readValue(viewNode, u8"componentSwizzleGreen", componentSwizzles, false, context),
                    readValue(viewNode, u8"componentSwizzleBlue",  componentSwizzles, false, context),
                    readValue(viewNode, u8"componentSwizzleAlpha", componentSwizzles, false, context)
                };
                VkImageSubresourceRange subRessourceRange;
                subRessourceRange.aspectMask = readValue(viewNode, u8"aspectMask", aspectFlags, true, context);
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
            const uint32_t id = getIdentifiant(samplerNode, u8"Sampler", _samplers, context, existing);
            if (existing)
            {
                continue;
            }

            // Read max LOD from image
            float maxLod = 0.0f;
            if (samplerNode->hasAttribute(u8"imageId"))
            {
                const uint32_t idI = getLinkedIdentifiant(samplerNode, u8"imageId", _images, context);
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
                compareOp = readValue(samplerNode, u8"compareOp", compareOperations, false, context);
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
                readValue(samplerNode, u8"magFilter", filters, false, context),
                readValue(samplerNode, u8"minFilter", filters, false, context),
                readValue(samplerNode, u8"mipmapMode", samplerMipmaps, false, context),
                readValue(samplerNode, u8"addressModeU", samplerAdresses, false, context),
                readValue(samplerNode, u8"addressModeV", samplerAdresses, false, context),
                readValue(samplerNode, u8"addressModeW", samplerAdresses, false, context),
                mipLodBias, anisotropyEnable, maxAnisotropy, compareEnable, compareOp,
                minLod, maxLod,
                readValue(samplerNode, u8"borderColor", borderColors, false, context),
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

            // Mandatory attribute
            bool existing;
            const uint32_t id = getIdentifiant(bufferNode, u8"Buffer", _buffers, context, existing);
            if (existing)
            {
                continue;
            }

            const auto usage          = readValue(bufferNode, u8"usage", bufferUsages, true, context);
            const auto memoryProperty = readValue(bufferNode, u8"memoryProperty", memoryProperties, true, context);
            VkDeviceSize size=0;
            if(bufferNode->hasAttribute(u8"size"))
            {
                bufferNode->getAttribute(u8"size", size);
            }
            else if(bufferNode->hasAttribute(u8"external"))
            {
                const uint32_t idE = getLinkedIdentifiant(bufferNode, u8"external", _cpuBuffers, context);
                size = _cpuBuffers[idE].lock()->getByteSize();
            }
            MOUCA_ASSERT(size > 0);

            auto buffer = std::make_shared<Vulkan::Buffer>();
            buffer->initialize(device->getDevice(), usage, memoryProperty, size);
            // Register
            device->insertBuffer(buffer);
            _buffers[id] = buffer;
        }
    }
}
// <Buffers>
// <Buffer id = "0" usage = "VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT" size = "192" / >
// < / Buffers>

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
            const uint32_t id  = getIdentifiant(graphicsPipelineNode, u8"GraphicsPipeline", _graphicsPipelines, context, existing);

            const uint32_t idR = getLinkedIdentifiant(graphicsPipelineNode, u8"renderPassId", _renderPasses, context);
            const uint32_t idL = getLinkedIdentifiant(graphicsPipelineNode, u8"pipelineLayoutId", _pipelineLayouts, context);

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

void Engine3DXMLLoader::loadCommandPools(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak)
{
    MOUCA_PRE_CONDITION(!deviceWeak.expired()); //DEV Issue: Bad device !

    // Search CommandBuffer
    auto result = context._parser.getNode(u8"CommandPools");
    if (result->getNbElements() > 0)
    {
        MOUCA_ASSERT(result->getNbElements() == 1); //DEV Issue: please clean xml ?
        auto device = deviceWeak.lock();

        auto aPush = context._parser.autoPushNode(*result->getNode(0));
        // Parsing all images
        auto allCommandPools = context._parser.getNode(u8"CommandPool");
        for (size_t idPool = 0; idPool < allCommandPools->getNbElements(); ++idPool)
        {
            auto poolNode = allCommandPools->getNode(idPool);

            // Mandatory attribute
            bool existing;
            const uint32_t id = getIdentifiant(poolNode, u8"CommandPool", _commandPools, context, existing);
            if (existing)
            {
                continue;
            }

            int idQueue = 0;
            if (poolNode->hasAttribute(u8"families"))
            {
                Core::String families;
                poolNode->getAttribute(u8"families", families);
                if (families == u8"graphic")
                {
                    idQueue = device->getDevice().getQueueFamilyGraphicId();
                }   
                else if (families == u8"compute")
                {
                    idQueue = device->getDevice().getQueueFamilyComputeId();
                }
                else if (families == u8"transfer")
                {
                    idQueue = device->getDevice().getQueueFamilyTransferId();
                }
                else
                {
                    MOUCA_THROW_ERROR_2(u8"Engine3D", u8"UnknownFamiliesError", context.getFileName(), families);
                }   
            }
            else if (poolNode->hasAttribute(u8"queueId"))
            {
                poolNode->getAttribute(u8"queueId", idQueue);
            }

            VkCommandPoolCreateFlags flags = 0;
            if (poolNode->hasAttribute(u8"flags"))
            {
                flags = readValue(poolNode, u8"flags", poolCreateFlags, true, context);
            }

            // Build
            auto pool = std::make_shared<Vulkan::CommandPool>();
            pool->initialize(device->getDevice(), idQueue, flags);

            // Register
            device->insertCommandPool(pool);
            _commandPools[id] = pool;
        }
    }
}

void Engine3DXMLLoader::loadCommandsGroup(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak)
{
    MOUCA_PRE_CONDITION(!deviceWeak.expired()); //DEV Issue: Bad device !

    // Search CommandBuffer
    auto result = context._parser.getNode(u8"CommandsGroups");
    if (result->getNbElements() > 0)
    {
        MOUCA_ASSERT(result->getNbElements() == 1); //DEV Issue: please clean xml ?
        auto device = deviceWeak.lock();

        auto aPush = context._parser.autoPushNode(*result->getNode(0));

        // Parsing all images
        auto allCommandsGroup = context._parser.getNode(u8"CommandsGroup");
        for (size_t idGroup = 0; idGroup < allCommandsGroup->getNbElements(); ++idGroup)
        {
            auto groupNode = allCommandsGroup->getNode(idGroup);

            bool existing;
            const uint32_t id = getIdentifiant(groupNode, u8"CommandsGroup", _commandsGroup, context, existing);
            if (existing)
            {
                continue;
            }

            // Load commands
            auto allCommands = std::make_unique<Vulkan::Commands>();
            {
                // Enter to node each time to parse properly
                auto aPushC = context._parser.autoPushNode(*groupNode);

                // Build command for specific framebuffer
                loadCommands(context, deviceWeak, VkExtent2D(), *allCommands.get());
            }

            // Register
            _commandsGroup[id] = std::move(allCommands);
        }
    }
}

void Engine3DXMLLoader::loadCommandBuffers(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak)
{
    MOUCA_PRE_CONDITION(!deviceWeak.expired()); //DEV Issue: Bad device !

    // Search CommandBuffer
    auto result = context._parser.getNode(u8"CommandBuffers");
    if (result->getNbElements() > 0)
    {
        MOUCA_ASSERT(result->getNbElements() == 1); //DEV Issue: please clean xml ?
        auto device = deviceWeak.lock();

        auto aPush = context._parser.autoPushNode(*result->getNode(0));

        // Parsing all images
        auto allCommandBuffers = context._parser.getNode(u8"CommandBuffer");
        for (size_t idCommandBuffer = 0; idCommandBuffer < allCommandBuffers->getNbElements(); ++idCommandBuffer)
        {
            auto commandBufferNode = allCommandBuffers->getNode(idCommandBuffer);

            const uint32_t poolId = getLinkedIdentifiant(commandBufferNode, u8"commandPoolId", _commandPools, context);

            const VkCommandBufferLevel level = readValue(commandBufferNode, u8"level", commandBufferLevels, false, context);

            VkCommandBufferUsageFlags usage = 0;
            if (commandBufferNode->hasAttribute(u8"usage"))
            {
                usage = readValue(commandBufferNode, u8"usage", commandBufferUsages, true, context);
            }

            // Special command buffer for surface
            if (commandBufferNode->hasAttribute("surfaceId"))
            {
                const uint32_t surfaceId       = getLinkedIdentifiant(commandBufferNode, u8"surfaceId", _surfaces, context);
                auto surface = _surfaces[surfaceId].lock();
                MOUCA_ASSERT(surface->getSwapChain().getImages().size() == surface->getFrameBuffer().size());

                // Load commands
                Vulkan::Commands allCommands;
                {
                    // Enter to node each time to parse properly
                    auto aPushC = context._parser.autoPushNode(*commandBufferNode);

                    // Build command for specific framebuffer
                    loadCommands(context, deviceWeak, surface->getFormat().getConfiguration()._extent, allCommands);
                }

                // Create command buffer for surface
                surface->createCommandBuffer(std::move(allCommands), _commandPools[poolId].lock(), level, usage);
            }
            else
            {
                bool existing;
                const uint32_t id = getIdentifiant(commandBufferNode, u8"CommandBuffer", _commandBuffers, context, existing);
                if (existing)
                {
                    continue;
                }

                // Enter to node each time to parse properly
                auto aPushC = context._parser.autoPushNode(*commandBufferNode);

                // Build command
                Vulkan::Commands allCommands;
                loadCommands(context, deviceWeak, VkExtent2D(), allCommands);

                // Build new CommandBuffer
                auto commandBuffer = std::make_shared<Vulkan::CommandBuffer>();
                commandBuffer->initialize(device->getDevice(), _commandPools[poolId].lock(), level, usage);
                commandBuffer->registerCommands(std::move(allCommands));
                commandBuffer->execute();

                deviceWeak.lock()->insertCommandBuffer(commandBuffer);
                _commandBuffers[id] = commandBuffer;
            }
        }
    }
}

void Engine3DXMLLoader::loadCommands(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak, const VkExtent2D& resolution, std::vector<Vulkan::CommandUPtr>& commands, const Core::String& nodeName)
{
    // Parsing all commands
    auto allCommands = context._parser.getNode(nodeName);
    for (size_t idCommand = 0; idCommand < allCommands->getNbElements(); ++idCommand)
    {
        auto commandNode = allCommands->getNode(idCommand);

        Core::String type;
        commandNode->getAttribute(u8"type", type);

        // Build command
        Vulkan::CommandUPtr command;        
        if(type == u8"viewport")
        {
            // Build default viewport
            VkViewport viewport
            {
                0.0f, 0.0f,
                static_cast<float>(resolution.width), static_cast<float>(resolution.height),
                0.0f, 1.0f
            };

            if(commandNode->hasAttribute(u8"x"))        { commandNode->getAttribute(u8"x",        viewport.x); }
            if(commandNode->hasAttribute(u8"y"))        { commandNode->getAttribute(u8"y",        viewport.y); }
            if(commandNode->hasAttribute(u8"width"))    { readAttribute(commandNode, u8"width",  viewport.width, context);  }
            if(commandNode->hasAttribute(u8"height"))   { readAttribute(commandNode, u8"height", viewport.height, context); }
            if(commandNode->hasAttribute(u8"minDepth")) { commandNode->getAttribute(u8"minDepth", viewport.minDepth); }
            if(commandNode->hasAttribute(u8"maxDepth")) { commandNode->getAttribute(u8"maxDepth", viewport.maxDepth); }
            MOUCA_ASSERT(viewport.width != 0 && viewport.height != 0);
            
            // Build command
            command = std::make_unique<Vulkan::CommandViewport>(viewport);
        }
        else if(type == u8"scissor")
        {
            // Build default rect
            VkRect2D rect{ {0, 0}, resolution };
            
            if(commandNode->hasAttribute(u8"x"))      { commandNode->getAttribute(u8"x",      rect.offset.x); }
            if(commandNode->hasAttribute(u8"y"))      { commandNode->getAttribute(u8"y",      rect.offset.y); }
            if(commandNode->hasAttribute(u8"width"))  { readAttribute(commandNode, u8"width",  rect.extent.width, context);  }
            if(commandNode->hasAttribute(u8"height")) { readAttribute(commandNode, u8"height", rect.extent.height, context); }

            MOUCA_ASSERT(rect.extent.width > 0 && rect.extent.height > 0);
            // Build command
            command = std::make_unique<Vulkan::CommandScissor>(rect);
        }
        else if(type == u8"beginRenderPass")
        {
            const uint32_t renderPassId = getLinkedIdentifiant(commandNode, u8"renderPassId", _renderPasses, context);

            // Get specified frameBuffer or use transmit (only for swapchain)
            VkRect2D renderArea{ {0,0}, resolution };
            std::vector<Vulkan::FrameBufferWPtr> frameBuffers;
            if (commandNode->hasAttribute(u8"frameBufferId"))
            {
                const uint32_t idF = getLinkedIdentifiant(commandNode, u8"frameBufferId", _frameBuffers, context);
                frameBuffers.emplace_back(_frameBuffers[idF]);

                // Use by default framebuffer extent
                renderArea.extent = _frameBuffers[idF].lock()->getResolution();
            }
            else if (commandNode->hasAttribute(u8"surfaceId"))
            {
                const uint32_t idS = getLinkedIdentifiant(commandNode, u8"surfaceId", _surfaces, context);
                const auto& fbs = _surfaces[idS].lock()->getFrameBuffer();
                frameBuffers.resize(fbs.size());
                std::copy(fbs.cbegin(), fbs.cend(), frameBuffers.begin());
            }

            MOUCA_ASSERT(renderArea.extent.width > 0 && renderArea.extent.height > 0);

            if(commandNode->hasAttribute(u8"x"))      { commandNode->getAttribute(u8"x",      renderArea.offset.x); }
            if(commandNode->hasAttribute(u8"y"))      { commandNode->getAttribute(u8"y",      renderArea.offset.y); }
            if(commandNode->hasAttribute(u8"width"))  { commandNode->getAttribute(u8"width",  renderArea.extent.width); }
            if(commandNode->hasAttribute(u8"height")) { commandNode->getAttribute(u8"height", renderArea.extent.height); }
            const auto subpassContent = readValue(commandNode, u8"subpassContent", subpassContents, false, context);
            
            // Read clear
            auto aPushC = context._parser.autoPushNode(*commandNode);

            auto allCleanValue = context._parser.getNode(u8"CleanValue");

            std::vector<VkClearValue> clearColors(allCleanValue->getNbElements());
            auto itClean = clearColors.begin();
            for(size_t idCleanValue = 0; idCleanValue < allCleanValue->getNbElements(); ++idCleanValue)
            {
                auto cleanValueNode = allCleanValue->getNode(idCleanValue);

                bool isColor = cleanValueNode->hasAttribute(u8"type") || cleanValueNode->hasAttribute(u8"colorR") || cleanValueNode->hasAttribute(u8"colorG") || cleanValueNode->hasAttribute(u8"colorB") || cleanValueNode->hasAttribute(u8"colorA");
                bool isDS    = cleanValueNode->hasAttribute(u8"depth") || cleanValueNode->hasAttribute(u8"stencil");
                // Check validity
                if(!(isColor ^ isDS))
                {
                    MOUCA_THROW_ERROR_1(u8"Engine3D", u8"XMLCleanValueMixError", context.getFileName());
                }

                if(isColor)
                {
                    Core::String dataType;
                    cleanValueNode->getAttribute(u8"type", dataType);

                    if(dataType==u8"float")
                    {
                        cleanValueNode->getAttribute(u8"colorR", itClean->color.float32[0]);
                        cleanValueNode->getAttribute(u8"colorG", itClean->color.float32[1]);
                        cleanValueNode->getAttribute(u8"colorB", itClean->color.float32[2]);
                        cleanValueNode->getAttribute(u8"colorA", itClean->color.float32[3]);
                    }
                    else if(dataType==u8"int")
                    {
                        cleanValueNode->getAttribute(u8"colorR", itClean->color.int32[0]);
                        cleanValueNode->getAttribute(u8"colorG", itClean->color.int32[1]);
                        cleanValueNode->getAttribute(u8"colorB", itClean->color.int32[2]);
                        cleanValueNode->getAttribute(u8"colorA", itClean->color.int32[3]);
                    }
                    else if(dataType==u8"uint")
                    {
                        cleanValueNode->getAttribute(u8"colorR", itClean->color.uint32[0]);
                        cleanValueNode->getAttribute(u8"colorG", itClean->color.uint32[1]);
                        cleanValueNode->getAttribute(u8"colorB", itClean->color.uint32[2]);
                        cleanValueNode->getAttribute(u8"colorA", itClean->color.uint32[3]);
                    }
                    else
                    {
                        MOUCA_THROW_ERROR_2(u8"Engine3D", u8"XMLCleanValueTypeError", context.getFileName(), dataType);
                    }
                }
                else
                {
                    cleanValueNode->getAttribute(u8"depth",   itClean->depthStencil.depth);
                    cleanValueNode->getAttribute(u8"stencil", itClean->depthStencil.stencil);
                }

                ++itClean;
            }

            // Build command
            if (frameBuffers.size() > 1)
            {
                command = std::make_unique<Vulkan::CommandBeginRenderPassSurface>(*_renderPasses[renderPassId].lock(), std::move(frameBuffers), renderArea, std::move(clearColors), subpassContent);
            }
            else
            {
                command = std::make_unique<Vulkan::CommandBeginRenderPass>(*_renderPasses[renderPassId].lock(), frameBuffers.front(), renderArea, std::move(clearColors), subpassContent);
            }
        }
        else if(type == u8"endRenderPass")
        {
            // Build command
            command = std::make_unique<Vulkan::CommandEndRenderPass>();
        }
        else if(type == u8"graphicsPipeline")
        {
            const uint32_t graphicsPipelineId = getLinkedIdentifiant(commandNode, u8"graphicsPipelineId", _graphicsPipelines, context);
            const auto       bindPoint          = readValue(commandNode, u8"bindPoint", bindPoints, false, context);
            command = std::make_unique<Vulkan::CommandPipeline>(*_graphicsPipelines[graphicsPipelineId].lock(), bindPoint);
        }
        else if (type == u8"draw")
        {
            uint32_t vertexCount;
            uint32_t instanceCount;
            uint32_t firstVertex;
            uint32_t firstInstance;
            commandNode->getAttribute(u8"vertexCount",   vertexCount);
            commandNode->getAttribute(u8"instanceCount", instanceCount);
            commandNode->getAttribute(u8"firstVertex",   firstVertex);
            commandNode->getAttribute(u8"firstInstance", firstInstance);
            command = std::make_unique<Vulkan::CommandDraw>(vertexCount, instanceCount, firstVertex, firstInstance);
        }
        else if (type == u8"bindVertexBuffers")
        {
            uint32_t firstBinding;
            uint32_t bindingCount;
            commandNode->getAttribute(u8"firstBinding",  firstBinding);
            commandNode->getAttribute(u8"bindingCount",  bindingCount);

            // Read clear
            auto aPushB = context._parser.autoPushNode(*commandNode);

            auto allBuffers = context._parser.getNode(u8"Buffer");

            std::vector<Vulkan::BufferWPtr> buffers;
            buffers.reserve(allBuffers->getNbElements());
            std::vector<VkDeviceSize> offsets;
            offsets.reserve(allBuffers->getNbElements());
            
            for (size_t idBuffer = 0; idBuffer < allBuffers->getNbElements(); ++idBuffer)
            {
                auto bufferNode = allBuffers->getNode(idBuffer);
                const uint32_t idB = getLinkedIdentifiant(bufferNode, u8"bufferId", _buffers, context);
                VkDeviceSize offset;
                bufferNode->getAttribute(u8"offset", offset);

                buffers.emplace_back(_buffers[idB]);
                offsets.emplace_back(offset);
            }
            command = std::make_unique<Vulkan::CommandBindVertexBuffer>(firstBinding, bindingCount, std::move(buffers), std::move(offsets));
        }
        else if (type == u8"bindIndexBuffers")
        {
            const uint32_t idB = getLinkedIdentifiant(commandNode, u8"bufferId", _buffers, context);
            VkDeviceSize offset;
            commandNode->getAttribute(u8"offset", offset);
            VkIndexType indexType = readValue(commandNode, u8"indexType", indexTypes, false, context);

            command = std::make_unique<Vulkan::CommandBindIndexBuffer>(_buffers[idB], offset, indexType);
        }
        else if (type == u8"drawIndexed")
        {
            uint32_t indexCount;
            if (commandNode->hasAttribute(u8"bufferId"))
            {
                uint32_t idB = getLinkedIdentifiant(commandNode, u8"bufferId", _cpuBuffers, context);
                indexCount = static_cast<uint32_t>(_cpuBuffers[idB].lock()->getNbElements() * 3);
            }
            else
            {
                commandNode->getAttribute(u8"indexCount", indexCount);
            }
            uint32_t instanceCount;
            commandNode->getAttribute(u8"instanceCount", instanceCount);
            uint32_t firstIndex;
            commandNode->getAttribute(u8"firstIndex", firstIndex);
            int32_t  vertexOffset;
            commandNode->getAttribute(u8"vertexOffset", vertexOffset);
            uint32_t firstInstance;
            commandNode->getAttribute(u8"firstInstance", firstInstance);

            command = std::make_unique<Vulkan::CommandDrawIndexed>(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
        }
        else if (type == u8"bindDescriptorSets")
        {
            const uint32_t idP       = getLinkedIdentifiant(commandNode, u8"pipelineLayoutId", _pipelineLayouts, context);
            const auto       bindPoint = readValue(commandNode, u8"bindPoint", bindPoints, false, context);
            uint32_t firstSet;
            commandNode->getAttribute(u8"firstSet", firstSet);
            const uint32_t idS       = getLinkedIdentifiant(commandNode, u8"descriptorSetId", _descriptorSets, context);

            auto aPushB = context._parser.autoPushNode(*commandNode);
                       
            // Read Dynamic Offset
            auto allDynamicOffsets = context._parser.getNode(u8"DynamicOffset");
            std::vector<uint32_t> dynamicOffsets;
            dynamicOffsets.reserve(allDynamicOffsets->getNbElements());

            for (size_t idDynamicOffset = 0; idDynamicOffset < allDynamicOffsets->getNbElements(); ++idDynamicOffset)
            {
                auto dynamicOffsetNode = allDynamicOffsets->getNode(idDynamicOffset);
                uint32_t offset;
                dynamicOffsetNode->getAttribute(u8"offset", offset);

                dynamicOffsets.emplace_back(offset);
            }
            command = std::make_unique<Vulkan::CommandBindDescriptorSets>(*_pipelineLayouts[idP].lock(), bindPoint, firstSet, _descriptorSets[idS].lock()->getDescriptorSets(), std::move(dynamicOffsets));
        }
        else if (type == u8"pipelineBarrier")
        {
            const auto srcStage   = readValue(commandNode, u8"srcStage",        pipelineStageFlags, true, context);
            const auto dstStage   = readValue(commandNode, u8"dstStage",        pipelineStageFlags, true, context);
            const auto dependency = readValue(commandNode, u8"dependencyFlags", dependencyFlags,    true, context);
         
            auto aPushP = context._parser.autoPushNode(*commandNode);

            // Read image memories
            auto allImageMemories = context._parser.getNode(u8"DynamicOffset");
            Vulkan::CommandPipelineBarrier::ImageMemoryBarriers imageBarriers;
            imageBarriers.reserve(allImageMemories->getNbElements());

            for (size_t idImageMemory = 0; idImageMemory < allImageMemories->getNbElements(); ++idImageMemory)
            {
                auto imageMemoryNode = allImageMemories->getNode(idImageMemory);

                const uint32_t idI = getLinkedIdentifiant(imageMemoryNode, u8"imageId", _images, context);

                VkImageMemoryBarrier barrier
                {
                    VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    nullptr,
                    readValue(imageMemoryNode, u8"srcAccessMask", accessFlags, true, context),
                    readValue(imageMemoryNode, u8"dstAccessMask", accessFlags, true, context),
                    readValue(imageMemoryNode, u8"oldLayout",     imageLayouts, false, context),
                    readValue(imageMemoryNode, u8"newLayout",     imageLayouts, false, context),
                    0, 0,
                    _images[idI].lock()->getImage(),
                    { readValue(imageMemoryNode, u8"aspectMask", aspectFlags, true, context)},
                };

                imageMemoryNode->getAttribute(u8"srcQueueFamilyIndex", barrier.srcQueueFamilyIndex);
                imageMemoryNode->getAttribute(u8"dstQueueFamilyIndex", barrier.dstQueueFamilyIndex);
                imageMemoryNode->getAttribute(u8"baseMipLevel",        barrier.subresourceRange.baseMipLevel);
                imageMemoryNode->getAttribute(u8"levelCount",          barrier.subresourceRange.levelCount);
                imageMemoryNode->getAttribute(u8"baseArrayLayer",      barrier.subresourceRange.baseArrayLayer);
                imageMemoryNode->getAttribute(u8"layerCount",          barrier.subresourceRange.layerCount);

                imageBarriers.emplace_back(barrier);
            }

            Vulkan::CommandPipelineBarrier::MemoryBarriers memoryBarriers;
            Vulkan::CommandPipelineBarrier::BufferMemoryBarriers bufferMemoryBarriers;

            command = std::make_unique<Vulkan::CommandPipelineBarrier>(srcStage, dstStage, dependency, std::move(memoryBarriers), std::move(bufferMemoryBarriers), std::move(imageBarriers));
        }
        else if (type == u8"pushConstants")
        {
            const uint32_t idL           = getLinkedIdentifiant(commandNode, u8"pipelineLayoutId", _pipelineLayouts, context);
            const VkShaderStageFlags stage = readValue(commandNode, u8"stage", shaderStages, true, context);
            const uint32_t idB           = getLinkedIdentifiant(commandNode, u8"external", _cpuBuffers, context);
            const auto& buffer             = _cpuBuffers[idB].lock();

            command = std::make_unique<Vulkan::CommandPushConstants>(*_pipelineLayouts[idL].lock(), stage, static_cast<uint32_t>(buffer->getByteSize()), buffer->getData());
        }
        else if (type == u8"container")
        {
            bool existing;
            const uint32_t id = getIdentifiant(commandNode, nodeName, _commandLinks, context, existing);
            if (existing)
            {
                continue;
            }

            command = std::make_unique<Vulkan::CommandContainer>();
            _commandLinks[id] = command.get(); // Keep pointer (badly because can be delete -> shared/weak ?)
        }
        else if (type == u8"switch")
        {
            bool existing;
            const uint32_t id = getIdentifiant(commandNode, nodeName, _commandLinks, context, existing);
            if (existing)
            {
                continue;
            }
            auto commandSwitch = std::make_unique<Vulkan::CommandSwitch>();
            _commandLinks[id] = commandSwitch.get(); // Keep pointer (badly because can be delete -> shared/weak ?)

            // Load sub-commands
            {
                auto aPush = context._parser.autoPushNode(*commandNode);

                std::vector<Vulkan::CommandUPtr> subCommands;
                loadCommands(context, deviceWeak, resolution, subCommands, Core::String("Sub"+nodeName));

                commandSwitch->transfer(std::move(subCommands));
            }

            command = std::move(commandSwitch);
        }
        else
        {
            MOUCA_THROW_ERROR_2(u8"Engine3D", u8"XMLUnknownCommandError", context.getFileName(), type);
        }

        // Transfer to vector
        commands.emplace_back(std::move(command));
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
            const uint32_t id = getIdentifiant(queueSequenceNode, u8"View", _queueSequences, context, existing);

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
                const uint32_t idF = getLinkedIdentifiant(sequenceNode, u8"fenceId", _fences, context);
                fence = _fences[idF];
            }
            if(sequenceNode->hasAttribute(u8"timeout")) { sequenceNode->getAttribute(u8"timeout", timeout); }
            if(sequenceNode->hasAttribute(u8"semaphoreId"))
            {
                const uint32_t idSem = getLinkedIdentifiant(sequenceNode, u8"semaphoreId", _semaphores, context);
                semaphore = _semaphores[idSem];
            }

            // Build Sequence
            const uint32_t idS = getLinkedIdentifiant(sequenceNode, u8"surfaceId", _surfaces, context);
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
                const uint32_t idF = getLinkedIdentifiant(fenceNode, u8"fenceId", _fences, context);
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
                const uint32_t idF = getLinkedIdentifiant(fenceNode, u8"fenceId", _fences, context);
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
                const uint32_t idF = getLinkedIdentifiant(sequenceNode, u8"fenceId", _fences, context);
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

            const uint32_t idI = getLinkedIdentifiant(sequenceNode, u8"imageId", _images, context);
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
                const uint32_t idF = getLinkedIdentifiant(sequenceNode, u8"fenceId", _fences, context);
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
                const uint32_t idS = getLinkedIdentifiant(swapChainNode, u8"surfaceId", _surfaces, context);
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
                const uint32_t idSem = getLinkedIdentifiant(semaphoreNode, u8"semaphoreId", _semaphores, context);
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
        const uint32_t idSem  = getLinkedIdentifiant(waitSyncNode, u8"semaphoreId", _semaphores, context);
        const auto pipelineFlag = readValue(waitSyncNode, u8"pipelineFlag", pipelineStageFlags, true, context);
        // Store
        waitSemaphores.emplace_back(Vulkan::WaitSemaphore(_semaphores[idSem], pipelineFlag));
    }

    // Parsing all Signal Semaphore
    auto allSignalSyncs = context._parser.getNode(u8"SignalSync");
    for (size_t idSignalSync = 0; idSignalSync < allSignalSyncs->getNbElements(); ++idSignalSync)
    {
        auto signalSyncNode = allSignalSyncs->getNode(idSignalSync);

        // Read info
        const uint32_t idSem  = getLinkedIdentifiant(signalSyncNode, u8"semaphoreId", _semaphores, context);
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
            const uint32_t idS = getLinkedIdentifiant(commandBufferNode, u8"surfaceId", _surfaces, context);
            
            // Prepare data
            auto surface = _surfaces[idS].lock();
            commandBuffers.emplace_back(surface->getICommandBuffer());
        }
        else
        {
            // Get surface associated
            const uint32_t idC = getLinkedIdentifiant(commandBufferNode, u8"id", _commandBuffers, context);
            commandBuffers.emplace_back(_commandBuffers[idC]);
        }
    }

    auto submitInfo = std::make_unique<Vulkan::SubmitInfo>();
    submitInfo->addSynchronization(waitSemaphores, signalSemaphores);
    submitInfo->initialize(std::move(commandBuffers));
    //submitInfo->buildSubmitInfo();

    submitInfos.emplace_back(std::move(submitInfo));
}

void loadStencilOp(Engine3DXMLLoader::ContextLoading& context, XML::NodeUPtr node, VkStencilOpState& state)
{
    state.failOp      = readValue(node, u8"failOp",      stencilOperations, false, context);
    state.passOp      = readValue(node, u8"passOp",      stencilOperations, false, context);
    state.depthFailOp = readValue(node, u8"depthFailOp", stencilOperations, false, context);
    state.compareOp   = readValue(node, u8"compareOp",   compareOperations, false, context);
    node->getAttribute(u8"compareMask", state.compareMask);
    node->getAttribute(u8"writeMask",   state.writeMask);
    node->getAttribute(u8"reference",   state.reference);
}

void Engine3DXMLLoader::loadPipelineStateCreate(ContextLoading& context, Vulkan::PipelineStateCreateInfo& info, const uint32_t graphicsPipelineId, const uint32_t renderPassId)
{
    uint32_t states = Vulkan::PipelineStateCreateInfo::Uninitialized;
    
    // Stages
    auto stages = context._parser.getNode(u8"Stages");
    if (stages->getNbElements() > 0)
    {
        MOUCA_ASSERT(stages->getNbElements() == 1); //DEV Issue: Need to clean xml !
        
        // cppcheck-suppress unreadVariable // false positive
        auto aPush = context._parser.autoPushNode(*stages->getNode(0));

        // Parsing all Graphics pipeline
        auto allStages = context._parser.getNode(u8"Stage");
        for (size_t idStage = 0; idStage < allStages->getNbElements(); ++idStage)
        {
            auto stageNode = allStages->getNode(idStage);

            const uint32_t idS = getLinkedIdentifiant(stageNode, u8"shaderModuleId", _shaderModules, context);

            auto aPushShader = context._parser.autoPushNode(*stageNode);

            Vulkan::ShaderSpecialization specialization;
            auto specializations = context._parser.getNode(u8"Specialization");
            if (specializations->getNbElements() > 0)
            {
                auto specializationNode = specializations->getNode(0);
                // Read buffer
                const uint32_t idB = getLinkedIdentifiant(specializationNode, u8"external", _cpuBuffers, context);
                const auto& buffer   = _cpuBuffers[idB].lock();
                MOUCA_ASSERT(buffer != nullptr && !buffer->isNull());
                specialization.addDataInfo(buffer->getData(), buffer->getByteSize());

                auto aPushS = context._parser.autoPushNode(*specializationNode);

                // Add map entry
                auto allEntries = context._parser.getNode(u8"Entry");

                std::vector<VkSpecializationMapEntry> entries;
                entries.resize(allEntries->getNbElements());
                auto itEntry = entries.begin();
                for (size_t idEntry = 0; idEntry < allEntries->getNbElements(); ++idEntry)
                {
                    auto entryNode = allEntries->getNode(idEntry);

                    entryNode->getAttribute(u8"constantID", itEntry->constantID);
                    entryNode->getAttribute(u8"offset",     itEntry->offset);
                    entryNode->getAttribute(u8"size",       itEntry->size);
                    ++itEntry;
                }

                specialization.setMapEntries(std::move(entries));
            }

            info.getStages().addShaderModule(_shaderModules[idS], std::move(specialization));
        }
    }

    if(info.getStages().getNbShaders() <= 0)
    {
        MOUCA_THROW_ERROR_3(u8"Engine3D", u8"XMLMissingNodeError", context.getFileName(), u8"GraphicsPipeline" , u8"Stages/Stage");
    }

    // Input Assembly
    auto inputAssemblies = context._parser.getNode(u8"InputAssembly");
    if (inputAssemblies->getNbElements() > 0)
    {
        MOUCA_ASSERT(inputAssemblies->getNbElements() == 1); //DEV Issue: Need to clean xml !
        auto inputAssembly = inputAssemblies->getNode(0);

        // Build Input Assembly
        bool restart;
        inputAssembly->getAttribute(u8"primitiveRestartEnable", restart);

        auto& state = info.getInputAssembly()._state;
        state.topology = readValue(inputAssembly, u8"topology", primitiveTopologies, false, context);
        state.primitiveRestartEnable = restart ? VK_TRUE : VK_FALSE;
            
        // Activate state
        states |= Vulkan::PipelineStateCreateInfo::InputAssembly;
    }

    // Vertex Input Assembly
    auto vertexInput = context._parser.getNode(u8"VertexInput");
    if (vertexInput->getNbElements() > 0)
    {
        MOUCA_ASSERT(vertexInput->getNbElements() == 1); //DEV Issue: Need to clean xml !
        auto vertexInputNode = vertexInput->getNode(0);

        // cppcheck-suppress unreadVariable // false positive
        auto pushV = context._parser.autoPushNode(*vertexInputNode);

        // Load binding
        auto allBindings = context._parser.getNode(u8"BindingDescription");
        std::vector<VkVertexInputBindingDescription> bindings;
        bindings.reserve(allBindings->getNbElements());
        for (size_t idBinding = 0; idBinding < allBindings->getNbElements(); ++idBinding)
        {
            auto bindingNode = allBindings->getNode(idBinding);

            VkVertexInputBindingDescription bindingDescription;
            bindingNode->getAttribute(u8"binding", bindingDescription.binding);
            bindingDescription.inputRate = readValue(bindingNode, u8"inputRate", vertexInputRates, false, context);

            if (bindingNode->hasAttribute(u8"stride"))
            {
                bindingNode->getAttribute(u8"stride", bindingDescription.stride);
            }
            else
            {
                const uint32_t idM = getLinkedIdentifiant(bindingNode, u8"meshDescriptorId", _cpuMeshDescriptors, context);
                bindingDescription.stride = static_cast<uint32_t>(_cpuMeshDescriptors[idM].getByteSize());
            }

            bindings.emplace_back(bindingDescription);
        }
        info.getVertexInput().setBindingDescriptions(std::move(bindings));
        
        // Load Attributs
        auto allAttributs = context._parser.getNode(u8"AttributeDescription");

        std::vector<VkVertexInputAttributeDescription> attributs;
        attributs.reserve(allAttributs->getNbElements());
        for (size_t idAttribute = 0; idAttribute < allAttributs->getNbElements(); ++idAttribute)
        {
            auto attributNode = allAttributs->getNode(idAttribute);

            VkVertexInputAttributeDescription attributDescription;
            attributNode->getAttribute(u8"binding",  attributDescription.binding);
            attributNode->getAttribute(u8"location", attributDescription.location);
            if (attributNode->hasAttribute(u8"offset"))
            {
                attributNode->getAttribute(u8"offset", attributDescription.offset);
                attributDescription.format = readValue(attributNode, u8"format", formats, false, context);

                attributs.emplace_back(attributDescription);
            }
            else
            {
                // Get descriptor
                const uint32_t idM = getLinkedIdentifiant(attributNode, u8"meshDescriptorId", _cpuMeshDescriptors, context);
                const auto& descriptors = _cpuMeshDescriptors[idM].getDescriptors();

                // Reallocate more
                attributs.reserve(attributs.size() + descriptors.size());

                attributDescription.offset = 0;

                for (const auto& descriptor : descriptors)
                {
                    attributDescription.format = Vulkan::PipelineStateCreateInfo::computeDescriptorFormat(descriptor);
                    
                    // Add new descriptor
                    attributs.emplace_back(attributDescription);

                    // Next descriptor
                    attributDescription.offset += static_cast<uint32_t>(descriptor.getSizeInByte());
                    attributDescription.location += 1;
                }
            }
        }
        info.getVertexInput().setAttributeDescriptions(std::move(attributs));

        // Activate state
        states |= Vulkan::PipelineStateCreateInfo::VertexInput;
    }

    // Rasterization
    auto rasterizations = context._parser.getNode(u8"Rasterization");
    if (rasterizations->getNbElements() > 0)
    {
        MOUCA_ASSERT(rasterizations->getNbElements() == 1); //DEV Issue: Need to clean xml !
        auto rasterization = rasterizations->getNode(0);

        auto& state = info.getRasterizer()._state;
        state.cullMode    = readValue(rasterization, u8"cullMode",    cullModes,    true,  context);
        state.polygonMode = readValue(rasterization, u8"polygonMode", polygonModes, false, context);
        state.frontFace   = readValue(rasterization, u8"frontFace",   frontFaces,   false, context);
        bool boolean;
        rasterization->getAttribute(u8"depthClampEnable",         boolean);
        state.depthClampEnable        = boolean ? VK_TRUE : VK_FALSE;
        rasterization->getAttribute(u8"rasterizerDiscardEnable",  boolean);
        state.rasterizerDiscardEnable = boolean ? VK_TRUE : VK_FALSE;
        rasterization->getAttribute(u8"depthBiasEnable",          boolean);
        state.depthBiasEnable         = boolean ? VK_TRUE : VK_FALSE;
        // Depth Bias
        if(state.depthBiasEnable)
        {
            rasterization->getAttribute(u8"depthBiasClamp",          state.depthBiasClamp);
            rasterization->getAttribute(u8"depthBiasConstantFactor", state.depthBiasConstantFactor);
            rasterization->getAttribute(u8"depthBiasSlopeFactor",    state.depthBiasSlopeFactor);
        }

        // Optional
        if(rasterization->hasAttribute(u8"lineWidth"))
        {
            rasterization->getAttribute(u8"lineWidth", state.lineWidth);
        }

        // Activate state
        states |= Vulkan::PipelineStateCreateInfo::Rasterization;
    }

    // Blend
    auto blends = context._parser.getNode(u8"BlendState");
    if (blends->getNbElements() > 0)
    {
        MOUCA_ASSERT(blends->getNbElements() == 1); //DEV Issue: Need to clean xml !
        auto blend = blends->getNode(0);
        
        auto& state = info.getBlending()._state;

        // Logic Operator
        bool enable;
        blend->getAttribute(u8"logicOpEnable", enable);
        state.logicOpEnable = enable ? VK_TRUE : VK_FALSE;
        state.logicOp       = VK_LOGIC_OP_CLEAR;
        if (state.logicOpEnable)
        {
            state.logicOp = readValue(blend, u8"logicOp", logicOperators, false, context);
        }
        
        // Blend constant
        blend->getAttribute(u8"blendConstantsR", state.blendConstants[0]);
        blend->getAttribute(u8"blendConstantsG", state.blendConstants[1]);
        blend->getAttribute(u8"blendConstantsB", state.blendConstants[2]);
        blend->getAttribute(u8"blendConstantsA", state.blendConstants[3]);

        // All attachments: must be equal to RenderPass/subpass/colorAttachment count (See doc)
        // cppcheck-suppress unreadVariable // false positive
        auto pushB = context._parser.autoPushNode(*blend);
        auto allBlends = context._parser.getNode(u8"BlendAttachment");
        
        const size_t nbColors = _renderPasses[renderPassId].lock()->getSubPasses()[0].colorAttachmentCount;
        if( nbColors != allBlends->getNbElements() )
        {
            MOUCA_THROW_ERROR_3(u8"Engine3D", u8"XMLBlendAttachmentSizeError", context.getFileName(), std::to_string(graphicsPipelineId), std::to_string(renderPassId));
        }

        std::vector<VkPipelineColorBlendAttachmentState> attachments;
        attachments.reserve(allBlends->getNbElements());
        for (size_t idBlend = 0; idBlend < allBlends->getNbElements(); ++idBlend)
        {
            auto blendAttachmentNode = allBlends->getNode(idBlend);
            blendAttachmentNode->getAttribute(u8"blendEnable", enable);
            const VkColorComponentFlags flags = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            VkPipelineColorBlendAttachmentState attachmentState
            {
                static_cast<VkBool32>(enable ? VK_TRUE : VK_FALSE), // VkBool32                 blendEnable
                VK_BLEND_FACTOR_ZERO,                               // VkBlendFactor            srcColorBlendFactor
                VK_BLEND_FACTOR_ZERO,                               // VkBlendFactor            dstColorBlendFactor
                VK_BLEND_OP_ADD,                                    // VkBlendOp                colorBlendOp
                VK_BLEND_FACTOR_ZERO,                               // VkBlendFactor            srcAlphaBlendFactor
                VK_BLEND_FACTOR_ZERO,                               // VkBlendFactor            dstAlphaBlendFactor
                VK_BLEND_OP_ADD,                                    // VkBlendOp                alphaBlendOp
                flags                                               // VkColorComponentFlags    colorWriteMask
            };
            if(attachmentState.blendEnable)
            {
                attachmentState.srcColorBlendFactor = readValue(blendAttachmentNode, u8"srcColorBlendFactor", blendFactors, false, context);
                attachmentState.dstColorBlendFactor = readValue(blendAttachmentNode, u8"dstColorBlendFactor", blendFactors, false, context);
                attachmentState.colorBlendOp        = readValue(blendAttachmentNode, u8"colorBlendOp",        blendOperations, false, context);
                attachmentState.srcAlphaBlendFactor = readValue(blendAttachmentNode, u8"srcAlphaBlendFactor", blendFactors, false, context);
                attachmentState.dstAlphaBlendFactor = readValue(blendAttachmentNode, u8"dstAlphaBlendFactor", blendFactors, false, context);
                attachmentState.alphaBlendOp        = readValue(blendAttachmentNode, u8"alphaBlendOp",        blendOperations, false, context);
            }

            attachmentState.colorWriteMask = readValue(blendAttachmentNode, u8"colorWriteMask", colorComponents, true, context);

            attachments.emplace_back(attachmentState);
        }
        
        info.getBlending().setColorBlendAttachments(attachments);

        // Activate state
        states |= Vulkan::PipelineStateCreateInfo::ColorBlend;
    }

    // DepthStencil
    auto depthStencils = context._parser.getNode(u8"DepthStencil");
    if (depthStencils->getNbElements() > 0)
    {
        MOUCA_ASSERT(depthStencils->getNbElements() == 1); //DEV Issue: Need to clean xml !
        auto depthStencilNode = depthStencils->getNode(0);

        auto& state = info.getDepthStencil()._state;
        bool enable;
        depthStencilNode->getAttribute(u8"depthTestEnable", enable);
        state.depthTestEnable  = enable ? VK_TRUE : VK_FALSE;
        depthStencilNode->getAttribute(u8"depthWriteEnable", enable);
        state.depthWriteEnable = enable ? VK_TRUE : VK_FALSE;
        state.depthCompareOp   = readValue(depthStencilNode, u8"depthCompareOp", compareOperations, false, context);

        // cppcheck-suppress unreadVariable // false positive
        auto pushD = context._parser.autoPushNode(*depthStencilNode);

        auto stencilFront = context._parser.getNode(u8"StencilFront");
        auto stencilBack  = context._parser.getNode(u8"StencilBack");
        if (stencilFront->getNbElements() == 0 && stencilBack->getNbElements() == 0)
        {
            state.stencilTestEnable = VK_FALSE;
        }
        else
        {
            if (stencilFront->getNbElements() != 1 || stencilBack->getNbElements() != 1)
            {
                MOUCA_THROW_ERROR_2(u8"Engine3D", u8"XMLMissingStencilFrontBackError", context.getFileName(), std::to_string(graphicsPipelineId));
            }

            loadStencilOp(context, stencilFront->getNode(0), state.front);
            loadStencilOp(context, stencilBack->getNode(0),  state.back);
        
            state.stencilTestEnable = VK_TRUE;
        }
        
        auto depthBound  = context._parser.getNode(u8"DepthBounds");
        if (depthBound->getNbElements() > 0)
        {
            MOUCA_ASSERT(depthBound->getNbElements() == 1); //DEV Issue: Need to clean xml !
            auto depthBoundNode = depthBound->getNode(0);

            state.stencilTestEnable = VK_TRUE;

            depthBoundNode->getAttribute(u8"min", state.minDepthBounds);
            depthBoundNode->getAttribute(u8"max", state.maxDepthBounds);
        }
        else
        {
            state.stencilTestEnable = VK_FALSE;
        }

        // Activate state
        states |= Vulkan::PipelineStateCreateInfo::DepthStencil;
    }

    // Dynamic
    bool dynamicViewport = false;
    bool dynamicScissor  = false;
    auto allDynamics = context._parser.getNode(u8"DynamicState");
    if (allDynamics->getNbElements() > 0)
    {
        for (size_t idDynamic = 0; idDynamic < allDynamics->getNbElements(); ++idDynamic)
        {
            auto dynamicNode   = allDynamics->getNode(idDynamic);
            
            // Read value
            const auto dynamic = readValue(dynamicNode, u8"state", dynamics, false, context);
            info.getDynamic().addDynamicState(dynamic);

            dynamicViewport |= (dynamic == VK_DYNAMIC_STATE_VIEWPORT);
            dynamicScissor  |= (dynamic == VK_DYNAMIC_STATE_SCISSOR);
        }

        // Activate state
        states |= Vulkan::PipelineStateCreateInfo::Dynamic;
    }

    // Viewport
    auto allViewports = context._parser.getNode(u8"Viewport");
    if (allViewports->getNbElements() > 0)
    {
        MOUCA_TODO("Find best way to make Dynamic<->Viewport/Scissor and manage all cases");
        if (dynamicViewport && dynamicScissor)
        {
            info.getViewport().addDynamicViewport();
        }
        else
        {
            for (size_t idViewport = 0; idViewport < allViewports->getNbElements(); ++idViewport)
            {
                auto viewportNode = allViewports->getNode(idViewport);

                VkViewport viewport{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
                VkRect2D   scissor { {0, 0}, {0, 0} };

                viewportNode->getAttribute(u8"x", viewport.x);
                viewportNode->getAttribute(u8"y", viewport.y);
                if (viewportNode->hasAttribute(u8"surfaceId"))
                {
                    const uint32_t surfaceId = getLinkedIdentifiant(viewportNode, u8"surfaceId", _surfaces, context);
                    const VkExtent2D& resolution = _surfaces[surfaceId].lock()->getFormat().getConfiguration()._extent;
                    viewport.width  = static_cast<float>(resolution.width);
                    viewport.height = static_cast<float>(resolution.height);
                    scissor.extent  = resolution;
                }
                else if (viewportNode->hasAttribute(u8"imageId"))
                {
                    const uint32_t imageId = getLinkedIdentifiant(viewportNode, u8"imageId", _images, context);
                    const VkExtent3D& imageRes = _images[imageId].lock()->getExtent();
                    const VkExtent2D resolution = { imageRes.width, imageRes.height };
                    viewport.width  = static_cast<float>(resolution.width);
                    viewport.height = static_cast<float>(resolution.height);
                    scissor.extent = resolution;
                }
                else
                {
                    viewportNode->getAttribute(u8"width",  viewport.width);
                    viewportNode->getAttribute(u8"height", viewport.height);
                }
                
                viewportNode->getAttribute(u8"minDepth", viewport.minDepth);
                viewportNode->getAttribute(u8"maxDepth", viewport.maxDepth);

                viewportNode->getAttribute(u8"scissorX",      scissor.offset.x);
                viewportNode->getAttribute(u8"scissorY",      scissor.offset.y);
                if (!viewportNode->hasAttribute(u8"surfaceId") && !viewportNode->hasAttribute(u8"imageId"))
                {
                    viewportNode->getAttribute(u8"scissorWidth", scissor.extent.width);
                    viewportNode->getAttribute(u8"scissorHeight", scissor.extent.height);
                }

                info.getViewport().addViewport(viewport, scissor);
            }
        }
        
        // Activate state
        states |= Vulkan::PipelineStateCreateInfo::Viewport;
    }

    // Multisample
    auto multisample = context._parser.getNode(u8"Multisample");
    if (multisample->getNbElements() > 0)
    {
        MOUCA_ASSERT(multisample->getNbElements() == 1); //DEV Issue: Need to clean xml !
        auto multisampleNode = multisample->getNode(0);

        auto& state = info.getMultisample()._state;

        state.rasterizationSamples = readValue(multisampleNode, u8"rasterizationSamples", samples, false, context);
        
        if (multisampleNode->hasAttribute(u8"minSampleShading"))
        {
            state.sampleShadingEnable = VK_TRUE;
            multisampleNode->getAttribute(u8"minSampleShading", state.minSampleShading);
        }
        else
        {
            state.sampleShadingEnable = VK_FALSE;
        }

        state.pSampleMask = nullptr;
        bool enable;
        multisampleNode->getAttribute(u8"alphaToCoverageEnable", enable);
        state.alphaToOneEnable = enable ? VK_TRUE : VK_FALSE;
        multisampleNode->getAttribute(u8"alphaToOneEnable", enable);
        state.alphaToOneEnable = enable ? VK_TRUE : VK_FALSE;

        // Activate state
        states |= Vulkan::PipelineStateCreateInfo::Multisample;
    }

    // Tessellation
    auto tessellation = context._parser.getNode(u8"Tessellation");
    if(tessellation->getNbElements() > 0)
    {
        MOUCA_ASSERT(tessellation->getNbElements() == 1); //DEV Issue: Need to clean xml !
        auto tessellationNode = tessellation->getNode(0);
        
        // Read data
        auto& state = info.getTessellation()._state;
        tessellationNode->getAttribute(u8"patchControlPoints", state.patchControlPoints);

        // Activate state
        states |= Vulkan::PipelineStateCreateInfo::Tessellation;
    }

    // Finish creation
    info.initialize(static_cast<Vulkan::PipelineStateCreateInfo::States>(states));
}

void Engine3DXMLLoader::loadPipelineLayouts(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak)
{
    // Search Pipeline Layouts
    auto result = context._parser.getNode(u8"PipelineLayouts");
    if (result->getNbElements() > 0)
    {
        MOUCA_ASSERT(result->getNbElements() == 1); //DEV Issue: please clean xml ?
        auto device = deviceWeak.lock();

        // cppcheck-suppress unreadVariable // false positive
        auto aPush = context._parser.autoPushNode(*result->getNode(0));

        // Parsing all PipelineLayouts
        auto allPipelineLayouts = context._parser.getNode(u8"PipelineLayout");
        for (size_t idPipelineLayout = 0; idPipelineLayout < allPipelineLayouts->getNbElements(); ++idPipelineLayout)
        {
            auto pipelineLayoutNode = allPipelineLayouts->getNode(idPipelineLayout);

            bool existing;
            const uint32_t id = getIdentifiant(pipelineLayoutNode, u8"PipelineLayout", _pipelineLayouts, context, existing);

            // Crate PipelineLayout
            auto pipelineLayout = std::make_shared<Vulkan::PipelineLayout>();

            auto aPushP = context._parser.autoPushNode(*pipelineLayoutNode);

            // Read DescriptorSetLayout
            auto allDescriptorSetLayouts = context._parser.getNode(u8"DescriptorSetLayout");
            std::vector<VkDescriptorSetLayout> setLayouts;
            setLayouts.reserve(allDescriptorSetLayouts->getNbElements());
            for (size_t idDescriptorSetLayout = 0; idDescriptorSetLayout < allDescriptorSetLayouts->getNbElements(); ++idDescriptorSetLayout)
            {
                auto DSLNode = allDescriptorSetLayouts->getNode(idDescriptorSetLayout);
                const uint32_t idDSL = getLinkedIdentifiant(DSLNode, u8"descriptorSetId", _descriptorSetLayouts, context);
                // Get Vulkan ID
                setLayouts.emplace_back(_descriptorSetLayouts[idDSL].lock()->getInstance());
            }

            // Read PushConstantRange
            auto allPushConstantRanges = context._parser.getNode(u8"PushConstantRange");
            for (size_t idPushConstant = 0; idPushConstant < allPushConstantRanges->getNbElements(); ++idPushConstant)
            {
                auto pushConstantNode = allPushConstantRanges->getNode(idPushConstant);

                VkPushConstantRange pushConstant;
                pushConstant.stageFlags = readValue(pushConstantNode, u8"shaderStageFlags", shaderStages, true, context);
                pushConstantNode->getAttribute(u8"offset", pushConstant.offset);
                pushConstantNode->getAttribute(u8"size",   pushConstant.size);
                // Register
                pipelineLayout->addPushConstant(pushConstant);
            }
            
            // Build PipelineLayout
            pipelineLayout->initialize(device->getDevice(), setLayouts);

            // Register
            device->insertPipelineLayout(pipelineLayout);
            _pipelineLayouts[id] = pipelineLayout;
        }
    }
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
            const uint32_t id = getIdentifiant(descriptorSetLayoutNode, u8"DescriptorSetLayout", _descriptorSetLayouts, context, existing);
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
                const auto flags = readValue(bindingNode, u8"shaderStageFlags", shaderStages, true, context);
                const auto type  = readValue(bindingNode, u8"type", descriptorTypes, false, context);
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
            const uint32_t id = getIdentifiant(descriptorPoolNode, u8"DescriptorPool", _descriptorPools, context, existing);
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
                
                poolSize.type = readValue(sizeNode, u8"type", descriptorTypes, false, context);
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
            const uint32_t id = getIdentifiant(descriptorSetNode, u8"DescriptorSet", _descriptorSets, context, existing);
            if (existing)
            {
                continue;
            }

            const uint32_t idPool = getLinkedIdentifiant(descriptorSetNode, u8"descriptorPoolId", _descriptorPools, context);
            
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

                const uint32_t idS = getLinkedIdentifiant(dSetLayoutNode, u8"descriptorSetLayoutId", _descriptorSetLayouts, context);
                dSetLayouts.emplace_back(_descriptorSetLayouts[idS].lock()->getInstance());

                // Parse WriteDescriptor
                auto aPushSL = context._parser.autoPushNode(*dSetLayoutNode);
                
                auto allWriteDescriptors = context._parser.getNode(u8"WriteDescriptor");
                for (size_t idWriteDescriptor = 0; idWriteDescriptor < allWriteDescriptors->getNbElements(); ++idWriteDescriptor)
                {
                    auto writeDescriptorNode = allWriteDescriptors->getNode(idWriteDescriptor);

                    // Read attribute
                    const auto     type    = readValue(writeDescriptorNode, u8"descriptorType", descriptorTypes, false, context);
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

                            const auto idBuffer = getLinkedIdentifiant(bufferInfoNode, u8"bufferId", _buffers, context);

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

                            const auto idSampler = getLinkedIdentifiant(imageInfoNode, u8"samplerId", _samplers, context);
                            const auto idView    = getLinkedIdentifiant(imageInfoNode, u8"viewId",    _view, context);

                            Vulkan::DescriptorImageInfo info
                            {
                                _samplers[idSampler].lock(),
                                _view[idView].lock(),
                                readValue(imageInfoNode, u8"layout", imageLayouts, false, context)
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
            const uint32_t id = getIdentifiant(shaderModuleNode, u8"ShaderModule", _shaderModules, context, existing);

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
            const auto stage = readValue(shaderModuleNode, u8"stage", shaderStages, false, context);

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

}