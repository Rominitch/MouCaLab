/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace Vulkan
{
    class Device;

    class Memory
    {
        private:
            MOUCA_NOCOPY(Memory);

        protected:
            VkDeviceMemory          _memory;
            void*                   _mapped;
            VkMemoryPropertyFlags   _memoryPropertyFlags;
            VkDeviceSize            _allocatedSize;
            VkDeviceSize            _alignment;

            Memory();

            void allocateMemory(const Device& device, const VkMemoryRequirements& memRequirements);

        public:
            virtual ~Memory();

            template<typename DataType>
            const DataType* getMappedMemory() const
            {
                return reinterpret_cast<const DataType*>(_mapped);
            }

            template<typename DataType>
            DataType* getMappedMemory()
            {
                return reinterpret_cast<DataType*>(_mapped);
            }

            bool isNull() const
            {
                return _memory == VK_NULL_HANDLE;
            }

            void release(const Device& device);

            VkDeviceSize getAllocatedSize() const
            {
                return _allocatedSize;
            }

            VkMemoryPropertyFlags getFlags() const { return _memoryPropertyFlags; }

            //--------------------------------------------------------------------------
            //									Mapping
            //--------------------------------------------------------------------------
            void map(const Device& device, const VkDeviceSize size = VK_WHOLE_SIZE, const VkDeviceSize offset = 0);

            void flush(const Device& device, const VkDeviceSize size = VK_WHOLE_SIZE, const VkDeviceSize offset = 0) const;

            void unmap(const Device& device);

            void copy(const Device& device, const VkDeviceSize size, const void *data);
    };

    class MemoryBuffer : public Memory
    {
        public:
            // Destructor
            ~MemoryBuffer() override = default;

            void initialize(const Device& device, const VkBuffer& buffer, const VkMemoryPropertyFlags memoryPropertyFlags);
    };

    class MemoryImage : public Memory
    {
        public:
            // Destructor
            ~MemoryImage() override = default;

            void initialize(const Device& device, const VkImage& image, const VkMemoryPropertyFlags memoryPropertyFlags);
    };
};