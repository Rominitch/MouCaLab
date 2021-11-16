/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibVulkan/include/VKPipelineStates.h"

#include "LibRT/include/RTBufferDescriptor.h"
#include "LibRT/include/RTViewport.h"

#include "LibVulkan/include/VKPipelineLayout.h"
#include "LibVulkan/include/VKRenderPass.h"
#include "LibVulkan/include/VKShaderProgram.h"

namespace Vulkan
{

void PipelineStageShaders::addShaderModule(const ShaderModuleWPtr shaderWeak, ShaderSpecialization&& specialization)
{
    MouCa::preCondition(!shaderWeak.expired());

    // Register data (keep memory alive)
    auto idData = _shaderData.size();
    _shaderData.emplace_back(ShaderInfo(shaderWeak, std::move(specialization)));
    const auto& data = _shaderData[idData];

    // Create Stage
    const VkSpecializationInfo* specializationInfo = data._specialization.isNull()
                                                   ? nullptr
                                                   : &data._specialization.getInstance();

    auto shader = shaderWeak.lock();

    const VkPipelineShaderStageCreateInfo info
    {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,    // VkStructureType                                sType
        nullptr,                                                // const void                                    *pNext
        0,                                                      // VkPipelineShaderStageCreateFlags               flags
        shader->getStage(),                                     // VkShaderStageFlagBits                          stage
        shader->getInstance(),                                  // VkShaderModule                                 module
        shader->getName().c_str(),                              // const char                                    *pName
        specializationInfo                                      // const VkSpecializationInfo                    *pSpecializationInfo
    };
    _shaderStage.emplace_back(info);
}

void PipelineStageShaders::addShaderGeneric(const ShaderProgram& shader, const VkShaderStageFlagBits stageFlag)
{
    const VkSpecializationInfo* specializationInfo = nullptr;
    const ShaderSpecialization& specialization = shader.getSpecialization();

    if (!specialization.isNull())
    {
        specializationInfo = &specialization.getInstance();
    }

    _shaderStage.push_back(
    {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,        // VkStructureType                                sType
        nullptr,                                                    // const void                                    *pNext
        0,                                                          // VkPipelineShaderStageCreateFlags               flags
        stageFlag,                                                  // VkShaderStageFlagBits                          stage
        shader.getInstance(),                                       // VkShaderModule                                 module
        shader.getName().c_str(),                                   // const char                                    *pName
        specializationInfo                                          // const VkSpecializationInfo                    *pSpecializationInfo
    });

    MouCa::postCondition(specializationInfo == nullptr || (specializationInfo->pData != nullptr && specializationInfo->dataSize > 0));
}

void PipelineStageShaders::addShaderProgram(const RT::ShaderKind id, const ShaderProgram& shader)
{
    MouCa::preCondition(id < RT::ShaderKind::NbShaders);

    const std::array<VkShaderStageFlagBits, static_cast<size_t>(RT::ShaderKind::NbShaders)> flags =
    {
        VK_SHADER_STAGE_VERTEX_BIT,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        VK_SHADER_STAGE_GEOMETRY_BIT,
        VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
        VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT
    };
    addShaderGeneric(shader, flags[static_cast<size_t>(id)]);
}

void PipelineStageShaders::addShaderVertex(const ShaderProgram& shader)
{
    addShaderGeneric(shader, VK_SHADER_STAGE_VERTEX_BIT);
}

void PipelineStageShaders::addShaderFragment(const ShaderProgram& shader)
{
    addShaderGeneric(shader, VK_SHADER_STAGE_FRAGMENT_BIT);
}

void PipelineStageShaders::addShaderTessellationControl(const ShaderProgram& shader)
{
    addShaderGeneric(shader, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
}

void PipelineStageShaders::addShaderTessellationEvaluation(const ShaderProgram& shader)
{
    addShaderGeneric(shader, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
}

void PipelineStageShaders::update()
{
    auto itShaderInfo = _shaderData.cbegin();
    for (auto& stage : _shaderStage)
    {
        stage.module = itShaderInfo->_module.lock()->getInstance();
        
        ++itShaderInfo;
    }
}

const VkPipelineShaderStageCreateInfo* PipelineStageShaders::getStage() const
{
    return _shaderStage.empty() ? nullptr : _shaderStage.data();
}

PipelineStateInputAssembly::PipelineStateInputAssembly():
_state(
{
    VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,  // VkStructureType                                sType
    nullptr,                                                      // const void                                    *pNext
    0,                                                            // VkPipelineInputAssemblyStateCreateFlags        flags
    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,                          // VkPrimitiveTopology                            topology
    VK_FALSE                                                      // VkBool32                                       primitiveRestartEnable
})
{}

PipelineStateVertexInput::PipelineStateVertexInput():
_state(
{
    VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,    // VkStructureType                                sType
    nullptr,                                                      // const void                                    *pNext
    0,                                                            // VkPipelineVertexInputStateCreateFlags          flags;
    0,                                                            // uint32_t                                       vertexBindingDescriptionCount
    nullptr,                                                      // const VkVertexInputBindingDescription         *pVertexBindingDescriptions
    0,                                                            // uint32_t                                       vertexAttributeDescriptionCount
    nullptr                                                       // const VkVertexInputAttributeDescription       *pVertexAttributeDescriptions
})
{}

void PipelineStateVertexInput::setBindingDescriptions(std::vector<VkVertexInputBindingDescription>&& bindings)
{
    _bindingDescriptions = bindings;
    
    _state.vertexBindingDescriptionCount = static_cast<uint32_t>(_bindingDescriptions.size());
    _state.pVertexBindingDescriptions    = _bindingDescriptions.empty() ? nullptr : _bindingDescriptions.data();
}

void PipelineStateVertexInput::setAttributeDescriptions(std::vector<VkVertexInputAttributeDescription>&& attributs)
{
    _attributeDescriptions = attributs;

    _state.vertexAttributeDescriptionCount = static_cast<uint32_t>(_attributeDescriptions.size());
    _state.pVertexAttributeDescriptions    = _attributeDescriptions.empty() ? nullptr : _attributeDescriptions.data();
}

PipelineStateViewport::PipelineStateViewport():
_state(
{
    VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,        // VkStructureType                                sType
    nullptr,                                                      // const void                                    *pNext
    0,                                                            // VkPipelineViewportStateCreateFlags             flags
    1,                                                            // uint32_t                                       viewportCount
    nullptr,                                                      // const VkViewport                              *pViewports
    1,                                                            // uint32_t                                       scissorCount
    nullptr                                                       // const VkRect2D                                *pScissors
})
{}

void PipelineStateViewport::addViewport(const VkViewport& viewport, const VkRect2D& scissor)
{
    _viewport.push_back(viewport);
    _scissor.push_back(scissor);

    _state.viewportCount = static_cast<uint32_t>(_viewport.size());
    _state.pViewports    = _viewport.data();
    _state.scissorCount  = static_cast<uint32_t>(_scissor.size());
    _state.pScissors     = _scissor.data();
}

void PipelineStateViewport::addDynamicViewport()
{
    _state.viewportCount = 1;
    _state.pViewports = nullptr;
    _state.scissorCount  = 1;
    _state.pScissors  = nullptr;
}

PipelineStateRasterizer::PipelineStateRasterizer():
_state({
    VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,   // VkStructureType                                sType
    nullptr,                                                      // const void                                    *pNext
    0,                                                            // VkPipelineRasterizationStateCreateFlags        flags
    VK_FALSE,                                                     // VkBool32                                       depthClampEnable
    VK_FALSE,                                                     // VkBool32                                       rasterizerDiscardEnable
    VK_POLYGON_MODE_FILL,                                         // VkPolygonMode                                  polygonMode
    VK_CULL_MODE_BACK_BIT,                                        // VkCullModeFlags                                cullMode
    VK_FRONT_FACE_COUNTER_CLOCKWISE,                              // VkFrontFace                                    frontFace
    VK_FALSE,                                                     // VkBool32                                       depthBiasEnable
    0.0f,                                                         // float                                          depthBiasConstantFactor
    0.0f,                                                         // float                                          depthBiasClamp
    0.0f,                                                         // float                                          depthBiasSlopeFactor
    1.0f                                                          // float                                          lineWidth
})
{}

PipelineStateMultisample::PipelineStateMultisample():
_state(
{
    VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,     // VkStructureType                                sType
    nullptr,                                                      // const void                                    *pNext
    0,                                                            // VkPipelineMultisampleStateCreateFlags          flags
    VK_SAMPLE_COUNT_1_BIT,                                        // VkSampleCountFlagBits                          rasterizationSamples
    VK_FALSE,                                                     // VkBool32                                       sampleShadingEnable
    0.0f,                                                         // float                                          minSampleShading
    nullptr,                                                      // const VkSampleMask                            *pSampleMask
    VK_FALSE,                                                     // VkBool32                                       alphaToCoverageEnable
    VK_FALSE                                                      // VkBool32                                       alphaToOneEnable
})
{}

PipelineStateBlending::PipelineStateBlending()
{
    _attachment.push_back(defaultAttachment());

    _state =
    {
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,     // VkStructureType                                sType
        nullptr,                                                      // const void                                    *pNext
        0,                                                            // VkPipelineColorBlendStateCreateFlags           flags
        VK_FALSE,                                                     // VkBool32                                       logicOpEnable
        VK_LOGIC_OP_CLEAR,                                            // VkLogicOp                                      logicOp
        static_cast<uint32_t>(_attachment.size()),                    // uint32_t                                       attachmentCount
        _attachment.data(),                                           // const VkPipelineColorBlendAttachmentState     *pAttachments
        {0.0f, 0.0f, 0.0f, 0.0f}                                      // float                                          blendConstants[4]
    };
}

VkPipelineColorBlendAttachmentState PipelineStateBlending::zeroAttachment()
{
    VkPipelineColorBlendAttachmentState state =
    {
        VK_FALSE,                                                     // VkBool32                                       blendEnable
        VK_BLEND_FACTOR_ZERO,                                         // VkBlendFactor                                  srcColorBlendFactor
        VK_BLEND_FACTOR_ZERO,                                         // VkBlendFactor                                  dstColorBlendFactor
        VK_BLEND_OP_ADD,                                              // VkBlendOp                                      colorBlendOp
        VK_BLEND_FACTOR_ZERO,                                         // VkBlendFactor                                  srcAlphaBlendFactor
        VK_BLEND_FACTOR_ZERO,                                         // VkBlendFactor                                  dstAlphaBlendFactor
        VK_BLEND_OP_ADD,                                              // VkBlendOp                                      alphaBlendOp
        getColorComponentRGBA()                                       // VkColorComponentFlags                          colorWriteMask
    };
    return state;
}

VkPipelineColorBlendAttachmentState PipelineStateBlending::defaultAttachment()
{
    VkPipelineColorBlendAttachmentState state =
    {
        VK_FALSE,                                                     // VkBool32                                       blendEnable
        VK_BLEND_FACTOR_ONE,                                          // VkBlendFactor                                  srcColorBlendFactor
        VK_BLEND_FACTOR_ZERO,                                         // VkBlendFactor                                  dstColorBlendFactor
        VK_BLEND_OP_ADD,                                              // VkBlendOp                                      colorBlendOp
        VK_BLEND_FACTOR_ONE,                                          // VkBlendFactor                                  srcAlphaBlendFactor
        VK_BLEND_FACTOR_ZERO,                                         // VkBlendFactor                                  dstAlphaBlendFactor
        VK_BLEND_OP_ADD,                                              // VkBlendOp                                      alphaBlendOp
        getColorComponentRGBA()                                       // VkColorComponentFlags                          colorWriteMask
    };
    return state;
}

void PipelineStateBlending::setColorBlendAttachments(const std::vector<VkPipelineColorBlendAttachmentState>& attachments)
{
    _attachment = attachments;
    _state.pAttachments    = _attachment.data();
    _state.attachmentCount = static_cast<int32_t>( _attachment.size() );
}

PipelineStateDynamic::PipelineStateDynamic():
_state(
{
    VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,   // VkStructureType                                sType
    nullptr,                                                // const void                                    *pNext
    0,                                                      // VkPipelineDynamicStateCreateFlags              flags
    0,                                                      // uint32_t                                       dynamicStateCount
    nullptr                                                 // const VkDynamicState                          *pDynamicStates
})
{}

void PipelineStateDynamic::addDynamicState(VkDynamicState state)
{
    _dynamicStates.emplace_back(state);

    _state.dynamicStateCount = static_cast<uint32_t>(_dynamicStates.size());
    _state.pDynamicStates    = _dynamicStates.data();
}

PipelineStateDepthStencil::PipelineStateDepthStencil():
_state(
{
    VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,     // VkStructureType                           sType
    nullptr,                                                        // const void*                               pNext
    0,                                                              // VkPipelineDepthStencilStateCreateFlags    flags
    VK_TRUE,                                                        // VkBool32                                  depthTestEnable
    VK_TRUE,                                                        // VkBool32                                  depthWriteEnable
    VK_COMPARE_OP_LESS_OR_EQUAL,                                    // VkCompareOp                               depthCompareOp
    VK_FALSE,                                                       // VkBool32                                  depthBoundsTestEnable
    VK_FALSE,                                                       // VkBool32                                  stencilTestEnable
    {},                                                             // VkStencilOpState                          front;
    {},                                                             // VkStencilOpState                          back;
    0.0f,                                                           // float                                     minDepthBounds
    0.0f                                                            // float                                     maxDepthBounds
})
{
    _state.back.compareOp = VK_COMPARE_OP_ALWAYS;
}

PipelineStateTessellation::PipelineStateTessellation():
_state(
{
    VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, // VkStructureType                           sType;
    nullptr,                                                   // const void*                               pNext;
    0,                                                         // VkPipelineTessellationStateCreateFlags    flags;
    1                                                          // uint32_t                                  patchControlPoints;
}
)
{}

PipelineStateCreateInfo::PipelineStateCreateInfo() :
_mode(Uninitialized), _subPass(0), _basePipelineHandle(VK_NULL_HANDLE), _basePipelineIndex(-1)
{}

void PipelineStateCreateInfo::initialize(const States mode)
{
    _mode = mode;
}

VkGraphicsPipelineCreateInfo PipelineStateCreateInfo::buildInfo(const RenderPass& renderPass, const PipelineLayout& layout) const
{
    return VkGraphicsPipelineCreateInfo
    {
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,       // VkStructureType                                sType
        nullptr,                                               // const void                                    *pNext
        0,                                                     // VkPipelineCreateFlags                          flags
        getStages().getNbShaders(),                            // uint32_t                                       stageCount
        getStages().getStage(),                                // const VkPipelineShaderStageCreateInfo         *pStages
        getInputVertexState(),                                 // const VkPipelineVertexInputStateCreateInfo    *pVertexInputState;
        getInputAssemblyState(),                               // const VkPipelineInputAssemblyStateCreateInfo  *pInputAssemblyState
        getTessellationState(),                                // const VkPipelineTessellationStateCreateInfo   *pTessellationState
        getViewportState(),                                    // const VkPipelineViewportStateCreateInfo       *pViewportState
        getRasterizerState(),                                  // const VkPipelineRasterizationStateCreateInfo  *pRasterizationState
        getMultisampleState(),                                 // const VkPipelineMultisampleStateCreateInfo    *pMultisampleState
        getDepthStencilState(),                                // const VkPipelineDepthStencilStateCreateInfo   *pDepthStencilState
        getBlendingState(),                                    // const VkPipelineColorBlendStateCreateInfo     *pColorBlendState
        getDynamicState(),                                     // const VkPipelineDynamicStateCreateInfo        *pDynamicState
        layout.getInstance(),                                  // VkPipelineLayout                               layout
        renderPass.getInstance(),                              // VkRenderPass                                   renderPass
        _subPass,                                              // uint32_t                                       subpass
        _basePipelineHandle,                                   // VkPipeline                                     basePipelineHandle
        _basePipelineIndex                                     // int32_t                                        basePipelineIndex
    };
}

VkFormat PipelineStateCreateInfo::computeDescriptorFormat(const RT::ComponentDescriptor& component)
{
    VkFormat format = VK_FORMAT_UNDEFINED;
    switch (component.getFormatType())
    {
        case RT::Type::UInt8:
        {
            if(component.isNormalized())
            {
                const std::array<VkFormat, 4> values{ { VK_FORMAT_R8_UINT, VK_FORMAT_R8G8_UNORM, VK_FORMAT_R8G8B8_UNORM, VK_FORMAT_R8G8B8A8_UNORM } };
                format = values[component.getNbComponents() - 1];
            }
            else
            {
                const std::array<VkFormat, 4> values{ { VK_FORMAT_R8_UINT, VK_FORMAT_R8G8_UINT, VK_FORMAT_R8G8B8_UINT, VK_FORMAT_R8G8B8A8_UINT } };
                format = values[component.getNbComponents() - 1];
            }
        }
        break;
        case RT::Type::Int8:
        {
            if (component.isNormalized())
            {
                const std::array<VkFormat, 4> values{ { VK_FORMAT_R8_SINT, VK_FORMAT_R8G8_SNORM, VK_FORMAT_R8G8B8_SNORM, VK_FORMAT_R8G8B8A8_SNORM } };
                format = values[component.getNbComponents() - 1];
            }
            else
            {
                const std::array<VkFormat, 4> values{ { VK_FORMAT_R8_SINT, VK_FORMAT_R8G8_SINT, VK_FORMAT_R8G8B8_SINT, VK_FORMAT_R8G8B8A8_SINT } };
                format = values[component.getNbComponents() - 1];
            }
        }
        break;

        case RT::Type::Int16:
        {
            if (component.isNormalized())
            {
                const std::array<VkFormat, 4> values{ { VK_FORMAT_R16_SINT, VK_FORMAT_R16G16_SNORM, VK_FORMAT_R16G16B16_SNORM, VK_FORMAT_R16G16B16A16_SNORM } };
                format = values[component.getNbComponents() - 1];
            }
            else
            {
                const std::array<VkFormat, 4> values{ { VK_FORMAT_R16_SINT, VK_FORMAT_R16G16_SINT, VK_FORMAT_R16G16B16_SINT, VK_FORMAT_R16G16B16A16_SINT } };
                format = values[component.getNbComponents() - 1];
            }
        }
        break;
        case RT::Type::UInt16:
        {
            if (component.isNormalized())
            {
                const std::array<VkFormat, 4> values{ { VK_FORMAT_R16_UINT, VK_FORMAT_R16G16_UNORM, VK_FORMAT_R16G16B16_UNORM, VK_FORMAT_R16G16B16A16_UNORM } };
                format = values[component.getNbComponents() - 1];
            }
            else
            {
                const std::array<VkFormat, 4> values{ { VK_FORMAT_R16_UINT, VK_FORMAT_R16G16_UINT, VK_FORMAT_R16G16B16_UINT, VK_FORMAT_R16G16B16A16_UINT } };
                format = values[component.getNbComponents() - 1];
            }
        }
        break;

        case RT::Type::Int32:
        {
            const std::array<VkFormat, 4> values{ { VK_FORMAT_R32_SINT, VK_FORMAT_R32G32_SINT, VK_FORMAT_R32G32B32_SINT, VK_FORMAT_R32G32B32A32_SINT } };
            format = values[component.getNbComponents() - 1];
        }
        break;
        case RT::Type::UInt32:
        {
            const std::array<VkFormat, 4> values{ { VK_FORMAT_R32_UINT, VK_FORMAT_R32G32_UINT, VK_FORMAT_R32G32B32_UINT, VK_FORMAT_R32G32B32A32_UINT } };
            format = values[component.getNbComponents() - 1];
        }
        break;

        case RT::Type::Float:
        {
            const std::array<VkFormat, 4> values{ { VK_FORMAT_R32_SFLOAT, VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT } };
            format = values[component.getNbComponents() - 1];
        }
        break;

        default:
            MouCa::assertion(false); // DEV Issue: None implemented case !
    }
    return format;
}

}