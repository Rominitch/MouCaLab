#pragma once

#include <LibVulkan/include/VKBuffer.h>

namespace Vulkan
{

    class BufferStrided : public Buffer
    {
        public:
            BufferStrided(MemoryBufferUPtr memory):
            Buffer(std::move(memory)), _strided({0,0,0})
            {
                MouCa::preCondition(isNull());
            }

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

            void release(const Device& device) override
            {
                MouCa::preCondition(!isNull());

                _strided = { 0,0,0 };
                Buffer::release(device);

                MouCa::postCondition(isNull());
            }

            bool isNull() const override
            {
                return Buffer::isNull() && _strided.deviceAddress == 0 && _strided.size == 0;
            }

            const VkStridedDeviceAddressRegionKHR& getStrided() const { return _strided; }

        private:
            VkStridedDeviceAddressRegionKHR _strided;
    };

    using BufferStridedSPtr = std::shared_ptr<BufferStrided>;
    using BufferStridedUPtr = std::unique_ptr<BufferStrided>;
    using BufferStridedWPtr = std::weak_ptr<BufferStrided>;
};
