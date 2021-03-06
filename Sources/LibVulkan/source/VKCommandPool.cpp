/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibVulkan/include/VKCommandPool.h"
#include "LibVulkan/include/VKDevice.h"

namespace Vulkan
{

CommandPool::CommandPool():
_commandPool(VK_NULL_HANDLE)
{
    MOUCA_PRE_CONDITION(isNull());
}

CommandPool::~CommandPool()
{
    MOUCA_POST_CONDITION(isNull());
}

void CommandPool::initialize(const Device& device, const uint32_t queueFamilyID, const VkCommandPoolCreateFlags flags)
{
    MOUCA_PRE_CONDITION(isNull());
    MOUCA_PRE_CONDITION(!device.isNull());

    VkCommandPoolCreateInfo createInfo =
    {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,         // VkStructureType              sType
        nullptr,                                            // const void*                  pNext
        flags,                                              // VkCommandPoolCreateFlags     flags
        queueFamilyID                                       // uint32_t                     queueFamilyIndex
    };

    // Build command pool
    if(vkCreateCommandPool(device.getInstance(), &createInfo, nullptr, &_commandPool) != VK_SUCCESS)
    {
        MOUCA_THROW_ERROR(u8"Vulkan", u8"CommandPoolCreationError");
    }

    MOUCA_POST_CONDITION(!isNull());
}

void CommandPool::release(const Device& device)
{
    MOUCA_PRE_CONDITION(!isNull());
    MOUCA_PRE_CONDITION(!device.isNull());

    vkDestroyCommandPool(device.getInstance(), _commandPool, nullptr);
    _commandPool = VK_NULL_HANDLE;

    MOUCA_POST_CONDITION(isNull());
}

}