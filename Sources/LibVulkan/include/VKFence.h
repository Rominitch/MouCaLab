/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace Vulkan
{
    class Device;

    class Fence final
    {
        MOUCA_NOCOPY(Fence);

        public:
            static const uint64_t infinityTimeout = std::numeric_limits<uint64_t>().max();

            Fence();
            ~Fence();

            void initialize(const Device& device, const VkFenceCreateFlags flag);

            [[deprecated]] void initialize(const Device& device, uint64_t timeout, const VkFenceCreateFlags flag);
            [[deprecated]] VkResult wait(const Device& device) const;

            void release(const Device& device);

            const VkFence& getInstance() const
            {
                MouCa::preCondition(!isNull());
                return _fence;
            }

            bool isNull() const
            {
                return _fence == VK_NULL_HANDLE;
            }

        private:
            VkFence     _fence;
            uint64_t  _timeout;   // Timeout in nanosecond.
    };

    using FenceSPtr = std::shared_ptr<Fence>;
    using FenceWPtr = std::weak_ptr<Fence>;
}