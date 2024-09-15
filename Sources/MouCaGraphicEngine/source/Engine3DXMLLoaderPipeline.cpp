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
    state.failOp      = LoaderHelper::readValue(node, "failOp",      stencilOperations, false, context);
    state.passOp      = LoaderHelper::readValue(node, "passOp",      stencilOperations, false, context);
    state.depthFailOp = LoaderHelper::readValue(node, "depthFailOp", stencilOperations, false, context);
    state.compareOp   = LoaderHelper::readValue(node, "compareOp",   compareOperations, false, context);
    node->getAttribute("compareMask", state.compareMask);
    node->getAttribute("writeMask",   state.writeMask);
    node->getAttribute("reference",   state.reference);
}

void Engine3DXMLLoader::loadPipelineStateCreate(ContextLoading& context, Vulkan::PipelineStateCreateInfo& info, const uint32_t graphicsPipelineId, const uint32_t renderPassId)
{
    uint32_t states = Vulkan::PipelineStateCreateInfo::Uninitialized;
    
    // Stages
    loadPipelineStages(context, info.getStages());

    if(info.getStages().getNbShaders() <= 0)
    {
        throw Core::Exception(makeLoaderError(context, "XMLMissingNodeError") << "GraphicsPipeline" << "Stages/Stage");
    }

    // Input Assembly
    auto inputAssemblies = context._parser.getNode("InputAssembly");
    if (inputAssemblies->getNbElements() > 0)
    {
        MouCa::assertion(inputAssemblies->getNbElements() == 1); //DEV Issue: Need to clean xml !
        auto inputAssembly = inputAssemblies->getNode(0);

        // Build Input Assembly
        bool restart;
        inputAssembly->getAttribute("primitiveRestartEnable", restart);

        auto& state = info.getInputAssembly()._state;
        state.topology = LoaderHelper::readValue(inputAssembly, "topology", primitiveTopologies, false, context);
        state.primitiveRestartEnable = restart ? VK_TRUE : VK_FALSE;
            
        // Activate state
        states |= Vulkan::PipelineStateCreateInfo::InputAssembly;
    }

    // Vertex Input Assembly
    auto vertexInput = context._parser.getNode("VertexInput");
    if (vertexInput->getNbElements() > 0)
    {
        MouCa::assertion(vertexInput->getNbElements() == 1); //DEV Issue: Need to clean xml !
        auto vertexInputNode = vertexInput->getNode(0);

        // cppcheck-suppress unreadVariable // false positive
        auto pushV = context._parser.autoPushNode(*vertexInputNode);

        // Load binding
        auto allBindings = context._parser.getNode("BindingDescription");
        std::vector<VkVertexInputBindingDescription> bindings;
        bindings.reserve(allBindings->getNbElements());
        for (size_t idBinding = 0; idBinding < allBindings->getNbElements(); ++idBinding)
        {
            auto bindingNode = allBindings->getNode(idBinding);

            VkVertexInputBindingDescription bindingDescription;
            bindingNode->getAttribute("binding", bindingDescription.binding);
            bindingDescription.inputRate = LoaderHelper::readValue(bindingNode, "inputRate", vertexInputRates, false, context);

            if (bindingNode->hasAttribute("stride"))
            {
                bindingNode->getAttribute("stride", bindingDescription.stride);
            }
            else
            {
                const uint32_t idM = LoaderHelper::getLinkedIdentifiant(bindingNode, "meshDescriptorId", _cpuMeshDescriptors, context);
                bindingDescription.stride = static_cast<uint32_t>(_cpuMeshDescriptors[idM].getByteSize());
            }

            bindings.emplace_back(bindingDescription);
        }
        info.getVertexInput().setBindingDescriptions(std::move(bindings));
        
        // Load Attributs
        auto allAttributs = context._parser.getNode("AttributeDescription");

        std::vector<VkVertexInputAttributeDescription> attributs;
        attributs.reserve(allAttributs->getNbElements());
        for (size_t idAttribute = 0; idAttribute < allAttributs->getNbElements(); ++idAttribute)
        {
            auto attributNode = allAttributs->getNode(idAttribute);

            VkVertexInputAttributeDescription attributDescription;
            attributNode->getAttribute("binding",  attributDescription.binding);
            attributNode->getAttribute("location", attributDescription.location);
            if (attributNode->hasAttribute("offset"))
            {
                attributNode->getAttribute("offset", attributDescription.offset);
                attributDescription.format = LoaderHelper::readValue(attributNode, "format", formats, false, context);

                attributs.emplace_back(attributDescription);
            }
            else
            {
                // Get descriptor
                const uint32_t idM = LoaderHelper::getLinkedIdentifiant(attributNode, "meshDescriptorId", _cpuMeshDescriptors, context);
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
    auto rasterizations = context._parser.getNode("Rasterization");
    if (rasterizations->getNbElements() > 0)
    {
        MouCa::assertion(rasterizations->getNbElements() == 1); //DEV Issue: Need to clean xml !
        auto rasterization = rasterizations->getNode(0);

        auto& state = info.getRasterizer()._state;
        state.cullMode    = LoaderHelper::readValue(rasterization, "cullMode",    cullModes,    true,  context);
        state.polygonMode = LoaderHelper::readValue(rasterization, "polygonMode", polygonModes, false, context);
        state.frontFace   = LoaderHelper::readValue(rasterization, "frontFace",   frontFaces,   false, context);
        bool boolean;
        rasterization->getAttribute("depthClampEnable",         boolean);
        state.depthClampEnable        = boolean ? VK_TRUE : VK_FALSE;
        rasterization->getAttribute("rasterizerDiscardEnable",  boolean);
        state.rasterizerDiscardEnable = boolean ? VK_TRUE : VK_FALSE;
        rasterization->getAttribute("depthBiasEnable",          boolean);
        state.depthBiasEnable         = boolean ? VK_TRUE : VK_FALSE;
        // Depth Bias
        if(state.depthBiasEnable)
        {
            rasterization->getAttribute("depthBiasClamp",          state.depthBiasClamp);
            rasterization->getAttribute("depthBiasConstantFactor", state.depthBiasConstantFactor);
            rasterization->getAttribute("depthBiasSlopeFactor",    state.depthBiasSlopeFactor);
        }

        // Optional
        if(rasterization->hasAttribute("lineWidth"))
        {
            rasterization->getAttribute("lineWidth", state.lineWidth);
        }

        // Activate state
        states |= Vulkan::PipelineStateCreateInfo::Rasterization;
    }

    // Blend
    auto blends = context._parser.getNode("BlendState");
    if (blends->getNbElements() > 0)
    {
        MouCa::assertion(blends->getNbElements() == 1); //DEV Issue: Need to clean xml !
        auto blend = blends->getNode(0);
        
        auto& state = info.getBlending()._state;

        // Logic Operator
        bool enable;
        blend->getAttribute("logicOpEnable", enable);
        state.logicOpEnable = enable ? VK_TRUE : VK_FALSE;
        state.logicOp       = VK_LOGIC_OP_CLEAR;
        if (state.logicOpEnable)
        {
            state.logicOp = LoaderHelper::readValue(blend, "logicOp", logicOperators, false, context);
        }
        
        // Blend constant
        blend->getAttribute("blendConstantsR", state.blendConstants[0]);
        blend->getAttribute("blendConstantsG", state.blendConstants[1]);
        blend->getAttribute("blendConstantsB", state.blendConstants[2]);
        blend->getAttribute("blendConstantsA", state.blendConstants[3]);

        // All attachments: must be equal to RenderPass/subpass/colorAttachment count (See doc)
        // cppcheck-suppress unreadVariable // false positive
        auto pushB = context._parser.autoPushNode(*blend);
        auto allBlends = context._parser.getNode("BlendAttachment");
        
        const size_t nbColors = _renderPasses[renderPassId].lock()->getSubPasses()[0].colorAttachmentCount;
        if( nbColors != allBlends->getNbElements() )
        {
            throw Core::Exception(makeLoaderError(context, "XMLBlendAttachmentSizeError") << std::to_string(graphicsPipelineId) << std::to_string(renderPassId));
        }

        std::vector<VkPipelineColorBlendAttachmentState> attachments;
        attachments.reserve(allBlends->getNbElements());
        for (size_t idBlend = 0; idBlend < allBlends->getNbElements(); ++idBlend)
        {
            auto blendAttachmentNode = allBlends->getNode(idBlend);
            blendAttachmentNode->getAttribute("blendEnable", enable);
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
                attachmentState.srcColorBlendFactor = LoaderHelper::readValue(blendAttachmentNode, "srcColorBlendFactor", blendFactors, false, context);
                attachmentState.dstColorBlendFactor = LoaderHelper::readValue(blendAttachmentNode, "dstColorBlendFactor", blendFactors, false, context);
                attachmentState.colorBlendOp        = LoaderHelper::readValue(blendAttachmentNode, "colorBlendOp",        blendOperations, false, context);
                attachmentState.srcAlphaBlendFactor = LoaderHelper::readValue(blendAttachmentNode, "srcAlphaBlendFactor", blendFactors, false, context);
                attachmentState.dstAlphaBlendFactor = LoaderHelper::readValue(blendAttachmentNode, "dstAlphaBlendFactor", blendFactors, false, context);
                attachmentState.alphaBlendOp        = LoaderHelper::readValue(blendAttachmentNode, "alphaBlendOp",        blendOperations, false, context);
            }

            attachmentState.colorWriteMask = LoaderHelper::readValue(blendAttachmentNode, "colorWriteMask", colorComponents, true, context);

            attachments.emplace_back(attachmentState);
        }
        
        info.getBlending().setColorBlendAttachments(attachments);

        // Activate state
        states |= Vulkan::PipelineStateCreateInfo::ColorBlend;
    }

    // DepthStencil
    auto depthStencils = context._parser.getNode("DepthStencil");
    if (depthStencils->getNbElements() > 0)
    {
        MouCa::assertion(depthStencils->getNbElements() == 1); //DEV Issue: Need to clean xml !
        auto depthStencilNode = depthStencils->getNode(0);

        auto& state = info.getDepthStencil()._state;
        bool enable;
        depthStencilNode->getAttribute("depthTestEnable", enable);
        state.depthTestEnable  = enable ? VK_TRUE : VK_FALSE;
        depthStencilNode->getAttribute("depthWriteEnable", enable);
        state.depthWriteEnable = enable ? VK_TRUE : VK_FALSE;
        state.depthCompareOp   = LoaderHelper::readValue(depthStencilNode, "depthCompareOp", compareOperations, false, context);

        // cppcheck-suppress unreadVariable // false positive
        auto pushD = context._parser.autoPushNode(*depthStencilNode);

        auto stencilFront = context._parser.getNode("StencilFront");
        auto stencilBack  = context._parser.getNode("StencilBack");
        if (stencilFront->getNbElements() == 0 && stencilBack->getNbElements() == 0)
        {
            state.stencilTestEnable = VK_FALSE;
        }
        else
        {
            if (stencilFront->getNbElements() != 1 || stencilBack->getNbElements() != 1)
            {
                throw Core::Exception(makeLoaderError(context, "XMLMissingStencilFrontBackError") << std::to_string(graphicsPipelineId));
            }

            loadStencilOp(context, stencilFront->getNode(0), state.front);
            loadStencilOp(context, stencilBack->getNode(0),  state.back);
        
            state.stencilTestEnable = VK_TRUE;
        }
        
        auto depthBound  = context._parser.getNode("DepthBounds");
        if (depthBound->getNbElements() > 0)
        {
            MouCa::assertion(depthBound->getNbElements() == 1); //DEV Issue: Need to clean xml !
            auto depthBoundNode = depthBound->getNode(0);

            state.stencilTestEnable = VK_TRUE;

            depthBoundNode->getAttribute("min", state.minDepthBounds);
            depthBoundNode->getAttribute("max", state.maxDepthBounds);
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
    auto allDynamics = context._parser.getNode("DynamicState");
    if (allDynamics->getNbElements() > 0)
    {
        for (size_t idDynamic = 0; idDynamic < allDynamics->getNbElements(); ++idDynamic)
        {
            auto dynamicNode   = allDynamics->getNode(idDynamic);
            
            // Read value
            const auto dynamic = LoaderHelper::readValue(dynamicNode, "state", dynamics, false, context);
            info.getDynamic().addDynamicState(dynamic);

            dynamicViewport |= (dynamic == VK_DYNAMIC_STATE_VIEWPORT);
            dynamicScissor  |= (dynamic == VK_DYNAMIC_STATE_SCISSOR);
        }

        // Activate state
        states |= Vulkan::PipelineStateCreateInfo::Dynamic;
    }

    // Viewport
    auto allViewports = context._parser.getNode("Viewport");
    if (allViewports->getNbElements() > 0)
    {
        MOUCA_TODO("Find best way to make Dynamic<->Viewport/Scissor and manage all cases")
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

                viewportNode->getAttribute("x", viewport.x);
                viewportNode->getAttribute("y", viewport.y);
                if (viewportNode->hasAttribute("surfaceId"))
                {
                    const uint32_t surfaceId = LoaderHelper::getLinkedIdentifiant(viewportNode, "surfaceId", _surfaces, context);
                    const VkExtent2D& resolution = _surfaces[surfaceId].lock()->getFormat().getConfiguration()._extent;
                    viewport.width  = static_cast<float>(resolution.width);
                    viewport.height = static_cast<float>(resolution.height);
                    scissor.extent  = resolution;
                }
                else if (viewportNode->hasAttribute("imageId"))
                {
                    const uint32_t imageId = LoaderHelper::getLinkedIdentifiant(viewportNode, "imageId", _images, context);
                    const VkExtent3D& imageRes = _images[imageId].lock()->getExtent();
                    const VkExtent2D resolution = { imageRes.width, imageRes.height };
                    viewport.width  = static_cast<float>(resolution.width);
                    viewport.height = static_cast<float>(resolution.height);
                    scissor.extent = resolution;
                }
                else
                {
                    viewportNode->getAttribute("width",  viewport.width);
                    viewportNode->getAttribute("height", viewport.height);
                }
                
                viewportNode->getAttribute("minDepth", viewport.minDepth);
                viewportNode->getAttribute("maxDepth", viewport.maxDepth);

                viewportNode->getAttribute("scissorX",      scissor.offset.x);
                viewportNode->getAttribute("scissorY",      scissor.offset.y);
                if (!viewportNode->hasAttribute("surfaceId") && !viewportNode->hasAttribute("imageId"))
                {
                    viewportNode->getAttribute("scissorWidth", scissor.extent.width);
                    viewportNode->getAttribute("scissorHeight", scissor.extent.height);
                }

                info.getViewport().addViewport(viewport, scissor);
            }
        }
        
        // Activate state
        states |= Vulkan::PipelineStateCreateInfo::Viewport;
    }

    // Multisample
    auto multisample = context._parser.getNode("Multisample");
    if (multisample->getNbElements() > 0)
    {
        MouCa::assertion(multisample->getNbElements() == 1); //DEV Issue: Need to clean xml !
        auto multisampleNode = multisample->getNode(0);

        auto& state = info.getMultisample()._state;

        state.rasterizationSamples = LoaderHelper::readValue(multisampleNode, "rasterizationSamples", samples, false, context);
        
        if (multisampleNode->hasAttribute("minSampleShading"))
        {
            state.sampleShadingEnable = VK_TRUE;
            multisampleNode->getAttribute("minSampleShading", state.minSampleShading);
        }
        else
        {
            state.sampleShadingEnable = VK_FALSE;
        }

        state.pSampleMask = nullptr;
        bool enable;
        multisampleNode->getAttribute("alphaToCoverageEnable", enable);
        state.alphaToOneEnable = enable ? VK_TRUE : VK_FALSE;
        multisampleNode->getAttribute("alphaToOneEnable", enable);
        state.alphaToOneEnable = enable ? VK_TRUE : VK_FALSE;

        // Activate state
        states |= Vulkan::PipelineStateCreateInfo::Multisample;
    }

    // Tessellation
    auto tessellation = context._parser.getNode("Tessellation");
    if(tessellation->getNbElements() > 0)
    {
        MouCa::assertion(tessellation->getNbElements() == 1); //DEV Issue: Need to clean xml !
        auto tessellationNode = tessellation->getNode(0);
        
        // Read data
        auto& state = info.getTessellation()._state;
        tessellationNode->getAttribute("patchControlPoints", state.patchControlPoints);

        // Activate state
        states |= Vulkan::PipelineStateCreateInfo::Tessellation;
    }

    // Finish creation
    info.initialize(static_cast<Vulkan::PipelineStateCreateInfo::States>(states));
}

void Engine3DXMLLoader::loadPipelineLayouts(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak)
{
    // Search Pipeline Layouts
    auto result = context._parser.getNode("PipelineLayouts");
    if (result->getNbElements() > 0)
    {
        MouCa::assertion(result->getNbElements() == 1); //DEV Issue: please clean xml ?
        auto device = deviceWeak.lock();

        // cppcheck-suppress unreadVariable // false positive
        auto aPush = context._parser.autoPushNode(*result->getNode(0));

        // Parsing all PipelineLayouts
        auto allPipelineLayouts = context._parser.getNode("PipelineLayout");
        for (size_t idPipelineLayout = 0; idPipelineLayout < allPipelineLayouts->getNbElements(); ++idPipelineLayout)
        {
            auto pipelineLayoutNode = allPipelineLayouts->getNode(idPipelineLayout);

            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(pipelineLayoutNode, "PipelineLayout", _pipelineLayouts, context, existing);

            // Crate PipelineLayout
            auto pipelineLayout = std::make_shared<Vulkan::PipelineLayout>();

            auto aPushP = context._parser.autoPushNode(*pipelineLayoutNode);

            // Read DescriptorSetLayout
            auto allDescriptorSetLayouts = context._parser.getNode("DescriptorSetLayout");
            std::vector<VkDescriptorSetLayout> setLayouts;
            setLayouts.reserve(allDescriptorSetLayouts->getNbElements());
            for (size_t idDescriptorSetLayout = 0; idDescriptorSetLayout < allDescriptorSetLayouts->getNbElements(); ++idDescriptorSetLayout)
            {
                auto DSLNode = allDescriptorSetLayouts->getNode(idDescriptorSetLayout);
                const uint32_t idDSL = LoaderHelper::getLinkedIdentifiant(DSLNode, "descriptorSetId", _descriptorSetLayouts, context);
                // Get Vulkan ID
                setLayouts.emplace_back(_descriptorSetLayouts[idDSL].lock()->getInstance());
            }

            // Read PushConstantRange
            auto allPushConstantRanges = context._parser.getNode("PushConstantRange");
            for (size_t idPushConstant = 0; idPushConstant < allPushConstantRanges->getNbElements(); ++idPushConstant)
            {
                auto pushConstantNode = allPushConstantRanges->getNode(idPushConstant);

                VkPushConstantRange pushConstant;
                pushConstant.stageFlags = LoaderHelper::readValue(pushConstantNode, "shaderStageFlags", shaderStages, true, context);
                pushConstantNode->getAttribute("offset", pushConstant.offset);
                pushConstantNode->getAttribute("size",   pushConstant.size);
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
    auto stages = context._parser.getNode("Stages");
    if (stages->getNbElements() > 0)
    {
        MouCa::assertion(stages->getNbElements() == 1); //DEV Issue: Need to clean xml !

        // cppcheck-suppress unreadVariable // false positive
        auto aPush = context._parser.autoPushNode(*stages->getNode(0));

        // Parsing all Graphics pipeline
        auto allStages = context._parser.getNode("Stage");
        for (size_t idStage = 0; idStage < allStages->getNbElements(); ++idStage)
        {
            auto stageNode = allStages->getNode(idStage);

            const uint32_t idS = LoaderHelper::getLinkedIdentifiant(stageNode, "shaderModuleId", _shaderModules, context);

            auto aPushShader = context._parser.autoPushNode(*stageNode);

            Vulkan::ShaderSpecialization specialization;
            auto specializations = context._parser.getNode("Specialization");
            if (specializations->getNbElements() > 0)
            {
                auto specializationNode = specializations->getNode(0);
                // Read buffer
                const uint32_t idB = LoaderHelper::getLinkedIdentifiant(specializationNode, "external", _cpuBuffers, context);
                const auto& buffer = _cpuBuffers[idB].lock();
                MouCa::assertion(buffer != nullptr && !buffer->isNull());
                specialization.addDataInfo(buffer->getData(), buffer->getByteSize());

                auto aPushS = context._parser.autoPushNode(*specializationNode);

                // Add map entry
                auto allEntries = context._parser.getNode("Entry");

                std::vector<VkSpecializationMapEntry> entries;
                entries.resize(allEntries->getNbElements());
                auto itEntry = entries.begin();
                for (size_t idEntry = 0; idEntry < allEntries->getNbElements(); ++idEntry)
                {
                    auto entryNode = allEntries->getNode(idEntry);

                    entryNode->getAttribute("constantID", itEntry->constantID);
                    entryNode->getAttribute("offset", itEntry->offset);
                    entryNode->getAttribute("size", itEntry->size);
                    ++itEntry;
                }

                specialization.setMapEntries(std::move(entries));
            }

            stageShader.addShaderModule(_shaderModules[idS], std::move(specialization));
        }
    }
}

}