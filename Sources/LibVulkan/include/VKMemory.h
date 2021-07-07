/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace Vulkan
{
    class Device;

    class Memory
    {
        MOUCA_NOCOPY(Memory);

        protected:
            VkDeviceMemory          _memory;
            void*                   _mapped;
            VkMemoryPropertyFlags   _memoryPropertyFlags;
            VkDeviceSize            _allocatedSize;
            VkDeviceSize            _alignment;

            Memory(const VkMemoryPropertyFlags memoryPropertyFlags=0);

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

            virtual const void* getNext() const { return nullptr; }

            //--------------------------------------------------------------------------
            //									Mapping
            //--------------------------------------------------------------------------
            void map(const Device& device, const VkDeviceSize size = VK_WHOLE_SIZE, const VkDeviceSize offset = 0);

            void flush(const Device& device, const VkDeviceSize size = VK_WHOLE_SIZE, const VkDeviceSize offset = 0) const;

            void unmap(const Device& device);

            void copy(const Device& device, const VkDeviceSize size, const void *data);
    };

    using MemoryUPtr = std::unique_ptr<Memory>;

    class MemoryBuffer : public Memory
    {
        public:
            MemoryBuffer(const VkMemoryPropertyFlags memoryPropertyFlags):
            Memory(memoryPropertyFlags)
            {}

            // Destructor
            ~MemoryBuffer() override = default;

            void initialize(const Device& device, const VkBuffer& buffer);
    };

    using MemoryBufferUPtr = std::unique_ptr<MemoryBuffer>;

    class MemoryBufferAllocate : public MemoryBuffer
    {
        public:
            MemoryBufferAllocate(const VkMemoryPropertyFlags memoryPropertyFlags, const VkMemoryAllocateFlags allocateFlag) :
            MemoryBuffer(memoryPropertyFlags), _flagInfo({ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO, nullptr, allocateFlag, 0 })
            {}

            // Destructor
            ~MemoryBufferAllocate() override = default;

            const void* getNext() const override { return &_flagInfo; }

        private:
            VkMemoryAllocateFlagsInfo _flagInfo;
    };

    class MemoryImage : public Memory
    {
        public:
            // Destructor
            ~MemoryImage() override = default;

            void initialize(const Device& device, const VkImage& image, const VkMemoryPropertyFlags memoryPropertyFlags);
    };
};