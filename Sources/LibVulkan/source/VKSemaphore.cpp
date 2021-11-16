/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibVulkan/include/VKDevice.h"
#include "LibVulkan/include/VKSemaphore.h"

namespace Vulkan
{

Semaphore::Semaphore():
_semaphore(VK_NULL_HANDLE)
{
    MouCa::assertion(isNull());
}

Semaphore::~Semaphore()
{
    MouCa::assertion(isNull());
}

void Semaphore::initialize(const Device& device)
{
    MouCa::assertion(isNull());
    VkSemaphoreCreateInfo semaphore_create_info =
    {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,      // VkStructureType          sType
        nullptr,                                      // const void*              pNext
        0                                             // VkSemaphoreCreateFlags   flags
    };

    if (vkCreateSemaphore(device.getInstance(), &semaphore_create_info, nullptr, &_semaphore) != VK_SUCCESS)
    {
        throw Core::Exception(Core::ErrorData("Vulkan", "SemaphoreCreationError"));
    }
}

void Semaphore::release(const Device& device)
{
    MouCa::assertion(!isNull());
    vkDestroySemaphore(device.getInstance(), _semaphore, nullptr);
    _semaphore = VK_NULL_HANDLE;
}

}