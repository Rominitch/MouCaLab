#include "Dependancies.h"

#include "LibVulkan/include/VKEnvironment.h"

#include "MouCa3DVulkanEngine/include/ContextDevice.h"

namespace MouCa3DEngine
{

ContextDevice::~ContextDevice()
{
    BT_POST_CONDITION(isNull());        //Dev Issue: Need to call release
}

void ContextDevice::initialize(const Vulkan::Environment& environment, const std::vector<const char*>& deviceExtensions, const VkPhysicalDeviceFeatures& enabled, const Vulkan::Surface* surface)
{
    BT_PRE_CONDITION(isNull());                 //DEV Issue: No re-entrance: call release before initialize again.
    BT_PRE_CONDITION(!environment.isNull());    //DEV Issue:  Environment is not ready.

    // Create best device for rendering and adapted to surface.
    _device.initializeBestGPU(environment, deviceExtensions, surface, enabled);

    // Create CommandPool
    _commandPool.initialize(_device, _device.getQueueFamilyGraphicId());

    // Create Pipeline Cache
    _pipelineCache.initialize(_device);

    BT_PRE_CONDITION(!isNull()); //DEV Issue: Something wrong ?
}

void ContextDevice::release()
{
    BT_PRE_CONDITION(!isNull()); //DEV Issue: initialize() must be call before !

    // Release all item which use device
    _pipelineCache.release(_device);
    _commandPool.release(_device);

    // Finally Release device
    _device.release();

    BT_PRE_CONDITION(isNull()); //DEV Issue: Something wrong ?
}

}