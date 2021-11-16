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
    MouCa::preCondition(isNull());
}

CommandPool::~CommandPool()
{
    MouCa::postCondition(isNull());
}

void CommandPool::initialize(const Device& device, const uint32_t queueFamilyID, const VkCommandPoolCreateFlags flags)
{
    MouCa::preCondition(isNull());
    MouCa::preCondition(!device.isNull());

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
        throw Core::Exception(Core::ErrorData("Vulkan", "CommandPoolCreationError"));
    }

    MouCa::postCondition(!isNull());
}

void CommandPool::release(const Device& device)
{
    MouCa::preCondition(!isNull());
    MouCa::preCondition(!device.isNull());

    vkDestroyCommandPool(device.getInstance(), _commandPool, nullptr);
    _commandPool = VK_NULL_HANDLE;

    MouCa::postCondition(isNull());
}

}