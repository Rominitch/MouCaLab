/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibVulkan/include/VKSurfaceFormat.h>

namespace Vulkan
{
    class Command;
    using CommandUPtr = std::unique_ptr<Command>;
    using Commands    = std::vector<CommandUPtr>;

    class ICommandBuffer;
    using ICommandBufferWPtr = std::weak_ptr<ICommandBuffer>;

    class CommandBuffer;
    using CommandBufferSPtr = std::shared_ptr<CommandBuffer>;
    using CommandBuffers    = std::vector<CommandBufferSPtr>;

    class CommandBufferSurface;
    using CommandBufferSurfaceSPtr = std::shared_ptr<CommandBufferSurface>;

    class CommandPool;
    using CommandPoolSPtr = std::shared_ptr<CommandPool>;

    class ContextDevice;

    class FrameBuffer;
    using FrameBufferSPtr = std::shared_ptr<FrameBuffer>;
    using FrameBuffers    = std::vector<FrameBufferSPtr>;

    class ImageView; 
    using ImageViewWPtr = std::weak_ptr<ImageView>;

    class RenderPass;
    using RenderPassWPtr = std::weak_ptr<RenderPass>;

    class Semaphore;
    using SemaphoreWPtr = std::weak_ptr<Semaphore>;

    class SwapChain;
    using SwapChainSPtr = std::shared_ptr<SwapChain>;
    using SwapChainWPtr = std::weak_ptr<SwapChain>;

    class WindowSurface;

    //----------------------------------------------------------------------------
    /// \brief Collection of all Vulkan object to renderer on window.
    class ContextWindow : public std::enable_shared_from_this<ContextWindow>
    {
        MOUCA_NOCOPY_NOMOVE(ContextWindow);

        public:
            using ViewAttachments = std::vector<ImageViewWPtr>;

            ContextWindow();
            ~ContextWindow() = default;

            void initialize(const ContextDevice& linkContext, const WindowSurface& linkSurface, const SurfaceFormat::Configuration& configuration);
            void release();

            void resize(const VkExtent2D& newSize);

            bool isWindowSurface(WindowSurface& window) const   { return &window == _linkSurface; }

            void createFrameBuffer(RenderPassWPtr renderPass, const ViewAttachments& attachments);

            void createCommandBuffer(Commands&& commands, CommandPoolSPtr pool, const VkCommandBufferLevel level, const VkCommandBufferUsageFlags usage);

            //------------------------------------------------------------------------
            /// \brief Refresh command buffer based on registered command.
            /// \note CommandBuffer MUST be not used during operation !
            void updateCommandBuffer(const VkCommandBufferResetFlags reset) const;

            const ContextDevice&   getContextDevice() const  { return *_linkContext; }
            const SwapChain&       getSwapChain() const      { return *_swapChain; }
            const SurfaceFormat&   getFormat() const         { return _format; }
            const FrameBuffers&    getFrameBuffer() const    { return _frameBuffers; }
            //const CommandBuffers&  getCommandBuffers() const { return _commandBuffers; }
            const CommandBufferSurface&  getCommandBuffer() const  { return *_commandBuffer; }
            CommandBufferSurface&        getCommandBuffer()        { return *_commandBuffer; }
            ICommandBufferWPtr           getICommandBuffer() const;

            SwapChainWPtr          getEditSwapChain()        { return _swapChain; }

            void setReady(bool ready);

        private:
            void fillFrameBuffer(RenderPassWPtr renderPass);

            ContextDevice const* _linkContext;
            WindowSurface const* _linkSurface;

            ViewAttachments _viewAttachments;

            FrameBuffers    _frameBuffers;          ///< Multi Framebuffer for each swapchain images.
            SwapChainSPtr   _swapChain;             ///< SwapChain attached to WindowSurface.
            SurfaceFormat   _format;                
            //CommandBuffers  _commandBuffers;        ///< Special command buffer using command RenderPass.
            CommandBufferSurfaceSPtr _commandBuffer;
    };
}