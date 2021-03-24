#include "Dependencies.h"

#include "MouCaGraphicEngine/include/VulkanManager.h"

#include <LibRT/include/RTImage.h>
#include <LibRT/include/RTRenderDialog.h>
#include <LibRT/include/RTShaderFile.h>

#include <LibVulkan/include/VKCommandBufferSurface.h>
#include <LibVulkan/include/VKContextDevice.h>
#include <LibVulkan/include/VKContextWindow.h>
#include <LibVulkan/include/VKGraphicsPipeline.h>
#include <LibVulkan/include/VKPipelineStates.h>
#include <LibVulkan/include/VKSequence.h>
#include <LibVulkan/include/VKShaderProgram.h>
#include <LibVulkan/include/VKScreenshot.h>
#include <LibVulkan/include/VKSurface.h>
#include <LibVulkan/include/VKSwapChain.h>
#include <LibVulkan/include/VKWindowSurface.h>

namespace MouCaGraphic
{

const float VulkanManager::_version = 1.0f;

#ifdef VULKAN_DEBUG
const bool VulkanManager::_enableValidator = true;
#else
const bool VulkanManager::_enableValidator = false;
#endif

//Define major OS which can support vulkan
#ifdef MOUCA_OS_WINDOWS
const Core::String VulkanManager::_osName = Core::String("windows");
#elif defined MOUCA_OS_LINUX
const Core::String VulkanManager::_osName = Core::String("linux");
#elif defined MOUCA_OS_APPLE
const Core::String VulkanManager::_osName = Core::String("apple");
#elif defined MOUCA_OS_ANDROID
const Core::String VulkanManager::_osName = Core::String("android");
#else
#   error "Currently Engine3D doesn't support your OS. Please contact team or update code !"
#endif

VulkanManager::~VulkanManager()
{
    // Validation release was called !
    MOUCA_PRE_CONDITION(_environment.isNull());    //DEV Issue: Check you call release() properly.

    // Remove latest link (Nothing to delete)
    _surfaces.clear();
}

void VulkanManager::initialize(const RT::ApplicationInfo& info, const std::vector<const char*>& extensions)
{
    MOUCA_PRE_CONDITION(_environment.isNull());    //DEV Issue: No re-entrance: already call initialize().

    _environment.initialize(info, extensions);

    MOUCA_PRE_CONDITION(!_environment.isNull());   //DEV Issue: Safety check.
}

void VulkanManager::release()
{
    _shaders.clear();

    // Clean context window
    for(auto& window : _windows)
    {
        // Stop rendering
        window->setReady(false);
        // Synchronize
        window->getContextDevice().getDevice().waitIdle();

        window->release();
    }
    _windows.clear();

    // Delete all surface but not unlink windows or release 
    for(auto& surface : _surfaces)
    {
        releaseSurface(*surface);
    }

    // Delete all devices and all data associated (shutdown rendering too)
    for (auto& device : _devices)
    {
        device->release();
    }
    // Clean list
    _devices.clear();

    // Finally release
    if (!_environment.isNull())
    {
        _environment.release();
    }
}

uint32_t VulkanManager::addRenderDialog(RT::RenderDialogWPtr window)
{
    MOUCA_PRE_CONDITION(!window.expired());
    MOUCA_PRE_CONDITION(std::find_if(_surfaces.cbegin(), _surfaces.cend(), [&](const auto& surface) { return surface->_linkWindow.lock() == window.lock(); }) == _surfaces.cend());

    // Event - register signal
    {
        auto lockedWindow = window.lock();
        lockedWindow->signalResize().connectMember(this,    &VulkanManager::afterResizeWindow);
        lockedWindow->signalStateSize().connectMember(this, &VulkanManager::afterStateSizeWindow);
        lockedWindow->signalClose().connectMember(this,     &VulkanManager::afterClose);
    }

    _surfaces.emplace_back(std::make_unique<Vulkan::WindowSurface>(window));
    return static_cast<uint32_t>(_surfaces.size() - 1);
}

//-----------------------------------------------------------------------------------------
//                                      Build item
//-----------------------------------------------------------------------------------------
void VulkanManager::buildSurface(Vulkan::WindowSurface& surface) const
{
    MOUCA_PRE_CONDITION(!_environment.isNull());           //DEV Issue: Need to call initialize()
    MOUCA_PRE_CONDITION(!surface._linkWindow.expired());   //DEV Issue: surface data is not anymore valid.
    MOUCA_PRE_CONDITION(surface._surface == nullptr);      //DEV Issue: Re-entrance is not allowed.

    RT::WindowSPtr lockedWindow = surface._linkWindow.lock();
    MOUCA_ASSERT(lockedWindow.get() != nullptr);

    surface._surface = std::make_unique<Vulkan::Surface>();
    surface._surface->initialize(_environment, *lockedWindow.get());

    MOUCA_POST_CONDITION(!surface._surface->isNull());     //DEV Issue: Bad initialization ?
}

Vulkan::ContextDeviceWPtr VulkanManager::createRenderingDevice(const std::vector<const char*>& deviceExtensions, const Vulkan::PhysicalDeviceFeatures& mandatoryFeatures, Vulkan::WindowSurface& surface)
{
    MOUCA_PRE_CONDITION(!_environment.isNull());           //DEV Issue: Need to call initialize()
    MOUCA_PRE_CONDITION(!surface._linkWindow.expired());   //DEV Issue: Need to give a valid dialog.

    // Build surface before use
    if (surface._surface == nullptr)
    {
        buildSurface(surface);
    }

    // Attach rendering system to window
    Vulkan::ContextDeviceSPtr context = std::make_shared<Vulkan::ContextDevice>();
    context->initialize(_environment, deviceExtensions, mandatoryFeatures, surface._surface.get());

    // Enable all debug layer if properly configured
#ifdef VULKAN_DEBUG
    if (_enableValidator)
    {
        _environment.setDeviceInReport(context->getDevice().getInstance());
    }
#endif

    MOUCA_POST_CONDITION(!context->isNull());    //DEV Issue: creation was done properly !
    // Save into list by move (Don't use context after this line)
    _devices.emplace_back(context);
    
    return context;
}

Vulkan::ContextWindowWPtr VulkanManager::createWindowSurface(Vulkan::ContextDeviceWPtr context, const uint32_t surfaceID, const Vulkan::SurfaceFormat::Configuration& configuration)
{
    auto contextWindow = std::make_shared<Vulkan::ContextWindow>();
    auto& surface = _surfaces[surfaceID];
    if (surface->_surface == nullptr)
    {
        buildSurface(*surface);
    }

    // Build context
    contextWindow->initialize(*context.lock().get(), *surface, configuration);
    // Register
    _windows.emplace_back(contextWindow);

    return contextWindow;
}

void VulkanManager::releaseSurface(Vulkan::WindowSurface& surface)
{
    if (surface._surface != nullptr)
    {
        // Parsing all context window to disabled rendering and clean data.
        for(auto& window : _windows)
        {
            if (window->isWindowSurface(surface))
            {
                window->release();
            }
        }

        surface._surface->release(_environment);
        surface._surface.reset();
    }
}

void VulkanManager::afterClose(RT::Window* window)
{
    MOUCA_PRE_CONDITION(window != nullptr); //DEV Issue: bad signal ?

    // Search current surface
    auto itSurface = std::find_if(_surfaces.begin(), _surfaces.end(), [&](const auto& surface) { return window->getHandle() == surface->getHandle(); });
    if (itSurface != _surfaces.end())
    {
        auto itContext = std::find_if(_windows.begin(), _windows.end(), [&](const auto& window) { return window->isWindowSurface(**itSurface); });
        if (itContext != _windows.end())
        {
            // Stop rendering
            (*itContext)->setReady(false);
            // Synchronize
            (*itContext)->getContextDevice().getDevice().waitIdle();

            (*itContext)->release();

            _windows.erase(itContext);
        }

        // Release object
        releaseSurface(**itSurface);

        // Remove item of list (disconnection is done because object is closed)
        _surfaces.erase(itSurface);

        // Send event
        _afterClose.emit();
    }
}

void VulkanManager::afterResizeWindow(RT::Window* window, const RT::Array2ui& size)
{
    // Search current surface
    auto itSurface = std::find_if(_surfaces.begin(), _surfaces.end(), [&](const auto& surface) { return window->getHandle() == surface->getHandle(); });
    MOUCA_ASSERT(itSurface != _surfaces.end()); //DEV Issue: Already remove ?

    auto itContext = std::find_if(_windows.begin(), _windows.end(), [&](const auto& window) { return window->isWindowSurface(**itSurface); });
    MOUCA_ASSERT(itContext != _windows.end()); //DEV Issue: Already remove ?

    // Stop rendering
    (*itContext)->setReady(false);
    
    // Synchronize: after this line we can create/release properly each memory/buffer/command/...
    (*itContext)->getContextDevice().getDevice().waitIdle();
    
    // Avoid minimized case: leave buffer/image in state
    if(size != RT::Array2ui(0, 0))
    {
        // Try to resize (if different size)
        (*itContext)->resize({ size.x, size.y });

        // Send event
        _afterResize.emit();
    }
}

void VulkanManager::afterStateSizeWindow(RT::Window* window, RT::Window::StateSize state)
{
    // Search current surface
    auto itSurface = std::find_if(_surfaces.begin(), _surfaces.end(), [&](const auto& surface) { return window->getHandle() == surface->getHandle(); });
    MOUCA_ASSERT(itSurface != _surfaces.end()); //DEV Issue: Already remove ?

    auto itContext = std::find_if(_windows.begin(), _windows.end(), [&](const auto& window) { return window->isWindowSurface(**itSurface); });
    MOUCA_ASSERT(itContext != _windows.end()); //DEV Issue: Already remove ?

    // Stop rendering
    (*itContext)->setReady(false);
    
    // Synchronize
    (*itContext)->getContextDevice().getDevice().waitIdle();

    // Send event
    _afterResize.emit();
}

void VulkanManager::execute(const uint32_t deviceID, const uint32_t sequenceID, bool sync) const
{
    const_cast<VulkanManager*>(this)->_locked.lock();
    
    MOUCA_ASSERT(deviceID < _devices.size());
    auto context = _devices.at(deviceID);
    MOUCA_ASSERT(sequenceID < context->getQueueSequences().size());
    MOUCA_ASSERT(context->getQueueSequences().at(sequenceID).use_count() > 0);
    MOUCA_ASSERT(!context->getQueueSequences().at(sequenceID).get()->empty());
    for(const auto& sequence : *context->getQueueSequences().at(sequenceID))
    {
        if (sequence->execute(context->getDevice()) != VK_SUCCESS)
            break;
    }

    // Synchronize after operation
    if (sync)
    {
        context->getDevice().waitIdle();
    }

    const_cast<VulkanManager*>(this)->_locked.unlock();
}

void VulkanManager::takeScreenshot(const Core::Path& imageFilePath, RT::ImageImportSPtr diskImage, const uint32_t contextWindowID) const
{
    auto surfaceContext = _windows.at(contextWindowID);

    // Stop rendering and synchronize
    surfaceContext->getContextDevice().getDevice().waitIdle();

    // Make rendering into buffer
    const uint32_t latestImage = surfaceContext->getEditSwapChain().lock()->getCurrentImage();
    
    const RT::ComponentDescriptor component(4, RT::Type::UnsignedChar, RT::ComponentUsage::Color);

    Vulkan::GPUImageReader screenshot;
    screenshot.initialize(*surfaceContext, component);

    auto cpuImage = diskImage->getImage().lock();
    // Extract from GPU to CPU memory
    {
        const VkImage srcImage = surfaceContext->getEditSwapChain().lock()->getImages()[latestImage].getImage();

        const RT::Array3ui sizes =
        {
            surfaceContext->getFormat().getConfiguration()._extent.width,
            surfaceContext->getFormat().getConfiguration()._extent.height,
            1
        };

        screenshot.extractTo(srcImage, *surfaceContext, RT::Array3ui(0,0,0), RT::Array3ui(0,0,0), sizes, *cpuImage);
    }

    // Save on disk
    cpuImage->saveImage(imageFilePath);

    screenshot.release(*surfaceContext);
}

void VulkanManager::registerShader(RT::ShaderFileWPtr file, const ShaderRegistration& shader)
{
    MOUCA_PRE_CONDITION(!file.expired());                          // DEV Issue: Need valid data !
    MOUCA_PRE_CONDITION(!shader.second.expired());                 // DEV Issue: Need valid data !
    MOUCA_PRE_CONDITION(_shaders.find(file) == _shaders.cend());   // DEV Issue: Never added !

    _shaders[file] = shader;

    MOUCA_POST_CONDITION(_shaders.find(file) != _shaders.cend());   // DEV Issue: Not added ?
}

void VulkanManager::afterShaderEdition(Core::Resource& resource)
{
    auto itShader = std::find_if(_shaders.cbegin(), _shaders.cend(), 
                                 [&](auto& current) {return current.first.lock().get() == &resource; });
    if (itShader != _shaders.cend())
    {
        try
        {
            // Compile new version
            auto shaderFile = itShader->first.lock();
            shaderFile->compile();

            _locked.lock();

            // ------ Brutal algo ------------------
            // Step1 : disable all
            const auto& context = itShader->second.first.lock();
            for (auto& window : _windows)
            {
                window->setReady(false);
            }

            context->getDevice().waitIdle();

            // Step2 : Rebuild module ...
            auto module = itShader->second.second.lock();
            module->release(context->getDevice());
            shaderFile->open();
            module->initialize(context->getDevice(), shaderFile->extractString(), module->getName(), module->getStage());
            shaderFile->close();

            // Step3 : Update all graphics pipeline
            for (auto& pipeline : context->getGraphicsPipelines())
            {
                pipeline->release(context->getDevice());

                pipeline->getInfo().getStages().update();     

                pipeline->initialize(context->getDevice(), pipeline->getRenderPass(), pipeline->getPipelineLayout(), pipeline->getPipelineCache());
            }

            // Step4 : Restart all commands
            for (auto& window : _windows)
            {
                window->getCommandBuffer().execute(VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
//                 for (auto& commandBuffer : window->getCommandBuffers())
//                 {
//                     commandBuffer->execute();
//                 }

                window->setReady(true);
            }

            context->getDevice().waitIdle();

            _locked.unlock();
        }
        catch (const Core::Exception&)
        {
            // Keep old shader
        }

        // Send event
        _afterShaderChanged.emit();
    }
}

}