/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include "LibVulkan/include/VKDevice.h"
#include "LibVulkan/include/VKMemory.h"

namespace Vulkan
{

Memory::Memory():
_memory(VK_NULL_HANDLE),
_mapped(nullptr),
_memoryPropertyFlags(0),
_allocatedSize(0),
_alignment(0)
{
    MOUCA_ASSERT(isNull());
}

Memory::~Memory()
{
    MOUCA_ASSERT(isNull());
}

void Memory::release(const Device& device)
{
    MOUCA_ASSERT(!isNull());
    MOUCA_ASSERT(!device.isNull());
    MOUCA_ASSERT(_mapped == nullptr);   //DEV Issue: need unmapped !!!
    
    vkFreeMemory(device.getInstance(), _memory, nullptr);
    _memory = VK_NULL_HANDLE;

    _memoryPropertyFlags = 0;
    _allocatedSize = 0;
    _alignment = 0;

    MOUCA_ASSERT(isNull());
}

void Memory::allocateMemory(const Device& device, const VkMemoryRequirements& memRequirements)
{
    MOUCA_ASSERT(isNull());
    MOUCA_ASSERT(!device.isNull());

    VkMemoryAllocateInfo memAllocateInfo =
    {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,                     // VkStructureType    sType;
        nullptr,                                                    // const void*        pNext;
        memRequirements.size,                                       // VkDeviceSize       allocationSize;
        device.getMemoryType(memRequirements, _memoryPropertyFlags) // uint32_t           memoryTypeIndex;
    };

    if(vkAllocateMemory(device.getInstance(), &memAllocateInfo, nullptr, &_memory) != VK_SUCCESS)
    {
        BT_THROW_ERROR(u8"Vulkan", u8"BufferMemoryAllocateError");
    }

    _alignment     = memRequirements.alignment;
    _allocatedSize = memAllocateInfo.allocationSize;
}

void Memory::copy(const Device& device, const VkDeviceSize size, const void *data)
{
    MOUCA_PRE_CONDITION(!isNull());
    MOUCA_PRE_CONDITION(!device.isNull());
    MOUCA_PRE_CONDITION(_mapped == nullptr);
    MOUCA_PRE_CONDITION(size != 0);
    MOUCA_PRE_CONDITION(data != nullptr);
    MOUCA_PRE_CONDITION(_memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);      /// DEV Issue: Only hosted can use this API ! 

    map(device, size);

    MOUCA_ASSERT(_mapped != nullptr);
    memcpy(_mapped, data, size);

    unmap(device);

    MOUCA_POST_CONDITION(_mapped == nullptr);
}

void Memory::map(const Device& device, const VkDeviceSize size, const VkDeviceSize offset)
{
    MOUCA_ASSERT(!isNull());
    MOUCA_ASSERT(!device.isNull());
    MOUCA_ASSERT(_mapped == nullptr);
    MOUCA_ASSERT(size != 0);

    // Map a part or whole of buffer
    if(vkMapMemory(device.getInstance(), _memory, offset, size, 0, &_mapped) != VK_SUCCESS)
    {
        BT_THROW_ERROR(u8"Vulkan", u8"BufferMappingError");
    }

    MOUCA_POST_CONDITION(_mapped != nullptr);
}

void Memory::unmap(const Device& device)
{
    MOUCA_PRE_CONDITION(!isNull());
    MOUCA_PRE_CONDITION(!device.isNull());
    MOUCA_PRE_CONDITION(_mapped != nullptr);

    vkUnmapMemory(device.getInstance(), _memory);
    _mapped = nullptr;
}

void Memory::flush(const Device& device, const VkDeviceSize size, const VkDeviceSize offset) const
{
    MOUCA_PRE_CONDITION(!isNull());
    MOUCA_PRE_CONDITION(!device.isNull());
    MOUCA_PRE_CONDITION(_mapped != nullptr);

    const VkMappedMemoryRange mappedRange = 
    {
        VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,  // VkStructureType    sType;
        nullptr,                                // const void*        pNext;
        _memory,                                // VkDeviceMemory     memory;
        offset,                                 // VkDeviceSize       offset;
        size                                    // VkDeviceSize       size;
    };

    if(vkFlushMappedMemoryRanges(device.getInstance(), 1, &mappedRange) != VK_SUCCESS)
    {
        BT_THROW_ERROR(u8"Vulkan", u8"FlushMappedBufferError");
    }
    MOUCA_POST_CONDITION(_mapped != nullptr);
}

void MemoryBuffer::initialize(const Device& device, const VkBuffer& buffer, const VkMemoryPropertyFlags memoryPropertyFlags)
{
    MOUCA_ASSERT(isNull());
    MOUCA_ASSERT(!device.isNull());
    MOUCA_ASSERT(buffer != VK_NULL_HANDLE);
    MOUCA_ASSERT(_mapped == nullptr);

    _memoryPropertyFlags = memoryPropertyFlags;

    //Create the memory backing up the buffer handle
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device.getInstance(), buffer, &memRequirements);

    //Create memory
    allocateMemory(device, memRequirements);

    // Attach the memory to the buffer object
    if(vkBindBufferMemory(device.getInstance(), buffer, _memory, 0) != VK_SUCCESS)
    {
        BT_THROW_ERROR(u8"Vulkan", u8"BufferBindingError");
    }
}

void MemoryImage::initialize(const Device& device, const VkImage& image, const VkMemoryPropertyFlags memoryPropertyFlags)
{
    MOUCA_ASSERT(isNull());
    MOUCA_ASSERT(!device.isNull());
    MOUCA_ASSERT(image != VK_NULL_HANDLE);
    MOUCA_ASSERT(_mapped == nullptr);

    _memoryPropertyFlags = memoryPropertyFlags;

    //Create the memory backing up the image handle
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device.getInstance(), image, &memRequirements);

    //Create memory
    allocateMemory(device, memRequirements);

    // Attach the memory to the image object
    if(vkBindImageMemory(device.getInstance(), image, _memory, 0) != VK_SUCCESS)
    {
        BT_THROW_ERROR(u8"Vulkan", u8"ImageBindingError");
    }
}

};