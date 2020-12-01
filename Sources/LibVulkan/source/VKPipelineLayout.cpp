/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibVulkan/include/VKDevice.h"
#include "LibVulkan/include/VKPipelineLayout.h"

namespace Vulkan
{

PipelineLayout::PipelineLayout() :
_layout(VK_NULL_HANDLE)
{
    MOUCA_PRE_CONDITION(isNull());
}

PipelineLayout::~PipelineLayout()
{
    MOUCA_PRE_CONDITION(isNull());
}

void PipelineLayout::addPushConstant(const VkPushConstantRange& constant)
{
    MOUCA_PRE_CONDITION(isNull());

    _constants.emplace_back(constant);
}

void PipelineLayout::initialize(const Device& device, const std::vector<VkDescriptorSetLayout>& descriptorLayouts)
{
    MOUCA_PRE_CONDITION(isNull());
    MOUCA_PRE_CONDITION(!device.isNull());

    VkPipelineLayoutCreateInfo layoutCreateInfo =
    {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,                      // VkStructureType                sType
        nullptr,                                                            // const void                    *pNext
        0,                                                                  // VkPipelineLayoutCreateFlags    flags
        static_cast<uint32_t>(descriptorLayouts.size()),                    // uint32_t                       setLayoutCount
        (descriptorLayouts.empty()) ? nullptr : descriptorLayouts.data(),   // const VkDescriptorSetLayout   *pSetLayouts
        static_cast<uint32_t>(_constants.size()),                           // uint32_t                       pushConstantRangeCount
        (_constants.empty()) ? nullptr : _constants.data()                  // const VkPushConstantRange     *pPushConstantRanges
    };

    if(vkCreatePipelineLayout(device.getInstance(), &layoutCreateInfo, nullptr, &_layout) != VK_SUCCESS)
    {
        MOUCA_THROW_ERROR(u8"Vulkan", u8"PipelineLayoutCreationError");
    }
}

void PipelineLayout::release(const Device& device)
{
    MOUCA_ASSERT(!isNull());

    vkDestroyPipelineLayout(device.getInstance(), _layout, nullptr);
    _layout = VK_NULL_HANDLE;
}

}