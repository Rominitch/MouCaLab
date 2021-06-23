/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibVulkan/include/VKRayTracingPipeline.h"

#include "LibVulkan/include/VKDevice.h"
#include "LibVulkan/include/VKPipelineLayout.h"

namespace Vulkan
{

void RayTracingPipeline::initialize(const Device& device, PipelineLayoutWPtr layout, uint32_t maxPipelineRayRecursionDepth)
{
    MOUCA_PRE_CONDITION(!device.isNull());
    MOUCA_PRE_CONDITION(isNull());

    _layout = layout;
    _maxPipelineRayRecursionDepth = maxPipelineRayRecursionDepth;

    VkRayTracingPipelineCreateInfoKHR createInfos = getInfo();

    if (device.vkCreateRayTracingPipelinesKHR(device.getInstance(),
        VK_NULL_HANDLE, // VkDeferredOperationKHR deferredOperation,
        VK_NULL_HANDLE, // VkPipelineCache        pipelineCache,
        1,
        &createInfos,
        nullptr,        // VkAllocationCallbacks*
        &_pipeline) != VK_SUCCESS)
    {
        MOUCA_THROW_ERROR(u8"Vulkan", u8"RayTracingPipelineCreationError");
    }

    MOUCA_POST_CONDITION(!isNull());
}

VkRayTracingPipelineCreateInfoKHR RayTracingPipeline::getInfo() const
{
    return 
    {
        VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR, //VkStructureType                                      sType;
        nullptr,                                                //const void* pNext;
        0,                                                      //VkPipelineCreateFlags                                flags;
        _shaderStages.getNbShaders(),                           //uint32_t                                             stageCount;
        _shaderStages.getStage(),                               //const VkPipelineShaderStageCreateInfo* pStages;
        static_cast<uint32_t>(_shaderGroups.size()),            //uint32_t                                             groupCount;
        _shaderGroups.data(),                                   //const VkRayTracingShaderGroupCreateInfoKHR* pGroups;
        _maxPipelineRayRecursionDepth,                          //uint32_t                                             maxPipelineRayRecursionDepth;
        nullptr,                                                //const VkPipelineLibraryCreateInfoKHR* pLibraryInfo;
        nullptr,                                                //const VkRayTracingPipelineInterfaceCreateInfoKHR* pLibraryInterface;
        nullptr,                                                //const VkPipelineDynamicStateCreateInfo* pDynamicState;
        _layout.lock()->getInstance(),                          //VkPipelineLayout                                     layout;
        VK_NULL_HANDLE,                                         //VkPipeline                                           basePipelineHandle;
        0,                                                      //int32_t                                              basePipelineIndex;
    };
}

void RayTracingPipeline::release(const Device& device)
{
    MOUCA_PRE_CONDITION(!device.isNull());
    MOUCA_PRE_CONDITION(!isNull());

    vkDestroyPipeline(device.getInstance(), _pipeline, nullptr);

    _pipeline = VK_NULL_HANDLE;

    MOUCA_POST_CONDITION(isNull());
}



}
