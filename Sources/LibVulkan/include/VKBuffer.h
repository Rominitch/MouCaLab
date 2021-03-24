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
            MemoryBufferUPtr        _memory;

            VkDescriptorBufferInfo  _descriptor;

            // Control variable
            VkDeviceSize            _currentSize;
            VkBufferUsageFlags      _usageFlags;
            VkBufferCreateFlags     _createFlags;

            MOUCA_NOCOPY(Buffer);

        public:
            Buffer(MemoryBufferUPtr memory);
            virtual ~Buffer();
        
            void initialize(const Device& device, const VkBufferCreateFlags createFlag, const VkBufferUsageFlags usageFlags, VkDeviceSize size, const void *data = nullptr);

            void resize(const Device& device, VkDeviceSize size);

            void release(const Device& device);

            bool isNull() const
            {
                MOUCA_PRE_CONDITION(_memory != nullptr);
                return _memory->isNull() && _buffer == VK_NULL_HANDLE;
            }

            Memory& getMemory()
            {
                return *_memory;
            }

            const Memory& getMemory() const
            {
                return *_memory;
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

            const VkDeviceAddress getDeviceAddress(const Device& device) const;

            static VkDeviceSize alignedSize(const VkDeviceSize value, const VkDeviceSize alignment)
            {
                return (value + alignment - 1) & ~(alignment - 1);
            }
    };

    using BufferSPtr = std::shared_ptr<Buffer>;
    using BufferUPtr = std::unique_ptr<Buffer>;
    using BufferWPtr = std::weak_ptr<Buffer>;
}