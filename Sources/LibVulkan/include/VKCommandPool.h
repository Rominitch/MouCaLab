/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace Vulkan
{
    class Device;

    class CommandPool
    {
        private:
            VkCommandPool _commandPool;

            MOUCA_NOCOPY(CommandPool);

        public:
            CommandPool();

            ~CommandPool();

            bool isNull() const
            {
                return _commandPool == VK_NULL_HANDLE;
            }

            void initialize(const Device& device, const uint32_t queueFamilyID, const VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

            void release(const Device& device);

            const VkCommandPool& getInstance() const
            {
                return _commandPool;
            }
    };
}