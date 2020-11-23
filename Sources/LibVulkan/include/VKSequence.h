/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace Vulkan
{
    class Device;

    class ContextWindow; 
    using ContextWindowWPtr = std::weak_ptr<ContextWindow>;

    class Fence;
    using FenceWPtr = std::weak_ptr<Fence>;

    class Semaphore;
    using SemaphoreWPtr = std::weak_ptr<Semaphore>;

    class SubmitInfo;
    using SubmitInfoUPtr = std::unique_ptr<SubmitInfo>;
    using SubmitInfos    = std::vector<SubmitInfoUPtr>;

    class SwapChain;
    using SwapChainWPtr = std::weak_ptr<SwapChain>;

    //----------------------------------------------------------------------------
    /// \brief Default graphic sequence for manage render order like submit/present/fence ...
    ///
    class Sequence : public std::enable_shared_from_this<Sequence>
    {
        MOUCA_NOCOPY(Sequence);
        public:
            virtual ~Sequence() = default;

            virtual VkResult execute(const Device& device) = 0;

            virtual void update() = 0;

        protected:
            Sequence() = default;
    };

    class SequenceAcquire : public Sequence
    {
        public:
            SequenceAcquire(SwapChainWPtr swapChain, SemaphoreWPtr semaphore, FenceWPtr fence, const uint64_t timeout);
            ~SequenceAcquire() override = default;

            VkResult execute(const Device& device) override;

            void update() override {}

        private:
            SwapChainWPtr _swapChain;

            VkFence       _fence;
            VkSemaphore   _semaphore;
            uint64_t      _timeout;
    };

    class SequenceWaitFence : public Sequence
    {
        public:
            SequenceWaitFence(const std::vector<FenceWPtr>& fences, const uint64_t timeout, const VkBool32 waitAll);
            ~SequenceWaitFence() override = default;

            VkResult execute(const Device& device) override;

            void update() override {}

        private:
            std::vector<VkFence> _fences;
            uint64_t             _timeout;
            VkBool32             _all;
    };

    class SequenceResetFence : public Sequence
    {
        public:
            SequenceResetFence(const std::vector<FenceWPtr>& fences);
            ~SequenceResetFence() override = default;

            VkResult execute(const Device& device) override;

            void update() override {}
        private:
            std::vector<VkFence> _fences;
    };

    class SequenceSubmit final : public Sequence
    {
        MOUCA_NOCOPY(SequenceSubmit);

        public:
            SequenceSubmit(SubmitInfos&& submitInfos, FenceWPtr fence);
            ~SequenceSubmit() override = default;

            VkResult execute(const Device& device) override;

            void update() override {}

        protected:
            SubmitInfos               _submitInfos; // [OWNERSHIP]

            std::vector<VkSubmitInfo> _vkSubmitInfos;
            VkFence                   _fence;
    };

    class SequencePresentKHR final : public Sequence
    {
        MOUCA_NOCOPY(SequencePresentKHR);

        public:
            SequencePresentKHR(const std::vector<SemaphoreWPtr>& semaphores, const std::vector<SwapChainWPtr>& swapChains);
            ~SequencePresentKHR() override;

            void registerSwapChain();

            VkResult execute(const Device& device) override;

            void update() override;

            const std::vector<VkResult>& getResult() const;

        protected:
            std::vector<SwapChainWPtr>  _swapChains;
        
            VkPresentInfoKHR            _presentInfo;
            std::vector<uint32_t>       _imageId;
            std::vector<VkSemaphore>    _semaphores;
            std::vector<VkResult>       _result;
            std::vector<VkSwapchainKHR> _swapChainIDs;
    };

    using SequenceSPtr = std::shared_ptr<Sequence>;
    using SequenceWPtr = std::weak_ptr<Sequence>;

    using QueueSequence     = std::vector<SequenceSPtr>;
    using QueueSequenceSPtr = std::shared_ptr<QueueSequence>;
    using QueueSequenceWPtr = std::weak_ptr<QueueSequence>;
    using QueueSequences    = std::vector<QueueSequenceSPtr>;
}