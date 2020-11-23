/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include "LibVulkan/include/VKDevice.h"
#include "LibVulkan/include/VKFence.h"

namespace Vulkan
{

Fence::Fence():
_fence(VK_NULL_HANDLE), _timeout(0)
{
    BT_PRE_CONDITION(isNull());
}

Fence::~Fence()
{
    BT_PRE_CONDITION(isNull());
}

void Fence::initialize(const Device& device, const VkFenceCreateFlags flag)
{
    BT_PRE_CONDITION(isNull());
    BT_PRE_CONDITION(!device.isNull());

    const VkFenceCreateInfo fenceCreateInfo =
    {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,    // VkStructureType       sType;
        nullptr,                                // const void*           pNext;
        flag                                    // VkFenceCreateFlags    flags;
    };

    if(vkCreateFence(device.getInstance(), &fenceCreateInfo, nullptr, &_fence) != VK_SUCCESS)
    {
        BT_THROW_ERROR(u8"Vulkan", u8"FenceCreationError");
    }

    BT_POST_CONDITION(!isNull());
}

void Fence::initialize(const Device& device, uint64_t timeout, const VkFenceCreateFlags flag)
{
    Fence::initialize(device, flag);

    _timeout = timeout;
}

VkResult Fence::wait(const Device& device) const
{
    BT_PRE_CONDITION(!isNull());
    BT_PRE_CONDITION(!device.isNull());

    return vkWaitForFences(device.getInstance(), 1, &_fence, VK_TRUE, _timeout);
}

void Fence::release(const Device& device)
{
    BT_PRE_CONDITION(!isNull());
    BT_PRE_CONDITION(!device.isNull());

    vkDestroyFence(device.getInstance(), _fence, nullptr);
    _fence = VK_NULL_HANDLE;

    BT_POST_CONDITION(isNull());
}

}