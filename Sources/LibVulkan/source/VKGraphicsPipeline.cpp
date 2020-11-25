/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include "LibVulkan/include/VKGraphicsPipeline.h"

#include "LibVulkan/include/VKDevice.h"
#include <LibVulkan/include/VKPipelineCache.h>
#include "LibVulkan/include/VKPipelineLayout.h"
#include "LibVulkan/include/VKPipelineStates.h"
#include "LibVulkan/include/VKRenderPass.h"

namespace Vulkan
{

GraphicsPipeline::GraphicsPipeline() :
_pipeline(VK_NULL_HANDLE)
{
    MOUCA_PRE_CONDITION(isNull());
}

GraphicsPipeline::~GraphicsPipeline()
{
    MOUCA_POST_CONDITION(isNull());
}

void GraphicsPipeline::initialize(const Device& device, const RenderPass& renderPass, const PipelineLayout& layout, const PipelineCache& pipelineCache)
{
    MOUCA_PRE_CONDITION(isNull());                                 // DEV Issue: Must be never initialize !
    MOUCA_PRE_CONDITION(!device.isNull());                         // DEV Issue: 
    MOUCA_PRE_CONDITION(!renderPass.isNull());                     // DEV Issue: 
    MOUCA_PRE_CONDITION(!layout.isNull());                         // DEV Issue: 
    MOUCA_PRE_CONDITION(_infos.getStages().getNbShaders() > 0);    // DEV Issue: Need at least one shader program linked !

    const std::array<VkGraphicsPipelineCreateInfo, 1> infos{ _infos.buildInfo(renderPass, layout) };

    if (vkCreateGraphicsPipelines(device.getInstance(), pipelineCache.getInstance(), static_cast<uint32_t>(infos.size()), infos.data(), nullptr, &_pipeline) != VK_SUCCESS)
    {
        BT_THROW_ERROR(u8"Vulkan", u8"PipelineGraphicCreationError");
    }
    MOUCA_POST_CONDITION(!isNull()); //Dev Issue: Must be initialize !
}

void GraphicsPipeline::initialize(const Device& device, RenderPassWPtr renderPass, PipelineLayoutWPtr layout, PipelineCacheWPtr pipelineCache)
{
    MOUCA_PRE_CONDITION(isNull());                                 // DEV Issue: Must be never initialize !
    MOUCA_PRE_CONDITION(!device.isNull());                         // DEV Issue: 
    MOUCA_PRE_CONDITION(!renderPass.lock()->isNull());              // DEV Issue: 
    MOUCA_PRE_CONDITION(!layout.lock()->isNull());                  // DEV Issue: 
    MOUCA_PRE_CONDITION(_infos.getStages().getNbShaders() > 0);    // DEV Issue: Need at least one shader program linked !
    
    // Keep data
    _renderPass     = renderPass;
    _layout         = layout;
    _pipelineCache  = pipelineCache;

    const std::array<VkGraphicsPipelineCreateInfo, 1> infos { _infos.buildInfo(*renderPass.lock(), *layout.lock()) };
    const VkPipelineCache cache = pipelineCache.expired() ? VK_NULL_HANDLE : pipelineCache.lock()->getInstance();

    if(vkCreateGraphicsPipelines(device.getInstance(), cache, static_cast<uint32_t>(infos.size()), infos.data(), nullptr, &_pipeline) != VK_SUCCESS)
    {
        BT_THROW_ERROR(u8"Vulkan", u8"PipelineGraphicCreationError");
    }
    MOUCA_POST_CONDITION(!isNull()); //Dev Issue: Must be initialize !
}

void GraphicsPipeline::release(const Device& device)
{
    MOUCA_PRE_CONDITION(!isNull()); //Dev Issue: Must be initialize !

    vkDestroyPipeline(device.getInstance(), _pipeline, nullptr);
    _pipeline = VK_NULL_HANDLE;

    MOUCA_POST_CONDITION(isNull()); //Dev Issue: Must be clean !
}

}