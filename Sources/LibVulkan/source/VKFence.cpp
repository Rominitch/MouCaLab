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
    MouCa::preCondition(isNull());
}

Fence::~Fence()
{
    MouCa::preCondition(isNull());
}

void Fence::initialize(const Device& device, const VkFenceCreateFlags flag)
{
    MouCa::preCondition(isNull());
    MouCa::preCondition(!device.isNull());

    const VkFenceCreateInfo fenceCreateInfo =
    {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,    // VkStructureType       sType;
        nullptr,                                // const void*           pNext;
        flag                                    // VkFenceCreateFlags    flags;
    };

    if(vkCreateFence(device.getInstance(), &fenceCreateInfo, nullptr, &_fence) != VK_SUCCESS)
    {
        throw Core::Exception(Core::ErrorData("Vulkan", "FenceCreationError"));
    }

    MouCa::postCondition(!isNull());
}

void Fence::initialize(const Device& device, uint64_t timeout, const VkFenceCreateFlags flag)
{
    Fence::initialize(device, flag);

    _timeout = timeout;
}

VkResult Fence::wait(const Device& device) const
{
    MouCa::preCondition(!isNull());
    MouCa::preCondition(!device.isNull());

    return vkWaitForFences(device.getInstance(), 1, &_fence, VK_TRUE, _timeout);
}

void Fence::release(const Device& device)
{
    MouCa::preCondition(!isNull());
    MouCa::preCondition(!device.isNull());

    vkDestroyFence(device.getInstance(), _fence, nullptr);
    _fence = VK_NULL_HANDLE;

    MouCa::postCondition(isNull());
}

}