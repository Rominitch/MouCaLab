/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace Vulkan
{
    class Device;
 
    class Semaphore
    {
        MOUCA_NOCOPY(Semaphore);

        public:
            Semaphore();
            ~Semaphore();

            void initialize(const Device& device);
            void release(const Device& device);

            const VkSemaphore& getInstance() const
            {
                return _semaphore;
            }

            bool isNull() const
            {
                return _semaphore == VK_NULL_HANDLE;
            }

        private:
            VkSemaphore _semaphore;
    };

    using SemaphoreSPtr = std::shared_ptr<Semaphore>;
    using SemaphoreWPtr = std::weak_ptr<Semaphore>;
    using Semaphores = std::vector<SemaphoreSPtr>;
}