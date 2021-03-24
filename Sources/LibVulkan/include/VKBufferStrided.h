#pragma once

#include <LibVulkan/include/VKBuffer.h>

namespace Vulkan
{

    class BufferStrided : public Buffer
    {
        public:
            BufferStrided(MemoryBufferUPtr memory):
            Buffer(std::move(memory))
            {}

            ~BufferStrided() override = default;

            void initialize(const Device& device, const VkBufferCreateFlags createFlags, const VkBufferUsageFlags usageFlags, 
                            const VkDeviceSize size, const VkDeviceSize handleSize, const VkDeviceSize handleAligned,
                            const void *data = nullptr)
            {
                Buffer::initialize(device, createFlags, usageFlags, size * handleSize, data);

                const VkDeviceSize handleSizeAligned = alignedSize(handleSize, handleAligned);
                _strided =
                {
                    getDeviceAddress(device),   // VkDeviceAddress    deviceAddress;
                    handleSizeAligned,          // VkDeviceSize       stride;
                    size * handleSizeAligned,   // VkDeviceSize       size;
                };
            }

            const VkStridedDeviceAddressRegionKHR& getStrided() const { return _strided; }

        private:
            VkStridedDeviceAddressRegionKHR _strided;
    };
};
