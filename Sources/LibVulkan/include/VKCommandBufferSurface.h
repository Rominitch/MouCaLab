/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibVulkan/include/VKCommandBuffer.h>

namespace Vulkan
{
    class ContextWindow;
    using ContextWindowWPtr = std::weak_ptr<ContextWindow>;

    class CommandBufferSurface : public ICommandBuffer
    {
        MOUCA_NOCOPY(CommandBufferSurface);

        public:
            /// Constructor
            CommandBufferSurface();
            /// Destructor
            ~CommandBufferSurface() override;

            void initialize(const ContextWindowWPtr window, CommandPoolSPtr pool, const VkCommandBufferLevel level, const VkCommandBufferUsageFlags usage);
            void release(const Device& device) override;

            //------------------------------------------------------------------------
            /// \brief  Check if command buffer is null.
            /// 
            /// \returns True if null, false otherwise.
            bool isNull() const override;

            const std::vector<VkCommandBuffer>& getCommandBuffers() const { return _commandBuffers; }

            void registerCommands(Commands&& commands);

            void execute(const VkCommandBufferResetFlags reset) const override;

            VkCommandBuffer getActiveCommandBuffer() const override;

        private:
            ContextWindowWPtr            _window;         ///< [LINK] Link to window.
            std::vector<VkCommandBuffer> _commandBuffers; ///< All real CommandBuffer managed.
    };
}