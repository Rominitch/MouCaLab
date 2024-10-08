/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibVulkan/include/VKBuffer.h"
#include "LibVulkan/include/VKCommand.h"
#include "LibVulkan/include/VKCommandBuffer.h"
#include "LibVulkan/include/VKDevice.h"

namespace Vulkan
{

Buffer::Buffer(MemoryBufferUPtr memory):
_buffer(VK_NULL_HANDLE),
_descriptor({VK_NULL_HANDLE, 0, 0}),
_usageFlags(0),
_createFlags(0),
_currentSize(0),
_memory(std::move(memory))
{
    MouCa::assertion(isNull());
}

Buffer::~Buffer()
{
    MouCa::assertion(isNull());
}

void Buffer::initialize(const Device& device, const VkBufferCreateFlags createFlag, const VkBufferUsageFlags usageFlags, VkDeviceSize size, const void *data)
{
    MouCa::assertion(isNull());
    MouCa::assertion(!device.isNull());
    MouCa::assertion(size > 0);

    //Create the buffer handle
    VkBufferCreateInfo bufferCreateInfo =
    {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,   // VkStructureType        sType;
        nullptr,                                // const void*            pNext;
        createFlag,                             // VkBufferCreateFlags    flags;
        size,                                   // VkDeviceSize           size;
        usageFlags,                             // VkBufferUsageFlags     usage;
        VK_SHARING_MODE_EXCLUSIVE,              // VkSharingMode          sharingMode;
        0,                                      // uint32_t               queueFamilyIndexCount;
        nullptr                                 // const uint32_t*        pQueueFamilyIndices;
    };

    //Build GPU Buffer
    if(vkCreateBuffer(device.getInstance(), &bufferCreateInfo, nullptr, &_buffer) != VK_SUCCESS)
    {
        throw Core::Exception(Core::ErrorData("Vulkan", "BufferCreationError"));
    }

    //Create memory
    _memory->initialize(device, _buffer);

    // Copy control variable
    _createFlags    = createFlag;
    _usageFlags     = usageFlags;
    _currentSize    = size;

    // Copy memory if existing
    if(data != nullptr)
    {
        _memory->copy(device, size, data);
    }

    // Initialize a default descriptor that covers the whole buffer size
    _descriptor = {_buffer, 0, VK_WHOLE_SIZE};
}

void Buffer::release(const Device& device)
{
    MouCa::preCondition(!isNull());
    MouCa::preCondition(!device.isNull()); 
    MouCa::preCondition(!_memory->isNull());

    vkDestroyBuffer(device.getInstance(), _buffer, nullptr);
    _buffer = VK_NULL_HANDLE;

    _memory->release(device);
}

void Buffer::resize(const Device& device, VkDeviceSize size)
{
    MouCa::preCondition(!isNull());
    MouCa::preCondition(!device.isNull());
    MouCa::preCondition(size > 0);

    release(device);
    initialize(device, _createFlags, _usageFlags, size);

    MouCa::postCondition(!isNull());
}

const VkDescriptorBufferInfo& Buffer::getDescriptor() const
{
    return _descriptor;
}

const VkDeviceAddress Buffer::getDeviceAddress(const Device& device) const
{
    const VkBufferDeviceAddressInfoKHR bufferDeviceAI
    {
        VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        nullptr,
        _buffer
    };
    //return vkGetBufferDeviceAddress(device.getInstance(), &bufferDeviceAI);
    return device.vkGetBufferDeviceAddressKHR(device.getInstance(), &bufferDeviceAI);
}

}