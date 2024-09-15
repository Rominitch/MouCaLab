/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibVulkan/include/VKGraphicsPipeline.h"

#include "LibVulkan/include/VKDevice.h"
#include <LibVulkan/include/VKPipelineCache.h>
#include "LibVulkan/include/VKPipelineLayout.h"
#include "LibVulkan/include/VKPipelineStates.h"
#include "LibVulkan/include/VKRenderPass.h"

namespace Vulkan
{

GraphicsPipeline::GraphicsPipeline() :
Pipeline()
{
    MouCa::preCondition(isNull());
}

GraphicsPipeline::~GraphicsPipeline()
{
    MouCa::postCondition(isNull());
}

void GraphicsPipeline::initialize(const Device& device, const RenderPass& renderPass, const PipelineLayout& layout, const PipelineCache& pipelineCache)
{
    MouCa::preCondition(isNull());                                 // DEV Issue: Must be never initialize !
    MouCa::preCondition(!device.isNull());                         // DEV Issue: 
    MouCa::preCondition(!renderPass.isNull());                     // DEV Issue: 
    MouCa::preCondition(!layout.isNull());                         // DEV Issue: 
    MouCa::preCondition(_infos.getStages().getNbShaders() > 0);    // DEV Issue: Need at least one shader program linked !

    const std::array<VkGraphicsPipelineCreateInfo, 1> infos{ _infos.buildInfo(renderPass, layout) };

    if (vkCreateGraphicsPipelines(device.getInstance(), pipelineCache.getInstance(), static_cast<uint32_t>(infos.size()), infos.data(), nullptr, &_pipeline) != VK_SUCCESS)
    {
        throw Core::Exception(Core::ErrorData("Vulkan", "PipelineGraphicCreationError"));
    }
    MouCa::postCondition(!isNull()); //Dev Issue: Must be initialize !
}

void GraphicsPipeline::initialize(const Device& device, RenderPassWPtr renderPass, PipelineLayoutWPtr layout, PipelineCacheWPtr pipelineCache)
{
    MouCa::preCondition(isNull());                                 // DEV Issue: Must be never initialize !
    MouCa::preCondition(!device.isNull());                         // DEV Issue: 
    MouCa::preCondition(!renderPass.lock()->isNull());              // DEV Issue: 
    MouCa::preCondition(!layout.lock()->isNull());                  // DEV Issue: 
    MouCa::preCondition(_infos.getStages().getNbShaders() > 0);    // DEV Issue: Need at least one shader program linked !
    
    // Keep data
    _renderPass     = renderPass;
    _layout         = layout;
    _pipelineCache  = pipelineCache;

    const std::array<VkGraphicsPipelineCreateInfo, 1> infos { _infos.buildInfo(*renderPass.lock(), *layout.lock()) };
    const VkPipelineCache cache = pipelineCache.expired() ? VK_NULL_HANDLE : pipelineCache.lock()->getInstance();

    if(vkCreateGraphicsPipelines(device.getInstance(), cache, static_cast<uint32_t>(infos.size()), infos.data(), nullptr, &_pipeline) != VK_SUCCESS)
    {
        throw Core::Exception(Core::ErrorData("Vulkan", "PipelineGraphicCreationError"));
    }
    MouCa::postCondition(!isNull()); //Dev Issue: Must be initialize !
}

void GraphicsPipeline::release(const Device& device)
{
    MouCa::preCondition(!isNull()); //Dev Issue: Must be initialize !

    vkDestroyPipeline(device.getInstance(), _pipeline, nullptr);
    _pipeline = VK_NULL_HANDLE;

    MouCa::postCondition(isNull()); //Dev Issue: Must be clean !
}

}