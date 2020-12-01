/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibVulkan/include/VKDevice.h"
#include "LibVulkan/include/VKFence.h"

namespace Vulkan
{

Fence::Fence():
_fence(VK_NULL_HANDLE), _timeout(0)
{
    MOUCA_PRE_CONDITION(isNull());
}

Fence::~Fence()
{
    MOUCA_PRE_CONDITION(isNull());
}

void Fence::initialize(const Device& device, const VkFenceCreateFlags flag)
{
    MOUCA_PRE_CONDITION(isNull());
    MOUCA_PRE_CONDITION(!device.isNull());

    const VkFenceCreateInfo fenceCreateInfo =
    {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,    // VkStructureType       sType;
        nullptr,                                // const void*           pNext;
        flag                                    // VkFenceCreateFlags    flags;
    };

    if(vkCreateFence(device.getInstance(), &fenceCreateInfo, nullptr, &_fence) != VK_SUCCESS)
    {
        MOUCA_THROW_ERROR(u8"Vulkan", u8"FenceCreationError");
    }

    MOUCA_POST_CONDITION(!isNull());
}

void Fence::initialize(const Device& device, uint64_t timeout, const VkFenceCreateFlags flag)
{
    Fence::initialize(device, flag);

    _timeout = timeout;
}

VkResult Fence::wait(const Device& device) const
{
    MOUCA_PRE_CONDITION(!isNull());
    MOUCA_PRE_CONDITION(!device.isNull());

    return vkWaitForFences(device.getInstance(), 1, &_fence, VK_TRUE, _timeout);
}

void Fence::release(const Device& device)
{
    MOUCA_PRE_CONDITION(!isNull());
    MOUCA_PRE_CONDITION(!device.isNull());

    vkDestroyFence(device.getInstance(), _fence, nullptr);
    _fence = VK_NULL_HANDLE;

    MOUCA_POST_CONDITION(isNull());
}

}