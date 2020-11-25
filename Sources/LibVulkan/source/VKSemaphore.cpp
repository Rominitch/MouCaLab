/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include "LibVulkan/include/VKDevice.h"
#include "LibVulkan/include/VKSemaphore.h"

namespace Vulkan
{

Semaphore::Semaphore():
_semaphore(VK_NULL_HANDLE)
{
    MOUCA_ASSERT(isNull());
}

Semaphore::~Semaphore()
{
    MOUCA_ASSERT(isNull());
}

void Semaphore::initialize(const Device& device)
{
    MOUCA_ASSERT(isNull());
    VkSemaphoreCreateInfo semaphore_create_info =
    {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,      // VkStructureType          sType
        nullptr,                                      // const void*              pNext
        0                                             // VkSemaphoreCreateFlags   flags
    };

    if (vkCreateSemaphore(device.getInstance(), &semaphore_create_info, nullptr, &_semaphore) != VK_SUCCESS)
    {
        BT_THROW_ERROR(u8"Vulkan", u8"SemaphoreCreationError");
    }
}

void Semaphore::release(const Device& device)
{
    MOUCA_ASSERT(!isNull());
    vkDestroySemaphore(device.getInstance(), _semaphore, nullptr);
    _semaphore = VK_NULL_HANDLE;
}

}