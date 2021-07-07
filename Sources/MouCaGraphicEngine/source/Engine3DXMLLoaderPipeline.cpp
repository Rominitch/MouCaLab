#include "Dependencies.h"

#include "MouCaGraphicEngine/include/Engine3DXMLLoader.h"
#include "MouCaGraphicEngine/include/Engine3DXMLHelper.h"

#include <LibVulkan/include/VKCommand.h>
#include <LibVulkan/include/VKContextDevice.h>
#include <LibVulkan/include/VKDescriptorSet.h>
#include <LibVulkan/include/VKImage.h>
#include <LibVulkan/include/VKPipelineLayout.h>
#include <LibVulkan/include/VKPipelineStates.h>
#include <LibVulkan/include/VKRenderPass.h>

#include "MouCaGraphicEngine/include/VulkanEnum.h"

namespace MouCaGraphic
{

void loadStencilOp(Engine3DXMLLoader::ContextLoading& context, XML::NodeUPtr node, VkStencilOpState& state)
{
    state.failOp      = LoaderHelper::readValue(node, u8"failOp",      stencilOperations, false, context);
    state.passOp      = LoaderHelper::readValue(node, u8"passOp",      stencilOperations, false, context);
    state.depthFailOp = LoaderHelper::readValue(node, u8"depthFailOp", stencilOperations, false, context);
    state.compareOp   = LoaderHelper::readValue(node, u8"compareOp",   compareOperations, false, context);
    node->getAttribute(u8"compareMask", state.compareMask);
    node->getAttribute(u8"writeMask",   state.writeMask);
    node->getAttribute(u8"reference",   state.reference);
}

void Engine3DXMLLoader::loadPipelineStateCreate(ContextLoading& context, Vulkan::PipelineStateCreateInfo& info, const uint32_t graphicsPipelineId, const uint32_t renderPassId)
{
    uint32_t states = Vulkan::PipelineStateCreateInfo::Uninitialized;
    
    // Stages
    loadPipelineStages(context, info.getStages());

    if(info.getStages().getNbShaders() <= 0)
    {
        MOUCA_THROW_ERROR_3(u8"Engine3D", u8"XMLMissingNodeError", context.getFileName(), u8"GraphicsPipeline" , u8"Stages/Stage");
    }

    // Input Assembly
    auto inputAssemblies = context._parser.getNode(u8"InputAssembly");
    if (inputAssemblies->getNbElements() > 0)
    {
        MOUCA_ASSERT(inputAssemblies->getNbElements() == 1); //DEV Issue: Need to clean xml !
        auto inputAssembly = inputAssemblies->getNode(0);

        // Build Input Assembly
        bool restart;
        inputAssembly->getAttribute(u8"primitiveRestartEnable", restart);

        auto& state = info.getInputAssembly()._state;
        state.topology = LoaderHelper::readValue(inputAssembly, u8"topology", primitiveTopologies, false, context);
        state.primitiveRestartEnable = restart ? VK_TRUE : VK_FALSE;
            
        // Activate state
        states |= Vulkan::PipelineStateCreateInfo::InputAssembly;
    }

    // Vertex Input Assembly
    auto vertexInput = context._parser.getNode(u8"VertexInput");
    if (vertexInput->getNbElements() > 0)
    {
        MOUCA_ASSERT(vertexInput->getNbElements() == 1); //DEV Issue: Need to clean xml !
        auto vertexInputNode = vertexInput->getNode(0);

        // cppcheck-suppress unreadVariable // false positive
        auto pushV = context._parser.autoPushNode(*vertexInputNode);

        // Load binding
        auto allBindings = context._parser.getNode(u8"BindingDescription");
        std::vector<VkVertexInputBindingDescription> bindings;
        bindings.reserve(allBindings->getNbElements());
        for (size_t idBinding = 0; idBinding < allBindings->getNbElements(); ++idBinding)
        {
            auto bindingNode = allBindings->getNode(idBinding);

            VkVertexInputBindingDescription bindingDescription;
            bindingNode->getAttribute(u8"binding", bindingDescription.binding);
            bindingDescription.inputRate = LoaderHelper::readValue(bindingNode, u8"inputRate", vertexInputRates, false, context);

            if (bindingNode->hasAttribute(u8"stride"))
            {
                bindingNode->getAttribute(u8"stride", bindingDescription.stride);
            }
            else
            {
                const uint32_t idM = LoaderHelper::getLinkedIdentifiant(bindingNode, u8"meshDescriptorId", _cpuMeshDescriptors, context);
                bindingDescription.stride = static_cast<uint32_t>(_cpuMeshDescriptors[idM].getByteSize());
            }

            bindings.emplace_back(bindingDescription);
        }
        info.getVertexInput().setBindingDescriptions(std::move(bindings));
        
        // Load Attributs
        auto allAttributs = context._parser.getNode(u8"AttributeDescription");

        std::vector<VkVertexInputAttributeDescription> attributs;
        attributs.reserve(allAttributs->getNbElements());
        for (size_t idAttribute = 0; idAttribute < allAttributs->getNbElements(); ++idAttribute)
        {
            auto attributNode = allAttributs->getNode(idAttribute);

            VkVertexInputAttributeDescription attributDescription;
            attributNode->getAttribute(u8"binding",  attributDescription.binding);
            attributNode->getAttribute(u8"location", attributDescription.location);
            if (attributNode->hasAttribute(u8"offset"))
            {
                attributNode->getAttribute(u8"offset", attributDescription.offset);
                attributDescription.format = LoaderHelper::readValue(attributNode, u8"format", formats, false, context);

                attributs.emplace_back(attributDescription);
            }
            else
            {
                // Get descriptor
                const uint32_t idM = LoaderHelper::getLinkedIdentifiant(attributNode, u8"meshDescriptorId", _cpuMeshDescriptors, context);
                const auto& descriptors = _cpuMeshDescriptors[idM].getDescriptors();

                // Reallocate more
                attributs.reserve(attributs.size() + descriptors.size());

                attributDescription.offset = 0;

                for (const auto& descriptor : descriptors)
                {
                    attributDescription.format = Vulkan::PipelineStateCreateInfo::computeDescriptorFormat(descriptor);
                    
                    // Add new descriptor
                    attributs.emplace_back(attributDescription);

                    // Next descriptor
                    attributDescription.offset += static_cast<uint32_t>(descriptor.getSizeInByte());
                    attributDescription.location += 1;
                }
            }
        }
        info.getVertexInput().setAttributeDescriptions(std::move(attributs));

        // Activate state
        states |= Vulkan::PipelineStateCreateInfo::VertexInput;
    }

    // Rasterization
    auto rasterizations = context._parser.getNode(u8"Rasterization");
    if (rasterizations->getNbElements() > 0)
    {
        MOUCA_ASSERT(rasterizations->getNbElements() == 1); //DEV Issue: Need to clean xml !
        auto rasterization = rasterizations->getNode(0);

        auto& state = info.getRasterizer()._state;
        state.cullMode    = LoaderHelper::readValue(rasterization, u8"cullMode",    cullModes,    true,  context);
        state.polygonMode = LoaderHelper::readValue(rasterization, u8"polygonMode", polygonModes, false, context);
        state.frontFace   = LoaderHelper::readValue(rasterization, u8"frontFace",   frontFaces,   false, context);
        bool boolean;
        rasterization->getAttribute(u8"depthClampEnable",         boolean);
        state.depthClampEnable        = boolean ? VK_TRUE : VK_FALSE;
        rasterization->getAttribute(u8"rasterizerDiscardEnable",  boolean);
        state.rasterizerDiscardEnable = boolean ? VK_TRUE : VK_FALSE;
        rasterization->getAttribute(u8"depthBiasEnable",          boolean);
        state.depthBiasEnable         = boolean ? VK_TRUE : VK_FALSE;
        // Depth Bias
        if(state.depthBiasEnable)
        {
            rasterization->getAttribute(u8"depthBiasClamp",          state.depthBiasClamp);
            rasterization->getAttribute(u8"depthBiasConstantFactor", state.depthBiasConstantFactor);
            rasterization->getAttribute(u8"depthBiasSlopeFactor",    state.depthBiasSlopeFactor);
        }

        // Optional
        if(rasterization->hasAttribute(u8"lineWidth"))
        {
            rasterization->getAttribute(u8"lineWidth", state.lineWidth);
        }

        // Activate state
        states |= Vulkan::PipelineStateCreateInfo::Rasterization;
    }

    // Blend
    auto blends = context._parser.getNode(u8"BlendState");
    if (blends->getNbElements() > 0)
    {
        MOUCA_ASSERT(blends->getNbElements() == 1); //DEV Issue: Need to clean xml !
        auto blend = blends->getNode(0);
        
        auto& state = info.getBlending()._state;

        // Logic Operator
        bool enable;
        blend->getAttribute(u8"logicOpEnable", enable);
        state.logicOpEnable = enable ? VK_TRUE : VK_FALSE;
        state.logicOp       = VK_LOGIC_OP_CLEAR;
        if (state.logicOpEnable)
        {
            state.logicOp = LoaderHelper::readValue(blend, u8"logicOp", logicOperators, false, context);
        }
        
        // Blend constant
        blend->getAttribute(u8"blendConstantsR", state.blendConstants[0]);
        blend->getAttribute(u8"blendConstantsG", state.blendConstants[1]);
        blend->getAttribute(u8"blendConstantsB", state.blendConstants[2]);
        blend->getAttribute(u8"blendConstantsA", state.blendConstants[3]);

        // All attachments: must be equal to RenderPass/subpass/colorAttachment count (See doc)
        // cppcheck-suppress unreadVariable // false positive
        auto pushB = context._parser.autoPushNode(*blend);
        auto allBlends = context._parser.getNode(u8"BlendAttachment");
        
        const size_t nbColors = _renderPasses[renderPassId].lock()->getSubPasses()[0].colorAttachmentCount;
        if( nbColors != allBlends->getNbElements() )
        {
            MOUCA_THROW_ERROR_3(u8"Engine3D", u8"XMLBlendAttachmentSizeError", context.getFileName(), std::to_string(graphicsPipelineId), std::to_string(renderPassId));
        }

        std::vector<VkPipelineColorBlendAttachmentState> attachments;
        attachments.reserve(allBlends->getNbElements());
        for (size_t idBlend = 0; idBlend < allBlends->getNbElements(); ++idBlend)
        {
            auto blendAttachmentNode = allBlends->getNode(idBlend);
            blendAttachmentNode->getAttribute(u8"blendEnable", enable);
            const VkColorComponentFlags flags = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            VkPipelineColorBlendAttachmentState attachmentState
            {
                static_cast<VkBool32>(enable ? VK_TRUE : VK_FALSE), // VkBool32                 blendEnable
                VK_BLEND_FACTOR_ZERO,                               // VkBlendFactor            srcColorBlendFactor
                VK_BLEND_FACTOR_ZERO,                               // VkBlendFactor            dstColorBlendFactor
                VK_BLEND_OP_ADD,                                    // VkBlendOp                colorBlendOp
                VK_BLEND_FACTOR_ZERO,                               // VkBlendFactor            srcAlphaBlendFactor
                VK_BLEND_FACTOR_ZERO,                               // VkBlendFactor            dstAlphaBlendFactor
                VK_BLEND_OP_ADD,                                    // VkBlendOp                alphaBlendOp
                flags                                               // VkColorComponentFlags    colorWriteMask
            };
            if(attachmentState.blendEnable)
            {
                attachmentState.srcColorBlendFactor = LoaderHelper::readValue(blendAttachmentNode, u8"srcColorBlendFactor", blendFactors, false, context);
                attachmentState.dstColorBlendFactor = LoaderHelper::readValue(blendAttachmentNode, u8"dstColorBlendFactor", blendFactors, false, context);
                attachmentState.colorBlendOp        = LoaderHelper::readValue(blendAttachmentNode, u8"colorBlendOp",        blendOperations, false, context);
                attachmentState.srcAlphaBlendFactor = LoaderHelper::readValue(blendAttachmentNode, u8"srcAlphaBlendFactor", blendFactors, false, context);
                attachmentState.dstAlphaBlendFactor = LoaderHelper::readValue(blendAttachmentNode, u8"dstAlphaBlendFactor", blendFactors, false, context);
                attachmentState.alphaBlendOp        = LoaderHelper::readValue(blendAttachmentNode, u8"alphaBlendOp",        blendOperations, false, context);
            }

            attachmentState.colorWriteMask = LoaderHelper::readValue(blendAttachmentNode, u8"colorWriteMask", colorComponents, true, context);

            attachments.emplace_back(attachmentState);
        }
        
        info.getBlending().setColorBlendAttachments(attachments);

        // Activate state
        states |= Vulkan::PipelineStateCreateInfo::ColorBlend;
    }

    // DepthStencil
    auto depthStencils = context._parser.getNode(u8"DepthStencil");
    if (depthStencils->getNbElements() > 0)
    {
        MOUCA_ASSERT(depthStencils->getNbElements() == 1); //DEV Issue: Need to clean xml !
        auto depthStencilNode = depthStencils->getNode(0);

        auto& state = info.getDepthStencil()._state;
        bool enable;
        depthStencilNode->getAttribute(u8"depthTestEnable", enable);
        state.depthTestEnable  = enable ? VK_TRUE : VK_FALSE;
        depthStencilNode->getAttribute(u8"depthWriteEnable", enable);
        state.depthWriteEnable = enable ? VK_TRUE : VK_FALSE;
        state.depthCompareOp   = LoaderHelper::readValue(depthStencilNode, u8"depthCompareOp", compareOperations, false, context);

        // cppcheck-suppress unreadVariable // false positive
        auto pushD = context._parser.autoPushNode(*depthStencilNode);

        auto stencilFront = context._parser.getNode(u8"StencilFront");
        auto stencilBack  = context._parser.getNode(u8"StencilBack");
        if (stencilFront->getNbElements() == 0 && stencilBack->getNbElements() == 0)
        {
            state.stencilTestEnable = VK_FALSE;
        }
        else
        {
            if (stencilFront->getNbElements() != 1 || stencilBack->getNbElements() != 1)
            {
                MOUCA_THROW_ERROR_2(u8"Engine3D", u8"XMLMissingStencilFrontBackError", context.getFileName(), std::to_string(graphicsPipelineId));
            }

            loadStencilOp(context, stencilFront->getNode(0), state.front);
            loadStencilOp(context, stencilBack->getNode(0),  state.back);
        
            state.stencilTestEnable = VK_TRUE;
        }
        
        auto depthBound  = context._parser.getNode(u8"DepthBounds");
        if (depthBound->getNbElements() > 0)
        {
            MOUCA_ASSERT(depthBound->getNbElements() == 1); //DEV Issue: Need to clean xml !
            auto depthBoundNode = depthBound->getNode(0);

            state.stencilTestEnable = VK_TRUE;

            depthBoundNode->getAttribute(u8"min", state.minDepthBounds);
            depthBoundNode->getAttribute(u8"max", state.maxDepthBounds);
        }
        else
        {
            state.stencilTestEnable = VK_FALSE;
        }

        // Activate state
        states |= Vulkan::PipelineStateCreateInfo::DepthStencil;
    }

    // Dynamic
    bool dynamicViewport = false;
    bool dynamicScissor  = false;
    auto allDynamics = context._parser.getNode(u8"DynamicState");
    if (allDynamics->getNbElements() > 0)
    {
        for (size_t idDynamic = 0; idDynamic < allDynamics->getNbElements(); ++idDynamic)
        {
            auto dynamicNode   = allDynamics->getNode(idDynamic);
            
            // Read value
            const auto dynamic = LoaderHelper::readValue(dynamicNode, u8"state", dynamics, false, context);
            info.getDynamic().addDynamicState(dynamic);

            dynamicViewport |= (dynamic == VK_DYNAMIC_STATE_VIEWPORT);
            dynamicScissor  |= (dynamic == VK_DYNAMIC_STATE_SCISSOR);
        }

        // Activate state
        states |= Vulkan::PipelineStateCreateInfo::Dynamic;
    }

    // Viewport
    auto allViewports = context._parser.getNode(u8"Viewport");
    if (allViewports->getNbElements() > 0)
    {
        MOUCA_TODO("Find best way to make Dynamic<->Viewport/Scissor and manage all cases");
        if (dynamicViewport && dynamicScissor)
        {
            info.getViewport().addDynamicViewport();
        }
        else
        {
            for (size_t idViewport = 0; idViewport < allViewports->getNbElements(); ++idViewport)
            {
                auto viewportNode = allViewports->getNode(idViewport);

                VkViewport viewport{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
                VkRect2D   scissor { {0, 0}, {0, 0} };

                viewportNode->getAttribute(u8"x", viewport.x);
                viewportNode->getAttribute(u8"y", viewport.y);
                if (viewportNode->hasAttribute(u8"surfaceId"))
                {
                    const uint32_t surfaceId = LoaderHelper::getLinkedIdentifiant(viewportNode, u8"surfaceId", _surfaces, context);
                    const VkExtent2D& resolution = _surfaces[surfaceId].lock()->getFormat().getConfiguration()._extent;
                    viewport.width  = static_cast<float>(resolution.width);
                    viewport.height = static_cast<float>(resolution.height);
                    scissor.extent  = resolution;
                }
                else if (viewportNode->hasAttribute(u8"imageId"))
                {
                    const uint32_t imageId = LoaderHelper::getLinkedIdentifiant(viewportNode, u8"imageId", _images, context);
                    const VkExtent3D& imageRes = _images[imageId].lock()->getExtent();
                    const VkExtent2D resolution = { imageRes.width, imageRes.height };
                    viewport.width  = static_cast<float>(resolution.width);
                    viewport.height = static_cast<float>(resolution.height);
                    scissor.extent = resolution;
                }
                else
                {
                    viewportNode->getAttribute(u8"width",  viewport.width);
                    viewportNode->getAttribute(u8"height", viewport.height);
                }
                
                viewportNode->getAttribute(u8"minDepth", viewport.minDepth);
                viewportNode->getAttribute(u8"maxDepth", viewport.maxDepth);

                viewportNode->getAttribute(u8"scissorX",      scissor.offset.x);
                viewportNode->getAttribute(u8"scissorY",      scissor.offset.y);
                if (!viewportNode->hasAttribute(u8"surfaceId") && !viewportNode->hasAttribute(u8"imageId"))
                {
                    viewportNode->getAttribute(u8"scissorWidth", scissor.extent.width);
                    viewportNode->getAttribute(u8"scissorHeight", scissor.extent.height);
                }

                info.getViewport().addViewport(viewport, scissor);
            }
        }
        
        // Activate state
        states |= Vulkan::PipelineStateCreateInfo::Viewport;
    }

    // Multisample
    auto multisample = context._parser.getNode(u8"Multisample");
    if (multisample->getNbElements() > 0)
    {
        MOUCA_ASSERT(multisample->getNbElements() == 1); //DEV Issue: Need to clean xml !
        auto multisampleNode = multisample->getNode(0);

        auto& state = info.getMultisample()._state;

        state.rasterizationSamples = LoaderHelper::readValue(multisampleNode, u8"rasterizationSamples", samples, false, context);
        
        if (multisampleNode->hasAttribute(u8"minSampleShading"))
        {
            state.sampleShadingEnable = VK_TRUE;
            multisampleNode->getAttribute(u8"minSampleShading", state.minSampleShading);
        }
        else
        {
            state.sampleShadingEnable = VK_FALSE;
        }

        state.pSampleMask = nullptr;
        bool enable;
        multisampleNode->getAttribute(u8"alphaToCoverageEnable", enable);
        state.alphaToOneEnable = enable ? VK_TRUE : VK_FALSE;
        multisampleNode->getAttribute(u8"alphaToOneEnable", enable);
        state.alphaToOneEnable = enable ? VK_TRUE : VK_FALSE;

        // Activate state
        states |= Vulkan::PipelineStateCreateInfo::Multisample;
    }

    // Tessellation
    auto tessellation = context._parser.getNode(u8"Tessellation");
    if(tessellation->getNbElements() > 0)
    {
        MOUCA_ASSERT(tessellation->getNbElements() == 1); //DEV Issue: Need to clean xml !
        auto tessellationNode = tessellation->getNode(0);
        
        // Read data
        auto& state = info.getTessellation()._state;
        tessellationNode->getAttribute(u8"patchControlPoints", state.patchControlPoints);

        // Activate state
        states |= Vulkan::PipelineStateCreateInfo::Tessellation;
    }

    // Finish creation
    info.initialize(static_cast<Vulkan::PipelineStateCreateInfo::States>(states));
}

void Engine3DXMLLoader::loadPipelineLayouts(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak)
{
    // Search Pipeline Layouts
    auto result = context._parser.getNode(u8"PipelineLayouts");
    if (result->getNbElements() > 0)
    {
        MOUCA_ASSERT(result->getNbElements() == 1); //DEV Issue: please clean xml ?
        auto device = deviceWeak.lock();

        // cppcheck-suppress unreadVariable // false positive
        auto aPush = context._parser.autoPushNode(*result->getNode(0));

        // Parsing all PipelineLayouts
        auto allPipelineLayouts = context._parser.getNode(u8"PipelineLayout");
        for (size_t idPipelineLayout = 0; idPipelineLayout < allPipelineLayouts->getNbElements(); ++idPipelineLayout)
        {
            auto pipelineLayoutNode = allPipelineLayouts->getNode(idPipelineLayout);

            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(pipelineLayoutNode, u8"PipelineLayout", _pipelineLayouts, context, existing);

            // Crate PipelineLayout
            auto pipelineLayout = std::make_shared<Vulkan::PipelineLayout>();

            auto aPushP = context._parser.autoPushNode(*pipelineLayoutNode);

            // Read DescriptorSetLayout
            auto allDescriptorSetLayouts = context._parser.getNode(u8"DescriptorSetLayout");
            std::vector<VkDescriptorSetLayout> setLayouts;
            setLayouts.reserve(allDescriptorSetLayouts->getNbElements());
            for (size_t idDescriptorSetLayout = 0; idDescriptorSetLayout < allDescriptorSetLayouts->getNbElements(); ++idDescriptorSetLayout)
            {
                auto DSLNode = allDescriptorSetLayouts->getNode(idDescriptorSetLayout);
                const uint32_t idDSL = LoaderHelper::getLinkedIdentifiant(DSLNode, u8"descriptorSetId", _descriptorSetLayouts, context);
                // Get Vulkan ID
                setLayouts.emplace_back(_descriptorSetLayouts[idDSL].lock()->getInstance());
            }

            // Read PushConstantRange
            auto allPushConstantRanges = context._parser.getNode(u8"PushConstantRange");
            for (size_t idPushConstant = 0; idPushConstant < allPushConstantRanges->getNbElements(); ++idPushConstant)
            {
                auto pushConstantNode = allPushConstantRanges->getNode(idPushConstant);

                VkPushConstantRange pushConstant;
                pushConstant.stageFlags = LoaderHelper::readValue(pushConstantNode, u8"shaderStageFlags", shaderStages, true, context);
                pushConstantNode->getAttribute(u8"offset", pushConstant.offset);
                pushConstantNode->getAttribute(u8"size",   pushConstant.size);
                // Register
                pipelineLayout->addPushConstant(pushConstant);
            }
            
            // Build PipelineLayout
            pipelineLayout->initialize(device->getDevice(), setLayouts);

            // Register
            device->insertPipelineLayout(pipelineLayout);
            _pipelineLayouts[id] = pipelineLayout;
        }
    }
}

void Engine3DXMLLoader::loadPipelineStages(ContextLoading& context, Vulkan::PipelineStageShaders& stageShader)
{
    auto stages = context._parser.getNode(u8"Stages");
    if (stages->getNbElements() > 0)
    {
        MOUCA_ASSERT(stages->getNbElements() == 1); //DEV Issue: Need to clean xml !

        // cppcheck-suppress unreadVariable // false positive
        auto aPush = context._parser.autoPushNode(*stages->getNode(0));

        // Parsing all Graphics pipeline
        auto allStages = context._parser.getNode(u8"Stage");
        for (size_t idStage = 0; idStage < allStages->getNbElements(); ++idStage)
        {
            auto stageNode = allStages->getNode(idStage);

            const uint32_t idS = LoaderHelper::getLinkedIdentifiant(stageNode, u8"shaderModuleId", _shaderModules, context);

            auto aPushShader = context._parser.autoPushNode(*stageNode);

            Vulkan::ShaderSpecialization specialization;
            auto specializations = context._parser.getNode(u8"Specialization");
            if (specializations->getNbElements() > 0)
            {
                auto specializationNode = specializations->getNode(0);
                // Read buffer
                const uint32_t idB = LoaderHelper::getLinkedIdentifiant(specializationNode, u8"external", _cpuBuffers, context);
                const auto& buffer = _cpuBuffers[idB].lock();
                MOUCA_ASSERT(buffer != nullptr && !buffer->isNull());
                specialization.addDataInfo(buffer->getData(), buffer->getByteSize());

                auto aPushS = context._parser.autoPushNode(*specializationNode);

                // Add map entry
                auto allEntries = context._parser.getNode(u8"Entry");

                std::vector<VkSpecializationMapEntry> entries;
                entries.resize(allEntries->getNbElements());
                auto itEntry = entries.begin();
                for (size_t idEntry = 0; idEntry < allEntries->getNbElements(); ++idEntry)
                {
                    auto entryNode = allEntries->getNode(idEntry);

                    entryNode->getAttribute(u8"constantID", itEntry->constantID);
                    entryNode->getAttribute(u8"offset", itEntry->offset);
                    entryNode->getAttribute(u8"size", itEntry->size);
                    ++itEntry;
                }

                specialization.setMapEntries(std::move(entries));
            }

            stageShader.addShaderModule(_shaderModules[idS], std::move(specialization));
        }
    }
}

}