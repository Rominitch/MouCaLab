#include "Dependencies.h"

#include "MouCaGraphicEngine/include/Engine3DXMLLoader.h"
#include "MouCaGraphicEngine/include/Engine3DXMLHelper.h"

#include <LibVulkan/include/VKContextDevice.h>
#include <LibVulkan/include/VKCommand.h>
#include <LibVulkan/include/VKCommandBufferSurface.h>
#include <LibVulkan/include/VKDescriptorSet.h>
#include <LibVulkan/include/VKFrameBuffer.h>
#include <LibVulkan/include/VKGraphicsPipeline.h>
#include <LibVulkan/include/VKRayTracingPipeline.h>
#include <LibVulkan/include/VKSwapChain.h>

#include "MouCaGraphicEngine/include/VulkanEnum.h"

namespace MouCaGraphic
{

void Engine3DXMLLoader::loadCommandPools(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak)
{
    MouCa::preCondition(!deviceWeak.expired()); //DEV Issue: Bad device !

    // Search CommandBuffer
    auto result = context._parser.getNode("CommandPools");
    if (result->getNbElements() > 0)
    {
        MouCa::assertion(result->getNbElements() == 1); //DEV Issue: please clean xml ?
        auto device = deviceWeak.lock();

        auto aPush = context._parser.autoPushNode(*result->getNode(0));
        // Parsing all images
        auto allCommandPools = context._parser.getNode("CommandPool");
        for (size_t idPool = 0; idPool < allCommandPools->getNbElements(); ++idPool)
        {
            auto poolNode = allCommandPools->getNode(idPool);

            // Mandatory attribute
            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(poolNode, "CommandPool", _commandPools, context, existing);
            if (existing)
            {
                continue;
            }

            int idQueue = 0;
            if (poolNode->hasAttribute("families"))
            {
                Core::String families;
                poolNode->getAttribute("families", families);
                if (families == "graphic")
                {
                    idQueue = device->getDevice().getQueueFamilyGraphicId();
                }   
                else if (families == "compute")
                {
                    idQueue = device->getDevice().getQueueFamilyComputeId();
                }
                else if (families == "transfer")
                {
                    idQueue = device->getDevice().getQueueFamilyTransferId();
                }
                else
                {
                    throw Core::Exception(Core::ErrorData("Engine3D", "UnknownFamiliesError") << context.getFileName() << families);
                }   
            }
            else if (poolNode->hasAttribute("queueId"))
            {
                poolNode->getAttribute("queueId", idQueue);
            }

            VkCommandPoolCreateFlags flags = 0;
            if (poolNode->hasAttribute("flags"))
            {
                flags = LoaderHelper::readValue(poolNode, "flags", poolCreateFlags, true, context);
            }

            // Build
            auto pool = std::make_shared<Vulkan::CommandPool>();
            pool->initialize(device->getDevice(), idQueue, flags);

            // Register
            device->insertCommandPool(pool);
            _commandPools[id] = pool;
        }
    }
}

void Engine3DXMLLoader::loadCommandsGroup(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak)
{
    MouCa::preCondition(!deviceWeak.expired()); //DEV Issue: Bad device !

    // Search CommandBuffer
    auto result = context._parser.getNode("CommandsGroups");
    if (result->getNbElements() > 0)
    {
        MouCa::assertion(result->getNbElements() == 1); //DEV Issue: please clean xml ?
        auto device = deviceWeak.lock();

        auto aPush = context._parser.autoPushNode(*result->getNode(0));

        // Parsing all images
        auto allCommandsGroup = context._parser.getNode("CommandsGroup");
        for (size_t idGroup = 0; idGroup < allCommandsGroup->getNbElements(); ++idGroup)
        {
            auto groupNode = allCommandsGroup->getNode(idGroup);

            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(groupNode, "CommandsGroup", _commandsGroup, context, existing);
            if (existing)
            {
                continue;
            }

            // Load commands
            auto allCommands = std::make_unique<Vulkan::Commands>();
            {
                // Enter to node each time to parse properly
                auto aPushC = context._parser.autoPushNode(*groupNode);

                // Build command for specific framebuffer
                loadCommands(context, deviceWeak, VkExtent2D(), *allCommands);
            }

            // Register
            _commandsGroup[id] = std::move(allCommands);
        }
    }
}

void Engine3DXMLLoader::loadCommandBuffers(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak)
{
    MouCa::preCondition(!deviceWeak.expired()); //DEV Issue: Bad device !

    // Search CommandBuffer
    auto result = context._parser.getNode("CommandBuffers");
    if (result->getNbElements() > 0)
    {
        MouCa::assertion(result->getNbElements() == 1); //DEV Issue: please clean xml ?
        auto device = deviceWeak.lock();

        auto aPush = context._parser.autoPushNode(*result->getNode(0));

        // Parsing all images
        auto allCommandBuffers = context._parser.getNode("CommandBuffer");
        for (size_t idCommandBuffer = 0; idCommandBuffer < allCommandBuffers->getNbElements(); ++idCommandBuffer)
        {
            auto commandBufferNode = allCommandBuffers->getNode(idCommandBuffer);

            const uint32_t poolId = LoaderHelper::getLinkedIdentifiant(commandBufferNode, "commandPoolId", _commandPools, context);

            const VkCommandBufferLevel level = LoaderHelper::readValue(commandBufferNode, "level", commandBufferLevels, false, context);

            VkCommandBufferUsageFlags usage = 0;
            if (commandBufferNode->hasAttribute("usage"))
            {
                usage = LoaderHelper::readValue(commandBufferNode, "usage", commandBufferUsages, true, context);
            }

            // Special command buffer for surface
            if (commandBufferNode->hasAttribute("surfaceId"))
            {
                const uint32_t surfaceId       = LoaderHelper::getLinkedIdentifiant(commandBufferNode, "surfaceId", _surfaces, context);
                auto surface = _surfaces[surfaceId].lock();
                MouCa::assertion(surface->getSwapChain().getImages().size() == surface->getFrameBuffer().size());

                // Load commands
                Vulkan::Commands allCommands;
                {
                    // Enter to node each time to parse properly
                    auto aPushC = context._parser.autoPushNode(*commandBufferNode);

                    // Build command for specific framebuffer
                    loadCommands(context, deviceWeak, surface->getFormat().getConfiguration()._extent, allCommands);
                }

                // Create command buffer for surface
                surface->createCommandBuffer(std::move(allCommands), _commandPools[poolId].lock(), level, usage);
            }
            else
            {
                bool existing;
                const uint32_t id = LoaderHelper::getIdentifiant(commandBufferNode, "CommandBuffer", _commandBuffers, context, existing);
                if (existing)
                {
                    continue;
                }

                // Enter to node each time to parse properly
                auto aPushC = context._parser.autoPushNode(*commandBufferNode);

                // Build command
                Vulkan::Commands allCommands;
                loadCommands(context, deviceWeak, VkExtent2D(), allCommands);

                // Build new CommandBuffer
                auto commandBuffer = std::make_shared<Vulkan::CommandBuffer>();
                commandBuffer->initialize(device->getDevice(), _commandPools[poolId].lock(), level, usage);
                commandBuffer->registerCommands(std::move(allCommands));

                deviceWeak.lock()->insertCommandBuffer(commandBuffer);
                _commandBuffers[id] = commandBuffer;
            }
        }
    }
}

void Engine3DXMLLoader::loadCommands(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak, const VkExtent2D& resolution, std::vector<Vulkan::CommandUPtr>& commands, const Core::String& nodeName)
{
    // Parsing all commands
    auto allCommands = context._parser.getNode(nodeName);
    for (size_t idCommand = 0; idCommand < allCommands->getNbElements(); ++idCommand)
    {
        auto commandNode = allCommands->getNode(idCommand);

        Core::String type;
        commandNode->getAttribute("type", type);

        // Build command
        Vulkan::CommandUPtr command;        
        if(type == "viewport")
        {
            // Build default viewport
            VkViewport viewport
            {
                0.0f, 0.0f,
                static_cast<float>(resolution.width), static_cast<float>(resolution.height),
                0.0f, 1.0f
            };

            if(commandNode->hasAttribute("x"))        { commandNode->getAttribute("x",        viewport.x); }
            if(commandNode->hasAttribute("y"))        { commandNode->getAttribute("y",        viewport.y); }
            if(commandNode->hasAttribute("width"))    { LoaderHelper::readAttribute(commandNode, "width",  viewport.width, context);  }
            if(commandNode->hasAttribute("height"))   { LoaderHelper::readAttribute(commandNode, "height", viewport.height, context); }
            if(commandNode->hasAttribute("minDepth")) { commandNode->getAttribute("minDepth", viewport.minDepth); }
            if(commandNode->hasAttribute("maxDepth")) { commandNode->getAttribute("maxDepth", viewport.maxDepth); }
            MouCa::assertion(viewport.width != 0 && viewport.height != 0);
            
            // Build command
            command = std::make_unique<Vulkan::CommandViewport>(viewport);
        }
        else if(type == "scissor")
        {
            // Build default rect
            VkRect2D rect{ {0, 0}, resolution };
            
            if(commandNode->hasAttribute("x"))      { commandNode->getAttribute("x",      rect.offset.x); }
            if(commandNode->hasAttribute("y"))      { commandNode->getAttribute("y",      rect.offset.y); }
            if(commandNode->hasAttribute("width"))  { LoaderHelper::readAttribute(commandNode, "width",  rect.extent.width, context);  }
            if(commandNode->hasAttribute("height")) { LoaderHelper::readAttribute(commandNode, "height", rect.extent.height, context); }

            MouCa::assertion(rect.extent.width > 0 && rect.extent.height > 0);
            // Build command
            command = std::make_unique<Vulkan::CommandScissor>(rect);
        }
        else if(type == "beginRenderPass")
        {
            const uint32_t renderPassId = LoaderHelper::getLinkedIdentifiant(commandNode, "renderPassId", _renderPasses, context);

            // Get specified frameBuffer or use transmit (only for swapchain)
            VkRect2D renderArea{ {0,0}, resolution };
            std::vector<Vulkan::FrameBufferWPtr> frameBuffers;
            if (commandNode->hasAttribute("frameBufferId"))
            {
                const uint32_t idF = LoaderHelper::getLinkedIdentifiant(commandNode, "frameBufferId", _frameBuffers, context);
                frameBuffers.emplace_back(_frameBuffers[idF]);

                // Use by default framebuffer extent
                renderArea.extent = _frameBuffers[idF].lock()->getResolution();
            }
            else if (commandNode->hasAttribute("surfaceId"))
            {
                const uint32_t idS = LoaderHelper::getLinkedIdentifiant(commandNode, "surfaceId", _surfaces, context);
                const auto& fbs = _surfaces[idS].lock()->getFrameBuffer();
                frameBuffers.resize(fbs.size());
                std::copy(fbs.cbegin(), fbs.cend(), frameBuffers.begin());
            }

            MouCa::assertion(renderArea.extent.width > 0 && renderArea.extent.height > 0);

            if(commandNode->hasAttribute("x"))      { commandNode->getAttribute("x",      renderArea.offset.x); }
            if(commandNode->hasAttribute("y"))      { commandNode->getAttribute("y",      renderArea.offset.y); }
            if(commandNode->hasAttribute("width"))  { commandNode->getAttribute("width",  renderArea.extent.width); }
            if(commandNode->hasAttribute("height")) { commandNode->getAttribute("height", renderArea.extent.height); }
            const auto subpassContent = LoaderHelper::readValue(commandNode, "subpassContent", subpassContents, false, context);
            
            // Read clear
            auto aPushC = context._parser.autoPushNode(*commandNode);

            auto allCleanValue = context._parser.getNode("CleanValue");

            std::vector<VkClearValue> clearColors(allCleanValue->getNbElements());
            auto itClean = clearColors.begin();
            for(size_t idCleanValue = 0; idCleanValue < allCleanValue->getNbElements(); ++idCleanValue)
            {
                auto cleanValueNode = allCleanValue->getNode(idCleanValue);

                bool isColor = cleanValueNode->hasAttribute("type") || cleanValueNode->hasAttribute("colorR") || cleanValueNode->hasAttribute("colorG") || cleanValueNode->hasAttribute("colorB") || cleanValueNode->hasAttribute("colorA");
                bool isDS    = cleanValueNode->hasAttribute("depth") || cleanValueNode->hasAttribute("stencil");
                // Check validity
                if(!(isColor ^ isDS))
                {
                    throw Core::Exception(Core::ErrorData("Engine3D", "XMLCleanValueMixError") << context.getFileName());
                }

                if(isColor)
                {
                    Core::String dataType;
                    cleanValueNode->getAttribute("type", dataType);

                    if(dataType=="float")
                    {
                        cleanValueNode->getAttribute("colorR", itClean->color.float32[0]);
                        cleanValueNode->getAttribute("colorG", itClean->color.float32[1]);
                        cleanValueNode->getAttribute("colorB", itClean->color.float32[2]);
                        cleanValueNode->getAttribute("colorA", itClean->color.float32[3]);
                    }
                    else if(dataType=="int")
                    {
                        cleanValueNode->getAttribute("colorR", itClean->color.int32[0]);
                        cleanValueNode->getAttribute("colorG", itClean->color.int32[1]);
                        cleanValueNode->getAttribute("colorB", itClean->color.int32[2]);
                        cleanValueNode->getAttribute("colorA", itClean->color.int32[3]);
                    }
                    else if(dataType=="uint")
                    {
                        cleanValueNode->getAttribute("colorR", itClean->color.uint32[0]);
                        cleanValueNode->getAttribute("colorG", itClean->color.uint32[1]);
                        cleanValueNode->getAttribute("colorB", itClean->color.uint32[2]);
                        cleanValueNode->getAttribute("colorA", itClean->color.uint32[3]);
                    }
                    else
                    {
                        throw Core::Exception(Core::ErrorData("Engine3D", "XMLCleanValueTypeError") << context.getFileName() << dataType);
                    }
                }
                else
                {
                    cleanValueNode->getAttribute("depth",   itClean->depthStencil.depth);
                    cleanValueNode->getAttribute("stencil", itClean->depthStencil.stencil);
                }

                ++itClean;
            }

            // Build command
            if (frameBuffers.size() > 1)
            {
                command = std::make_unique<Vulkan::CommandBeginRenderPassSurface>(*_renderPasses[renderPassId].lock(), std::move(frameBuffers), renderArea, std::move(clearColors), subpassContent);
            }
            else
            {
                command = std::make_unique<Vulkan::CommandBeginRenderPass>(*_renderPasses[renderPassId].lock(), frameBuffers.front(), renderArea, std::move(clearColors), subpassContent);
            }
        }
        else if(type == "endRenderPass")
        {
            // Build command
            command = std::make_unique<Vulkan::CommandEndRenderPass>();
        }
        else if(type == "bindPipeline")
        {
            const auto       bindPoint          = LoaderHelper::readValue(commandNode, "bindPoint", bindPoints, false, context);

            if(commandNode->hasAttribute("graphicsPipelineId"))
            {
                const uint32_t pipelineId = LoaderHelper::getLinkedIdentifiant(commandNode, "graphicsPipelineId", _graphicsPipelines, context);
                command = std::make_unique<Vulkan::CommandBindPipeline>(_graphicsPipelines[pipelineId], bindPoint);
            }
            else if (commandNode->hasAttribute("rayTracingPipelineId"))
            {
                const uint32_t pipelineId = LoaderHelper::getLinkedIdentifiant(commandNode, "rayTracingPipelineId", _rayTracingPipelines, context);
                command = std::make_unique<Vulkan::CommandBindPipeline>(_rayTracingPipelines[pipelineId], bindPoint);
            }
            else
            {
                throw Core::Exception(Core::ErrorData("Engine3D", "XMLUnknownBindPipelineError") << context.getFileName() << type);
            }
        }
        else if (type == "draw")
        {
            uint32_t vertexCount;
            uint32_t instanceCount;
            uint32_t firstVertex;
            uint32_t firstInstance;
            commandNode->getAttribute("vertexCount",   vertexCount);
            commandNode->getAttribute("instanceCount", instanceCount);
            commandNode->getAttribute("firstVertex",   firstVertex);
            commandNode->getAttribute("firstInstance", firstInstance);
            command = std::make_unique<Vulkan::CommandDraw>(vertexCount, instanceCount, firstVertex, firstInstance);
        }
        else if (type == "bindVertexBuffers")
        {
            uint32_t firstBinding;
            uint32_t bindingCount;
            commandNode->getAttribute("firstBinding",  firstBinding);
            commandNode->getAttribute("bindingCount",  bindingCount);

            // Read clear
            auto aPushB = context._parser.autoPushNode(*commandNode);

            auto allBuffers = context._parser.getNode("Buffer");

            std::vector<Vulkan::BufferWPtr> buffers;
            buffers.reserve(allBuffers->getNbElements());
            std::vector<VkDeviceSize> offsets;
            offsets.reserve(allBuffers->getNbElements());
            
            for (size_t idBuffer = 0; idBuffer < allBuffers->getNbElements(); ++idBuffer)
            {
                auto bufferNode = allBuffers->getNode(idBuffer);
                const uint32_t idB = LoaderHelper::getLinkedIdentifiant(bufferNode, "bufferId", _buffers, context);
                VkDeviceSize offset;
                bufferNode->getAttribute("offset", offset);

                buffers.emplace_back(_buffers[idB]);
                offsets.emplace_back(offset);
            }
            command = std::make_unique<Vulkan::CommandBindVertexBuffer>(firstBinding, bindingCount, std::move(buffers), std::move(offsets));
        }
        else if (type == "bindIndexBuffers")
        {
            const uint32_t idB = LoaderHelper::getLinkedIdentifiant(commandNode, "bufferId", _buffers, context);
            VkDeviceSize offset;
            commandNode->getAttribute("offset", offset);
            VkIndexType indexType = LoaderHelper::readValue(commandNode, "indexType", indexTypes, false, context);

            command = std::make_unique<Vulkan::CommandBindIndexBuffer>(_buffers[idB], offset, indexType);
        }
        else if (type == "drawIndexed")
        {
            uint32_t indexCount;
            if (commandNode->hasAttribute("bufferId"))
            {
                uint32_t idB = LoaderHelper::getLinkedIdentifiant(commandNode, "bufferId", _cpuBuffers, context);
                indexCount = static_cast<uint32_t>(_cpuBuffers[idB].lock()->getNbElements() * 3);
            }
            else
            {
                commandNode->getAttribute("indexCount", indexCount);
            }
            uint32_t instanceCount;
            commandNode->getAttribute("instanceCount", instanceCount);
            uint32_t firstIndex;
            commandNode->getAttribute("firstIndex", firstIndex);
            int32_t  vertexOffset;
            commandNode->getAttribute("vertexOffset", vertexOffset);
            uint32_t firstInstance;
            commandNode->getAttribute("firstInstance", firstInstance);

            command = std::make_unique<Vulkan::CommandDrawIndexed>(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
        }
        else if (type == "bindDescriptorSets")
        {
            const uint32_t idP       = LoaderHelper::getLinkedIdentifiant(commandNode, "pipelineLayoutId", _pipelineLayouts, context);
            const auto       bindPoint = LoaderHelper::readValue(commandNode, "bindPoint", bindPoints, false, context);
            uint32_t firstSet;
            commandNode->getAttribute("firstSet", firstSet);
            const uint32_t idS       = LoaderHelper::getLinkedIdentifiant(commandNode, "descriptorSetId", _descriptorSets, context);

            auto aPushB = context._parser.autoPushNode(*commandNode);
                       
            // Read Dynamic Offset
            auto allDynamicOffsets = context._parser.getNode("DynamicOffset");
            std::vector<uint32_t> dynamicOffsets;
            dynamicOffsets.reserve(allDynamicOffsets->getNbElements());

            for (size_t idDynamicOffset = 0; idDynamicOffset < allDynamicOffsets->getNbElements(); ++idDynamicOffset)
            {
                auto dynamicOffsetNode = allDynamicOffsets->getNode(idDynamicOffset);
                uint32_t offset;
                dynamicOffsetNode->getAttribute("offset", offset);

                dynamicOffsets.emplace_back(offset);
            }
            command = std::make_unique<Vulkan::CommandBindDescriptorSets>(*_pipelineLayouts[idP].lock(), bindPoint, firstSet, _descriptorSets[idS].lock()->getDescriptorSets(), std::move(dynamicOffsets));
        }
        else if (type == "pipelineBarrier")
        {
            const auto srcStage   = LoaderHelper::readValue(commandNode, "srcStage",        pipelineStageFlags, true, context);
            const auto dstStage   = LoaderHelper::readValue(commandNode, "dstStage",        pipelineStageFlags, true, context);
            const auto dependency = LoaderHelper::readValue(commandNode, "dependencyFlags", dependencyFlags,    true, context);
         
            auto aPushP = context._parser.autoPushNode(*commandNode);

            // Read image memories
            auto allImageMemories = context._parser.getNode("ImageBarrier");
            Vulkan::CommandPipelineBarrier::ImageMemoryBarriers imageBarriers;
            imageBarriers.reserve(allImageMemories->getNbElements());

            for (size_t idImageMemory = 0; idImageMemory < allImageMemories->getNbElements(); ++idImageMemory)
            {
                auto imageMemoryNode = allImageMemories->getNode(idImageMemory);

                const uint32_t idI = LoaderHelper::getLinkedIdentifiant(imageMemoryNode, "imageId", _images, context);

                VkImageMemoryBarrier barrier
                {
                    VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    nullptr,
                    LoaderHelper::readValue(imageMemoryNode, "srcAccessMask", accessFlags, true, context),
                    LoaderHelper::readValue(imageMemoryNode, "dstAccessMask", accessFlags, true, context),
                    LoaderHelper::readValue(imageMemoryNode, "oldLayout",     imageLayouts, false, context),
                    LoaderHelper::readValue(imageMemoryNode, "newLayout",     imageLayouts, false, context),
                    0, 0,
                    _images[idI].lock()->getImage(),
                    { LoaderHelper::readValue(imageMemoryNode, "aspectMask", aspectFlags, true, context)},
                };

                imageMemoryNode->getAttribute("srcQueueFamilyIndex", barrier.srcQueueFamilyIndex);
                imageMemoryNode->getAttribute("dstQueueFamilyIndex", barrier.dstQueueFamilyIndex);
                imageMemoryNode->getAttribute("baseMipLevel",        barrier.subresourceRange.baseMipLevel);
                imageMemoryNode->getAttribute("levelCount",          barrier.subresourceRange.levelCount);
                imageMemoryNode->getAttribute("baseArrayLayer",      barrier.subresourceRange.baseArrayLayer);
                imageMemoryNode->getAttribute("layerCount",          barrier.subresourceRange.layerCount);

                imageBarriers.emplace_back(barrier);
            }

            Vulkan::CommandPipelineBarrier::MemoryBarriers memoryBarriers;
            Vulkan::CommandPipelineBarrier::BufferMemoryBarriers bufferMemoryBarriers;

            command = std::make_unique<Vulkan::CommandPipelineBarrier>(srcStage, dstStage, dependency, std::move(memoryBarriers), std::move(bufferMemoryBarriers), std::move(imageBarriers));
        }
        else if (type == "pushConstants")
        {
            const uint32_t idL           = LoaderHelper::getLinkedIdentifiant(commandNode, "pipelineLayoutId", _pipelineLayouts, context);
            const VkShaderStageFlags stage = LoaderHelper::readValue(commandNode, "stage", shaderStages, true, context);
            const uint32_t idB           = LoaderHelper::getLinkedIdentifiant(commandNode, "external", _cpuBuffers, context);
            const auto& buffer             = _cpuBuffers[idB].lock();

            command = std::make_unique<Vulkan::CommandPushConstants>(*_pipelineLayouts[idL].lock(), stage, static_cast<uint32_t>(buffer->getByteSize()), buffer->getData());
        }
        else if (type == "container")
        {
            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(commandNode, nodeName, _commandLinks, context, existing);
            if (existing)
            {
                continue;
            }

            command = std::make_unique<Vulkan::CommandContainer>();
            _commandLinks[id] = command.get(); // Keep pointer (badly because can be delete -> shared/weak ?)
        }
        else if (type == "switch")
        {
            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(commandNode, nodeName, _commandLinks, context, existing);
            if (existing)
            {
                continue;
            }
            auto commandSwitch = std::make_unique<Vulkan::CommandSwitch>();
            _commandLinks[id] = commandSwitch.get(); // Keep pointer (badly because can be delete -> shared/weak ?)

            // Load sub-commands
            {
                auto aPush = context._parser.autoPushNode(*commandNode);

                std::vector<Vulkan::CommandUPtr> subCommands;
                loadCommands(context, deviceWeak, resolution, subCommands, Core::String("Sub"+nodeName));

                commandSwitch->transfer(std::move(subCommands));
            }

            command = std::move(commandSwitch);
        }
        else if (type == "traceRays")
        {
            uint32_t width;
            uint32_t height;
            uint32_t depth;
            LoaderHelper::readAttribute(commandNode, "width",  width,  context);
            LoaderHelper::readAttribute(commandNode, "height", height, context);
            LoaderHelper::readAttribute(commandNode, "depth",  depth,  context);

            const uint32_t idT = LoaderHelper::getLinkedIdentifiant(commandNode, "tracingRayId", _tracingRays, context);
           
            auto commandTraceRay = std::make_unique<Vulkan::CommandTraceRay>(deviceWeak.lock()->getDevice(), _tracingRays[idT], width, height, depth);
            command = std::move(commandTraceRay);
        }
        else
        {
        throw Core::Exception(Core::ErrorData("Engine3D", "XMLUnknownCommandError") << context.getFileName() << type);
        }

        // Transfer to vector
        commands.emplace_back(std::move(command));
    }
}

}