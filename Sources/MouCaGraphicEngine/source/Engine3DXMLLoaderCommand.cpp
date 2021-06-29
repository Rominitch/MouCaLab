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
    MOUCA_PRE_CONDITION(!deviceWeak.expired()); //DEV Issue: Bad device !

    // Search CommandBuffer
    auto result = context._parser.getNode(u8"CommandPools");
    if (result->getNbElements() > 0)
    {
        MOUCA_ASSERT(result->getNbElements() == 1); //DEV Issue: please clean xml ?
        auto device = deviceWeak.lock();

        auto aPush = context._parser.autoPushNode(*result->getNode(0));
        // Parsing all images
        auto allCommandPools = context._parser.getNode(u8"CommandPool");
        for (size_t idPool = 0; idPool < allCommandPools->getNbElements(); ++idPool)
        {
            auto poolNode = allCommandPools->getNode(idPool);

            // Mandatory attribute
            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(poolNode, u8"CommandPool", _commandPools, context, existing);
            if (existing)
            {
                continue;
            }

            int idQueue = 0;
            if (poolNode->hasAttribute(u8"families"))
            {
                Core::String families;
                poolNode->getAttribute(u8"families", families);
                if (families == u8"graphic")
                {
                    idQueue = device->getDevice().getQueueFamilyGraphicId();
                }   
                else if (families == u8"compute")
                {
                    idQueue = device->getDevice().getQueueFamilyComputeId();
                }
                else if (families == u8"transfer")
                {
                    idQueue = device->getDevice().getQueueFamilyTransferId();
                }
                else
                {
                    MOUCA_THROW_ERROR_2(u8"Engine3D", u8"UnknownFamiliesError", context.getFileName(), families);
                }   
            }
            else if (poolNode->hasAttribute(u8"queueId"))
            {
                poolNode->getAttribute(u8"queueId", idQueue);
            }

            VkCommandPoolCreateFlags flags = 0;
            if (poolNode->hasAttribute(u8"flags"))
            {
                flags = LoaderHelper::readValue(poolNode, u8"flags", poolCreateFlags, true, context);
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
    MOUCA_PRE_CONDITION(!deviceWeak.expired()); //DEV Issue: Bad device !

    // Search CommandBuffer
    auto result = context._parser.getNode(u8"CommandsGroups");
    if (result->getNbElements() > 0)
    {
        MOUCA_ASSERT(result->getNbElements() == 1); //DEV Issue: please clean xml ?
        auto device = deviceWeak.lock();

        auto aPush = context._parser.autoPushNode(*result->getNode(0));

        // Parsing all images
        auto allCommandsGroup = context._parser.getNode(u8"CommandsGroup");
        for (size_t idGroup = 0; idGroup < allCommandsGroup->getNbElements(); ++idGroup)
        {
            auto groupNode = allCommandsGroup->getNode(idGroup);

            bool existing;
            const uint32_t id = LoaderHelper::getIdentifiant(groupNode, u8"CommandsGroup", _commandsGroup, context, existing);
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
    MOUCA_PRE_CONDITION(!deviceWeak.expired()); //DEV Issue: Bad device !

    // Search CommandBuffer
    auto result = context._parser.getNode(u8"CommandBuffers");
    if (result->getNbElements() > 0)
    {
        MOUCA_ASSERT(result->getNbElements() == 1); //DEV Issue: please clean xml ?
        auto device = deviceWeak.lock();

        auto aPush = context._parser.autoPushNode(*result->getNode(0));

        // Parsing all images
        auto allCommandBuffers = context._parser.getNode(u8"CommandBuffer");
        for (size_t idCommandBuffer = 0; idCommandBuffer < allCommandBuffers->getNbElements(); ++idCommandBuffer)
        {
            auto commandBufferNode = allCommandBuffers->getNode(idCommandBuffer);

            const uint32_t poolId = LoaderHelper::getLinkedIdentifiant(commandBufferNode, u8"commandPoolId", _commandPools, context);

            const VkCommandBufferLevel level = LoaderHelper::readValue(commandBufferNode, u8"level", commandBufferLevels, false, context);

            VkCommandBufferUsageFlags usage = 0;
            if (commandBufferNode->hasAttribute(u8"usage"))
            {
                usage = LoaderHelper::readValue(commandBufferNode, u8"usage", commandBufferUsages, true, context);
            }

            // Special command buffer for surface
            if (commandBufferNode->hasAttribute("surfaceId"))
            {
                const uint32_t surfaceId       = LoaderHelper::getLinkedIdentifiant(commandBufferNode, u8"surfaceId", _surfaces, context);
                auto surface = _surfaces[surfaceId].lock();
                MOUCA_ASSERT(surface->getSwapChain().getImages().size() == surface->getFrameBuffer().size());

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
                const uint32_t id = LoaderHelper::getIdentifiant(commandBufferNode, u8"CommandBuffer", _commandBuffers, context, existing);
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
        commandNode->getAttribute(u8"type", type);

        // Build command
        Vulkan::CommandUPtr command;        
        if(type == u8"viewport")
        {
            // Build default viewport
            VkViewport viewport
            {
                0.0f, 0.0f,
                static_cast<float>(resolution.width), static_cast<float>(resolution.height),
                0.0f, 1.0f
            };

            if(commandNode->hasAttribute(u8"x"))        { commandNode->getAttribute(u8"x",        viewport.x); }
            if(commandNode->hasAttribute(u8"y"))        { commandNode->getAttribute(u8"y",        viewport.y); }
            if(commandNode->hasAttribute(u8"width"))    { LoaderHelper::readAttribute(commandNode, u8"width",  viewport.width, context);  }
            if(commandNode->hasAttribute(u8"height"))   { LoaderHelper::readAttribute(commandNode, u8"height", viewport.height, context); }
            if(commandNode->hasAttribute(u8"minDepth")) { commandNode->getAttribute(u8"minDepth", viewport.minDepth); }
            if(commandNode->hasAttribute(u8"maxDepth")) { commandNode->getAttribute(u8"maxDepth", viewport.maxDepth); }
            MOUCA_ASSERT(viewport.width != 0 && viewport.height != 0);
            
            // Build command
            command = std::make_unique<Vulkan::CommandViewport>(viewport);
        }
        else if(type == u8"scissor")
        {
            // Build default rect
            VkRect2D rect{ {0, 0}, resolution };
            
            if(commandNode->hasAttribute(u8"x"))      { commandNode->getAttribute(u8"x",      rect.offset.x); }
            if(commandNode->hasAttribute(u8"y"))      { commandNode->getAttribute(u8"y",      rect.offset.y); }
            if(commandNode->hasAttribute(u8"width"))  { LoaderHelper::readAttribute(commandNode, u8"width",  rect.extent.width, context);  }
            if(commandNode->hasAttribute(u8"height")) { LoaderHelper::readAttribute(commandNode, u8"height", rect.extent.height, context); }

            MOUCA_ASSERT(rect.extent.width > 0 && rect.extent.height > 0);
            // Build command
            command = std::make_unique<Vulkan::CommandScissor>(rect);
        }
        else if(type == u8"beginRenderPass")
        {
            const uint32_t renderPassId = LoaderHelper::getLinkedIdentifiant(commandNode, u8"renderPassId", _renderPasses, context);

            // Get specified frameBuffer or use transmit (only for swapchain)
            VkRect2D renderArea{ {0,0}, resolution };
            std::vector<Vulkan::FrameBufferWPtr> frameBuffers;
            if (commandNode->hasAttribute(u8"frameBufferId"))
            {
                const uint32_t idF = LoaderHelper::getLinkedIdentifiant(commandNode, u8"frameBufferId", _frameBuffers, context);
                frameBuffers.emplace_back(_frameBuffers[idF]);

                // Use by default framebuffer extent
                renderArea.extent = _frameBuffers[idF].lock()->getResolution();
            }
            else if (commandNode->hasAttribute(u8"surfaceId"))
            {
                const uint32_t idS = LoaderHelper::getLinkedIdentifiant(commandNode, u8"surfaceId", _surfaces, context);
                const auto& fbs = _surfaces[idS].lock()->getFrameBuffer();
                frameBuffers.resize(fbs.size());
                std::copy(fbs.cbegin(), fbs.cend(), frameBuffers.begin());
            }

            MOUCA_ASSERT(renderArea.extent.width > 0 && renderArea.extent.height > 0);

            if(commandNode->hasAttribute(u8"x"))      { commandNode->getAttribute(u8"x",      renderArea.offset.x); }
            if(commandNode->hasAttribute(u8"y"))      { commandNode->getAttribute(u8"y",      renderArea.offset.y); }
            if(commandNode->hasAttribute(u8"width"))  { commandNode->getAttribute(u8"width",  renderArea.extent.width); }
            if(commandNode->hasAttribute(u8"height")) { commandNode->getAttribute(u8"height", renderArea.extent.height); }
            const auto subpassContent = LoaderHelper::readValue(commandNode, u8"subpassContent", subpassContents, false, context);
            
            // Read clear
            auto aPushC = context._parser.autoPushNode(*commandNode);

            auto allCleanValue = context._parser.getNode(u8"CleanValue");

            std::vector<VkClearValue> clearColors(allCleanValue->getNbElements());
            auto itClean = clearColors.begin();
            for(size_t idCleanValue = 0; idCleanValue < allCleanValue->getNbElements(); ++idCleanValue)
            {
                auto cleanValueNode = allCleanValue->getNode(idCleanValue);

                bool isColor = cleanValueNode->hasAttribute(u8"type") || cleanValueNode->hasAttribute(u8"colorR") || cleanValueNode->hasAttribute(u8"colorG") || cleanValueNode->hasAttribute(u8"colorB") || cleanValueNode->hasAttribute(u8"colorA");
                bool isDS    = cleanValueNode->hasAttribute(u8"depth") || cleanValueNode->hasAttribute(u8"stencil");
                // Check validity
                if(!(isColor ^ isDS))
                {
                    MOUCA_THROW_ERROR_1(u8"Engine3D", u8"XMLCleanValueMixError", context.getFileName());
                }

                if(isColor)
                {
                    Core::String dataType;
                    cleanValueNode->getAttribute(u8"type", dataType);

                    if(dataType==u8"float")
                    {
                        cleanValueNode->getAttribute(u8"colorR", itClean->color.float32[0]);
                        cleanValueNode->getAttribute(u8"colorG", itClean->color.float32[1]);
                        cleanValueNode->getAttribute(u8"colorB", itClean->color.float32[2]);
                        cleanValueNode->getAttribute(u8"colorA", itClean->color.float32[3]);
                    }
                    else if(dataType==u8"int")
                    {
                        cleanValueNode->getAttribute(u8"colorR", itClean->color.int32[0]);
                        cleanValueNode->getAttribute(u8"colorG", itClean->color.int32[1]);
                        cleanValueNode->getAttribute(u8"colorB", itClean->color.int32[2]);
                        cleanValueNode->getAttribute(u8"colorA", itClean->color.int32[3]);
                    }
                    else if(dataType==u8"uint")
                    {
                        cleanValueNode->getAttribute(u8"colorR", itClean->color.uint32[0]);
                        cleanValueNode->getAttribute(u8"colorG", itClean->color.uint32[1]);
                        cleanValueNode->getAttribute(u8"colorB", itClean->color.uint32[2]);
                        cleanValueNode->getAttribute(u8"colorA", itClean->color.uint32[3]);
                    }
                    else
                    {
                        MOUCA_THROW_ERROR_2(u8"Engine3D", u8"XMLCleanValueTypeError", context.getFileName(), dataType);
                    }
                }
                else
                {
                    cleanValueNode->getAttribute(u8"depth",   itClean->depthStencil.depth);
                    cleanValueNode->getAttribute(u8"stencil", itClean->depthStencil.stencil);
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
        else if(type == u8"endRenderPass")
        {
            // Build command
            command = std::make_unique<Vulkan::CommandEndRenderPass>();
        }
        else if(type == u8"bindPipeline")
        {
            const auto       bindPoint          = LoaderHelper::readValue(commandNode, u8"bindPoint", bindPoints, false, context);

            if(commandNode->hasAttribute(u8"graphicsPipelineId"))
            {
                const uint32_t pipelineId = LoaderHelper::getLinkedIdentifiant(commandNode, u8"graphicsPipelineId", _graphicsPipelines, context);
                command = std::make_unique<Vulkan::CommandBindPipeline>(_graphicsPipelines[pipelineId], bindPoint);
            }
            else if (commandNode->hasAttribute(u8"rayTracingPipelineId"))
            {
                const uint32_t pipelineId = LoaderHelper::getLinkedIdentifiant(commandNode, u8"rayTracingPipelineId", _rayTracingPipelines, context);
                command = std::make_unique<Vulkan::CommandBindPipeline>(_rayTracingPipelines[pipelineId], bindPoint);
            }
            else
            {
                MOUCA_THROW_ERROR_2(u8"Engine3D", u8"XMLUnknownBindPipelineError", context.getFileName(), type);
            }
        }
        else if (type == u8"draw")
        {
            uint32_t vertexCount;
            uint32_t instanceCount;
            uint32_t firstVertex;
            uint32_t firstInstance;
            commandNode->getAttribute(u8"vertexCount",   vertexCount);
            commandNode->getAttribute(u8"instanceCount", instanceCount);
            commandNode->getAttribute(u8"firstVertex",   firstVertex);
            commandNode->getAttribute(u8"firstInstance", firstInstance);
            command = std::make_unique<Vulkan::CommandDraw>(vertexCount, instanceCount, firstVertex, firstInstance);
        }
        else if (type == u8"bindVertexBuffers")
        {
            uint32_t firstBinding;
            uint32_t bindingCount;
            commandNode->getAttribute(u8"firstBinding",  firstBinding);
            commandNode->getAttribute(u8"bindingCount",  bindingCount);

            // Read clear
            auto aPushB = context._parser.autoPushNode(*commandNode);

            auto allBuffers = context._parser.getNode(u8"Buffer");

            std::vector<Vulkan::BufferWPtr> buffers;
            buffers.reserve(allBuffers->getNbElements());
            std::vector<VkDeviceSize> offsets;
            offsets.reserve(allBuffers->getNbElements());
            
            for (size_t idBuffer = 0; idBuffer < allBuffers->getNbElements(); ++idBuffer)
            {
                auto bufferNode = allBuffers->getNode(idBuffer);
                const uint32_t idB = LoaderHelper::getLinkedIdentifiant(bufferNode, u8"bufferId", _buffers, context);
                VkDeviceSize offset;
                bufferNode->getAttribute(u8"offset", offset);

                buffers.emplace_back(_buffers[idB]);
                offsets.emplace_back(offset);
            }
            command = std::make_unique<Vulkan::CommandBindVertexBuffer>(firstBinding, bindingCount, std::move(buffers), std::move(offsets));
        }
        else if (type == u8"bindIndexBuffers")
        {
            const uint32_t idB = LoaderHelper::getLinkedIdentifiant(commandNode, u8"bufferId", _buffers, context);
            VkDeviceSize offset;
            commandNode->getAttribute(u8"offset", offset);
            VkIndexType indexType = LoaderHelper::readValue(commandNode, u8"indexType", indexTypes, false, context);

            command = std::make_unique<Vulkan::CommandBindIndexBuffer>(_buffers[idB], offset, indexType);
        }
        else if (type == u8"drawIndexed")
        {
            uint32_t indexCount;
            if (commandNode->hasAttribute(u8"bufferId"))
            {
                uint32_t idB = LoaderHelper::getLinkedIdentifiant(commandNode, u8"bufferId", _cpuBuffers, context);
                indexCount = static_cast<uint32_t>(_cpuBuffers[idB].lock()->getNbElements() * 3);
            }
            else
            {
                commandNode->getAttribute(u8"indexCount", indexCount);
            }
            uint32_t instanceCount;
            commandNode->getAttribute(u8"instanceCount", instanceCount);
            uint32_t firstIndex;
            commandNode->getAttribute(u8"firstIndex", firstIndex);
            int32_t  vertexOffset;
            commandNode->getAttribute(u8"vertexOffset", vertexOffset);
            uint32_t firstInstance;
            commandNode->getAttribute(u8"firstInstance", firstInstance);

            command = std::make_unique<Vulkan::CommandDrawIndexed>(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
        }
        else if (type == u8"bindDescriptorSets")
        {
            const uint32_t idP       = LoaderHelper::getLinkedIdentifiant(commandNode, u8"pipelineLayoutId", _pipelineLayouts, context);
            const auto       bindPoint = LoaderHelper::readValue(commandNode, u8"bindPoint", bindPoints, false, context);
            uint32_t firstSet;
            commandNode->getAttribute(u8"firstSet", firstSet);
            const uint32_t idS       = LoaderHelper::getLinkedIdentifiant(commandNode, u8"descriptorSetId", _descriptorSets, context);

            auto aPushB = context._parser.autoPushNode(*commandNode);
                       
            // Read Dynamic Offset
            auto allDynamicOffsets = context._parser.getNode(u8"DynamicOffset");
            std::vector<uint32_t> dynamicOffsets;
            dynamicOffsets.reserve(allDynamicOffsets->getNbElements());

            for (size_t idDynamicOffset = 0; idDynamicOffset < allDynamicOffsets->getNbElements(); ++idDynamicOffset)
            {
                auto dynamicOffsetNode = allDynamicOffsets->getNode(idDynamicOffset);
                uint32_t offset;
                dynamicOffsetNode->getAttribute(u8"offset", offset);

                dynamicOffsets.emplace_back(offset);
            }
            command = std::make_unique<Vulkan::CommandBindDescriptorSets>(*_pipelineLayouts[idP].lock(), bindPoint, firstSet, _descriptorSets[idS].lock()->getDescriptorSets(), std::move(dynamicOffsets));
        }
        else if (type == u8"pipelineBarrier")
        {
            const auto srcStage   = LoaderHelper::readValue(commandNode, u8"srcStage",        pipelineStageFlags, true, context);
            const auto dstStage   = LoaderHelper::readValue(commandNode, u8"dstStage",        pipelineStageFlags, true, context);
            const auto dependency = LoaderHelper::readValue(commandNode, u8"dependencyFlags", dependencyFlags,    true, context);
         
            auto aPushP = context._parser.autoPushNode(*commandNode);

            // Read image memories
            auto allImageMemories = context._parser.getNode(u8"ImageBarrier");
            Vulkan::CommandPipelineBarrier::ImageMemoryBarriers imageBarriers;
            imageBarriers.reserve(allImageMemories->getNbElements());

            for (size_t idImageMemory = 0; idImageMemory < allImageMemories->getNbElements(); ++idImageMemory)
            {
                auto imageMemoryNode = allImageMemories->getNode(idImageMemory);

                const uint32_t idI = LoaderHelper::getLinkedIdentifiant(imageMemoryNode, u8"imageId", _images, context);

                VkImageMemoryBarrier barrier
                {
                    VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    nullptr,
                    LoaderHelper::readValue(imageMemoryNode, u8"srcAccessMask", accessFlags, true, context),
                    LoaderHelper::readValue(imageMemoryNode, u8"dstAccessMask", accessFlags, true, context),
                    LoaderHelper::readValue(imageMemoryNode, u8"oldLayout",     imageLayouts, false, context),
                    LoaderHelper::readValue(imageMemoryNode, u8"newLayout",     imageLayouts, false, context),
                    0, 0,
                    _images[idI].lock()->getImage(),
                    { LoaderHelper::readValue(imageMemoryNode, u8"aspectMask", aspectFlags, true, context)},
                };

                imageMemoryNode->getAttribute(u8"srcQueueFamilyIndex", barrier.srcQueueFamilyIndex);
                imageMemoryNode->getAttribute(u8"dstQueueFamilyIndex", barrier.dstQueueFamilyIndex);
                imageMemoryNode->getAttribute(u8"baseMipLevel",        barrier.subresourceRange.baseMipLevel);
                imageMemoryNode->getAttribute(u8"levelCount",          barrier.subresourceRange.levelCount);
                imageMemoryNode->getAttribute(u8"baseArrayLayer",      barrier.subresourceRange.baseArrayLayer);
                imageMemoryNode->getAttribute(u8"layerCount",          barrier.subresourceRange.layerCount);

                imageBarriers.emplace_back(barrier);
            }

            Vulkan::CommandPipelineBarrier::MemoryBarriers memoryBarriers;
            Vulkan::CommandPipelineBarrier::BufferMemoryBarriers bufferMemoryBarriers;

            command = std::make_unique<Vulkan::CommandPipelineBarrier>(srcStage, dstStage, dependency, std::move(memoryBarriers), std::move(bufferMemoryBarriers), std::move(imageBarriers));
        }
        else if (type == u8"pushConstants")
        {
            const uint32_t idL           = LoaderHelper::getLinkedIdentifiant(commandNode, u8"pipelineLayoutId", _pipelineLayouts, context);
            const VkShaderStageFlags stage = LoaderHelper::readValue(commandNode, u8"stage", shaderStages, true, context);
            const uint32_t idB           = LoaderHelper::getLinkedIdentifiant(commandNode, u8"external", _cpuBuffers, context);
            const auto& buffer             = _cpuBuffers[idB].lock();

            command = std::make_unique<Vulkan::CommandPushConstants>(*_pipelineLayouts[idL].lock(), stage, static_cast<uint32_t>(buffer->getByteSize()), buffer->getData());
        }
        else if (type == u8"container")
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
        else if (type == u8"switch")
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
        else if (type == u8"traceRays")
        {
            uint32_t width;
            uint32_t height;
            uint32_t depth;
            LoaderHelper::readAttribute(commandNode, u8"width",  width,  context);
            LoaderHelper::readAttribute(commandNode, u8"height", height, context);
            LoaderHelper::readAttribute(commandNode, u8"depth",  depth,  context);

            const uint32_t idT = LoaderHelper::getLinkedIdentifiant(commandNode, u8"tracingRayId", _tracingRays, context);
           
            auto commandTraceRay = std::make_unique<Vulkan::CommandTraceRay>(deviceWeak.lock()->getDevice(), _tracingRays[idT], width, height, depth);
            command = std::move(commandTraceRay);
        }
        else
        {
            MOUCA_THROW_ERROR_2(u8"Engine3D", u8"XMLUnknownCommandError", context.getFileName(), type);
        }

        // Transfer to vector
        commands.emplace_back(std::move(command));
    }
}

}