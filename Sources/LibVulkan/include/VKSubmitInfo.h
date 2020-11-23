/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace Vulkan
{
    // Forward declaration
    class ICommandBuffer;
    using ICommandBufferWPtr = std::weak_ptr<ICommandBuffer>;

    class Device;

    class Semaphore;
    using SemaphoreWPtr = std::weak_ptr<Semaphore>;

    class SwapChain;
    using SwapChainWPtr = std::weak_ptr<SwapChain>;

    using WaitSemaphore = std::pair<SemaphoreWPtr, VkPipelineStageFlags>;

    class SubmitInfo
    {
        MOUCA_NOCOPY(SubmitInfo);

        public:
            SubmitInfo() = default;
            ~SubmitInfo() = default;

            void addSynchronization(const std::vector<WaitSemaphore>& waitSemaphore, const std::vector<SemaphoreWPtr>& signalSemaphore);
            
            void initialize(std::vector<ICommandBufferWPtr>&& commandBuffers);

            VkSubmitInfo buildSubmitInfo();

        private:
            std::vector<ICommandBufferWPtr>     _commandBuffers;      ///< [LINK] Automatic system to choose right commandBuffer used by SwapChain.

            std::vector<VkSemaphore>            _waitSemaphore;
            std::vector<VkPipelineStageFlags>   _waitStageFlag;
            std::vector<VkSemaphore>            _signalSemaphore;
            std::vector<VkCommandBuffer>        _commands;
    };

    using SubmitInfoSPtr = std::shared_ptr<SubmitInfo>;
    using SubmitInfoWPtr = std::weak_ptr<SubmitInfo>;

}