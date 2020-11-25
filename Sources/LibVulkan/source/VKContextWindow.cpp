/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include "LibVulkan/include/VKContextWindow.h"

#include "LibVulkan/include/VKCommand.h"
#include "LibVulkan/include/VKCommandBufferSurface.h"
#include "LibVulkan/include/VKContextDevice.h"
#include "LibVulkan/include/VKFrameBuffer.h"
#include "LibVulkan/include/VKRenderPass.h"
#include "LibVulkan/include/VKSemaphore.h"
#include "LibVulkan/include/VKSurface.h"
#include "LibVulkan/include/VKSurfaceFormat.h"
#include "LibVulkan/include/VKSwapChain.h"
#include "LibVulkan/include/VKWindowSurface.h"

namespace Vulkan
{

ContextWindow::ContextWindow():
_linkContext(nullptr), _linkSurface(nullptr), _swapChain(std::make_shared<SwapChain>())
{}

void ContextWindow::initialize(const ContextDevice& linkContext, const WindowSurface& linkSurface, const SurfaceFormat::Configuration& configuration)
{
    MOUCA_PRE_CONDITION(!linkContext.isNull());
    MOUCA_PRE_CONDITION(!linkSurface._surface->isNull());

    _linkContext = &linkContext;
    _linkSurface = &linkSurface;

    const Device& device = _linkContext->getDevice();

    // Build format of surface
    linkSurface._surface->computeSurfaceFormat(device, configuration, _format);

    // Build swapchain
    _swapChain->initialize(device, *linkSurface._surface.get(), _format);
    _swapChain->generateImages(device, _format);

    MOUCA_POST_CONDITION(!_swapChain->isNull() && !_swapChain->getImages().empty()); // DEV Issue: Not ready ?
}

void ContextWindow::createFrameBuffer(RenderPassWPtr renderPass, const ViewAttachments& attachments)
{
    MOUCA_PRE_CONDITION(!attachments.empty());
    MOUCA_PRE_CONDITION(!_swapChain->isNull());
    MOUCA_PRE_CONDITION(!renderPass.expired() && !renderPass.lock()->isNull());
    //MOUCA_PRE_CONDITION( std::count(attachments.begin(), attachments.end(), ImageViewWPtr()) == 1 );
    
    // Keep configuration
    _viewAttachments = attachments;

    // Allocated data
    _frameBuffers.resize(_swapChain->getImages().size());
    std::generate(_frameBuffers.begin(), _frameBuffers.end(), []() -> auto { return std::make_shared<FrameBuffer>(); });
    
    // Fill
    fillFrameBuffer(renderPass);
}

void ContextWindow::fillFrameBuffer(RenderPassWPtr renderPass)
{
    MOUCA_PRE_CONDITION(!_swapChain->isNull());
    MOUCA_PRE_CONDITION(!renderPass.expired() && !renderPass.lock()->isNull());
    MOUCA_PRE_CONDITION(!_frameBuffers.empty());

    auto itSwapChainImage = _swapChain->getImages().cbegin();

    FrameBuffer::Attachments viewIdAttachments;
    viewIdAttachments.resize(_viewAttachments.size());

    // Allocated data
    for (auto& frameBuffer : _frameBuffers)
    {
        // Get index of swapchain View
        const Device& device = _linkContext->getDevice();
        //     const auto itImage = std::find(attachments.cbegin(), attachments.cend(), static_cast<VkImageView>(VK_NULL_HANDLE));
        //     BT_ASSERT(itImage != attachments.end());
        //     const size_t index = std::distance(attachments.cbegin(), itImage);

        // Transfer attachment and setup swapchain
        std::transform(_viewAttachments.begin(), _viewAttachments.end(), viewIdAttachments.begin(),
            [&](const auto& view) -> VkImageView
            { return view.expired() ? itSwapChainImage->getView() : view.lock()->getInstance(); });

        // Build FrameBuffer
        frameBuffer->initialize(device, renderPass, _format.getConfiguration()._extent, viewIdAttachments);

        ++itSwapChainImage;
    }

    MOUCA_POST_CONDITION(itSwapChainImage == _swapChain->getImages().cend()); //Security
}

void ContextWindow::createCommandBuffer(Commands&& commands, CommandPoolSPtr pool, const VkCommandBufferLevel level, const VkCommandBufferUsageFlags usage)
{
    MOUCA_PRE_CONDITION(!_linkContext->isNull());
    MOUCA_PRE_CONDITION(shared_from_this() != nullptr);

    // Create
    _commandBuffer = std::make_shared<CommandBufferSurface>();
    _commandBuffer->initialize(shared_from_this(), pool, level, usage);

    _commandBuffer->registerCommands(std::move(commands));
    _commandBuffer->execute(VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

    //Security to never access it anymore
    commands.clear();

    MOUCA_POST_CONDITION(!_commandBuffer->isNull()); // DEV Issue: Not ready ?

    /*
    MOUCA_PRE_CONDITION(_commandBuffers.empty()); // DEV Issue: Already build CommandBuffer ?
    MOUCA_PRE_CONDITION(!_linkContext->isNull());
    MOUCA_PRE_CONDITION(commands.size() == _swapChain->getImages().size());

    const Device& device = _linkContext->getDevice();

    // Prepare memory
    _commandBuffers.resize(commands.size());
    auto itCommand = commands.begin();
    // Initialize and make command ready
    for(auto& commandBuffer : _commandBuffers)
    {
        commandBuffer = std::make_shared<CommandBuffer>();
        commandBuffer->initialize(device, pool, level);

        commandBuffer->registerCommands(std::move(*itCommand));
        commandBuffer->execute();
        
        ++itCommand;
    }
    BT_ASSERT(itCommand == commands.end()); //Security
    
    //Security to never access it anymore
    commands.clear();

    MOUCA_POST_CONDITION(!_commandBuffers.empty()); // DEV Issue: Not ready ?
    */
}

void ContextWindow::updateCommandBuffer(const VkCommandBufferResetFlags reset) const
{
    MOUCA_PRE_CONDITION(!_linkContext->isNull());
    MOUCA_PRE_CONDITION(!_commandBuffer->isNull());

    _commandBuffer->execute(reset);
}

void ContextWindow::release()
{
    MOUCA_PRE_CONDITION(!_linkContext->isNull());

    const Device& device = _linkContext->getDevice();

    // Release CommandBuffers
    if(_commandBuffer)
    {
        _commandBuffer->release(device);
        _commandBuffer.reset();
    }

    // Release FrameBuffer if possible
    for(auto& frameBuffer : _frameBuffers)
    {
        frameBuffer->release(device);
    }
    _frameBuffers.clear();

    // Release swapchain
    _swapChain->release(device);
}

void ContextWindow::setReady(bool ready)
{
    MOUCA_PRE_CONDITION(_swapChain != nullptr);

    _swapChain->setReady(ready);
}

void ContextWindow::resize(const VkExtent2D& newSize)
{
    MOUCA_PRE_CONDITION(!_swapChain->isReady());                       //DEV Issue: Please disable rendering before and properly !
    MOUCA_PRE_CONDITION(newSize.width != 0 && newSize.height != 0);    //DEV Issue: Need valid size !

    // Check we have different size
    const auto& surfaceSize = _format.getConfiguration()._extent;
    if (newSize.width != surfaceSize.width || newSize.height != surfaceSize.height)
    {
        // Set new size
        _format.setSize(newSize);

        const auto& device     = _linkContext->getDevice();
        auto        renderPass = _frameBuffers.front()->getRenderPass().lock();

        // Clear memory
        for(auto& frameBuffer: _frameBuffers)
        {
            frameBuffer->release(device);
        }

        //renderPass->release(device);
        _swapChain->release(device);        

        // Restart initialize
        initialize(*_linkContext, *_linkSurface, _format.getConfiguration() );

        // Rebuild frame buffer
        fillFrameBuffer(renderPass);

        // Refresh command buffer
        _commandBuffer->execute(VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
//         for(auto& commandBuffer : _commandBuffers)
//         {
//             commandBuffer->execute();
//         }
    }

    // Ready to run
    _swapChain->setReady(true);

    MOUCA_POST_CONDITION(!_swapChain->isNull()); //DEV Issue: All must working !
    MOUCA_POST_CONDITION(_swapChain->isReady()); //DEV Issue: All must working !
}

ICommandBufferWPtr ContextWindow::getICommandBuffer() const
{
    return _commandBuffer;
}

}