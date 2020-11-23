/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include "LibVulkan/include/VKMemory.h"

namespace Vulkan
{
    class Device;

    class Buffer
    {
        private:
            VkBuffer                _buffer;
            MemoryBuffer            _memory;

            VkDescriptorBufferInfo  _descriptor;

            // Control variable
            VkDeviceSize            _currentSize;
            VkMemoryPropertyFlags   _memoryProperty;
            VkBufferUsageFlags      _usageFlags;

            MOUCA_NOCOPY(Buffer);

        public:
            Buffer();
            ~Buffer();
        
            void initialize(const Device& device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, const void *data = nullptr);

            void resize(const Device& device, VkDeviceSize size);

            void release(const Device& device);

            bool isNull() const
            {
                return _buffer == VK_NULL_HANDLE;
            }

            Memory& getMemory()
            {
                return _memory;
            }

            const Memory& getMemory() const
            {
                return _memory;
            }

            const VkBuffer& getBuffer() const
            {
                return _buffer;
            }

            VkDeviceSize getSize() const
            {
                return _currentSize;
            }

            const VkDescriptorBufferInfo& getDescriptor() const;
    };
}