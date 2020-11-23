/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include "LibVulkan/include/VKBuffer.h"
#include "LibVulkan/include/VKCommand.h"
#include "LibVulkan/include/VKCommandBuffer.h"
#include "LibVulkan/include/VKDevice.h"

namespace Vulkan
{

Buffer::Buffer():
_buffer(VK_NULL_HANDLE),
_descriptor({VK_NULL_HANDLE, 0, 0}),
_usageFlags(0),
_currentSize(0),
_memoryProperty(VK_NULL_HANDLE)
{
    BT_ASSERT(isNull());
}

Buffer::~Buffer()
{
    BT_ASSERT(isNull());
}

void Buffer::initialize(const Device& device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, const void *data)
{
    BT_ASSERT(isNull());
    BT_ASSERT(!device.isNull());
    BT_ASSERT(size > 0);

    //Create the buffer handle
    VkBufferCreateInfo bufferCreateInfo =
    {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,   // VkStructureType        sType;
        nullptr,                                // const void*            pNext;
        0,                                      // VkBufferCreateFlags    flags;
        size,                                   // VkDeviceSize           size;
        usageFlags,                             // VkBufferUsageFlags     usage;
        VK_SHARING_MODE_EXCLUSIVE,              // VkSharingMode          sharingMode;
        0,                                      // uint32_t               queueFamilyIndexCount;
        nullptr                                 // const uint32_t*        pQueueFamilyIndices;
    };

    //Build GPU Buffer
    if(vkCreateBuffer(device.getInstance(), &bufferCreateInfo, nullptr, &_buffer) != VK_SUCCESS)
    {
        BT_THROW_ERROR(u8"Vulkan", u8"BufferCreationError");
    }

    //Create memory
    _memory.initialize(device, _buffer, memoryPropertyFlags);

    // Copy control variable
    _usageFlags     = usageFlags;
    _currentSize    = size;
    _memoryProperty = memoryPropertyFlags;

    // Copy memory if existing
    if(data != nullptr)
    {
        _memory.copy(device, size, data);
    }

    // Initialize a default descriptor that covers the whole buffer size
    _descriptor = {_buffer, 0, VK_WHOLE_SIZE};
}

void Buffer::release(const Device& device)
{
    BT_PRE_CONDITION(!isNull());
    BT_PRE_CONDITION(!device.isNull()); 
    BT_PRE_CONDITION(!_memory.isNull());

    vkDestroyBuffer(device.getInstance(), _buffer, nullptr);
    _buffer = VK_NULL_HANDLE;

    _memory.release(device);
}

void Buffer::resize(const Device& device, VkDeviceSize size)
{
    BT_PRE_CONDITION(!isNull());
    BT_PRE_CONDITION(!device.isNull());
    BT_PRE_CONDITION(size > 0);

    release(device);
    initialize(device, _usageFlags, _memoryProperty, size);

    BT_POST_CONDITION(!isNull());
}

const VkDescriptorBufferInfo& Buffer::getDescriptor() const
{
    return _descriptor;
}

}