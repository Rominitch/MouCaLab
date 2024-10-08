/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibVulkan/include/VKDevice.h"
#include "LibVulkan/include/VKMemory.h"

namespace Vulkan
{

Memory::Memory(const VkMemoryPropertyFlags memoryPropertyFlags):
_memory(VK_NULL_HANDLE),
_mapped(nullptr),
_memoryPropertyFlags(memoryPropertyFlags),
_allocatedSize(0),
_alignment(0)
{
    MouCa::assertion(isNull());
}

Memory::~Memory()
{
    MouCa::assertion(isNull());
}

void Memory::release(const Device& device)
{
    MouCa::assertion(!isNull());
    MouCa::assertion(!device.isNull());
    MouCa::assertion(_mapped == nullptr);   //DEV Issue: need unmapped !!!
    
    vkFreeMemory(device.getInstance(), _memory, nullptr);
    _memory = VK_NULL_HANDLE;

    //_memoryPropertyFlags = 0;
    _allocatedSize = 0;
    _alignment = 0;

    MouCa::assertion(isNull());
}

void Memory::allocateMemory(const Device& device, const VkMemoryRequirements& memRequirements)
{
    MouCa::assertion(isNull());
    MouCa::assertion(!device.isNull());

    VkMemoryAllocateInfo memAllocateInfo =
    {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,                     // VkStructureType    sType;
        getNext(),                                                  // const void*        pNext;
        memRequirements.size,                                       // VkDeviceSize       allocationSize;
        device.getMemoryType(memRequirements, _memoryPropertyFlags) // uint32_t           memoryTypeIndex;
    };

    if(vkAllocateMemory(device.getInstance(), &memAllocateInfo, nullptr, &_memory) != VK_SUCCESS)
    {
        throw Core::Exception(Core::ErrorData("Vulkan", "BufferMemoryAllocateError"));
    }

    _alignment     = memRequirements.alignment;
    _allocatedSize = memAllocateInfo.allocationSize;
}

void Memory::copy(const Device& device, const VkDeviceSize size, const void *data)
{
    MouCa::preCondition(!isNull());
    MouCa::preCondition(!device.isNull());
    MouCa::preCondition(_mapped == nullptr);
    MouCa::preCondition(size != 0);
    MouCa::preCondition(data != nullptr);
    MouCa::preCondition(_memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);      /// DEV Issue: Only hosted can use this API ! 

    map(device, size);

    MouCa::assertion(_mapped != nullptr);
    memcpy(_mapped, data, size);

    unmap(device);

    MouCa::postCondition(_mapped == nullptr);
}

void Memory::map(const Device& device, const VkDeviceSize size, const VkDeviceSize offset)
{
    MouCa::assertion(!isNull());
    MouCa::assertion(!device.isNull());
    MouCa::assertion(_mapped == nullptr);
    MouCa::assertion(size != 0);

    // Map a part or whole of buffer
    if(vkMapMemory(device.getInstance(), _memory, offset, size, 0, &_mapped) != VK_SUCCESS)
    {
        throw Core::Exception(Core::ErrorData("Vulkan", "BufferMappingError"));
    }

    MouCa::postCondition(_mapped != nullptr);
}

void Memory::unmap(const Device& device)
{
    MouCa::preCondition(!isNull());
    MouCa::preCondition(!device.isNull());
    MouCa::preCondition(_mapped != nullptr);

    vkUnmapMemory(device.getInstance(), _memory);
    _mapped = nullptr;
}

void Memory::flush(const Device& device, const VkDeviceSize size, const VkDeviceSize offset) const
{
    MouCa::preCondition(!isNull());
    MouCa::preCondition(!device.isNull());
    MouCa::preCondition(_mapped != nullptr);

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
        throw Core::Exception(Core::ErrorData("Vulkan", "FlushMappedBufferError"));
    }
    MouCa::postCondition(_mapped != nullptr);
}

void MemoryBuffer::initialize(const Device& device, const VkBuffer& buffer)
{
    MouCa::assertion(isNull());
    MouCa::assertion(!device.isNull());
    MouCa::assertion(buffer != VK_NULL_HANDLE);
    MouCa::assertion(_mapped == nullptr);

    //Create the memory backing up the buffer handle
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device.getInstance(), buffer, &memRequirements);

    //Create memory
    allocateMemory(device, memRequirements);

    // Attach the memory to the buffer object
    if(vkBindBufferMemory(device.getInstance(), buffer, _memory, 0) != VK_SUCCESS)
    {
        throw Core::Exception(Core::ErrorData("Vulkan", "BufferBindingError"));
    }
}

void MemoryImage::initialize(const Device& device, const VkImage& image, const VkMemoryPropertyFlags memoryPropertyFlags)
{
    MouCa::assertion(isNull());
    MouCa::assertion(!device.isNull());
    MouCa::assertion(image != VK_NULL_HANDLE);
    MouCa::assertion(_mapped == nullptr);

    _memoryPropertyFlags = memoryPropertyFlags;

    //Create the memory backing up the image handle
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device.getInstance(), image, &memRequirements);

    //Create memory
    allocateMemory(device, memRequirements);

    // Attach the memory to the image object
    if(vkBindImageMemory(device.getInstance(), image, _memory, 0) != VK_SUCCESS)
    {
        throw Core::Exception(Core::ErrorData("Vulkan", "ImageBindingError"));
    }
}

};