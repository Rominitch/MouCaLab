/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include "LibVulkan/include/VKBuffer.h"
#include "LibVulkan/include/VKCommand.h"
#include "LibVulkan/include/VKDescriptorSet.h"
#include "LibVulkan/include/VKFrameBuffer.h"
#include "LibVulkan/include/VKImage.h"
#include "LibVulkan/include/VKMesh.h"
#include "LibVulkan/include/VKGraphicsPipeline.h"
#include "LibVulkan/include/VKPipelineLayout.h"
#include "LibVulkan/include/VKRenderPass.h"

namespace Vulkan
{

CommandBeginRenderPass::CommandBeginRenderPass(const RenderPass& renderPass, FrameBufferWPtr frameBuffer, const VkRect2D& renderArea, std::vector<VkClearValue>&& clearColor, const VkSubpassContents subpassContent):
_clearColor(std::move(clearColor)), _subpassContent(subpassContent), _frameBuffer(frameBuffer),
_renderPassBeginInfo(
{
    VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,                   // VkStructureType        sType
    nullptr,                                                    // const void*            pNext
    renderPass.getInstance(),                                   // VkRenderPass           renderPass
    VK_NULL_HANDLE,                                             // VkFramebuffer          framebuffer
    renderArea,                                                 // VkRect2D               renderArea
    static_cast<uint32_t>(_clearColor.size()),                  // uint32_t               clearValueCount
    _clearColor.empty() ? VK_NULL_HANDLE : _clearColor.data()   // const VkClearValue*    pClearValues
})
{
    MOUCA_PRE_CONDITION(!frameBuffer.expired() && !frameBuffer.lock()->isNull());
    MOUCA_PRE_CONDITION(!renderPass.isNull());
}

void CommandBeginRenderPass::execute(const VkCommandBuffer& commandBuffer)
{
    MOUCA_PRE_CONDITION(!_frameBuffer.expired() ); //DEV Issue: Need a valid FrameBuffer
    const auto frameBuffer = _frameBuffer.lock();
    MOUCA_PRE_CONDITION(!frameBuffer->isNull());

    // Refresh frameBuffer info
    _renderPassBeginInfo.renderArea.extent = frameBuffer->getResolution();
    _renderPassBeginInfo.framebuffer       = frameBuffer->getInstance();

    vkCmdBeginRenderPass(commandBuffer, &_renderPassBeginInfo, _subpassContent);
}

void CommandBeginRenderPass::execute(const ExecuteCommands& executer)
{
    MOUCA_PRE_CONDITION(!_frameBuffer.expired()); //DEV Issue: Need a valid FrameBuffer
    const auto frameBuffer = _frameBuffer.lock();
    MOUCA_PRE_CONDITION(!frameBuffer->isNull());

    // Refresh frameBuffer info
    _renderPassBeginInfo.renderArea.extent = frameBuffer->getResolution();
    _renderPassBeginInfo.framebuffer       = frameBuffer->getInstance();

    vkCmdBeginRenderPass(executer.commandBuffer, &_renderPassBeginInfo, _subpassContent);
}

CommandBeginRenderPassSurface::CommandBeginRenderPassSurface(const RenderPass& renderPass, std::vector<FrameBufferWPtr>&& frameBuffer, const VkRect2D& renderArea, std::vector<VkClearValue>&& clearColor, const VkSubpassContents subpassContent):
_clearColor(std::move(clearColor)), _subpassContent(subpassContent), _frameBuffer(std::move(frameBuffer)),
_renderPassBeginInfo(
{
    VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,                   // VkStructureType        sType
    nullptr,                                                    // const void*            pNext
    renderPass.getInstance(),                                   // VkRenderPass           renderPass
    VK_NULL_HANDLE,                                             // VkFramebuffer          framebuffer
    renderArea,                                                 // VkRect2D               renderArea
    static_cast<uint32_t>(_clearColor.size()),                  // uint32_t               clearValueCount
    _clearColor.empty() ? VK_NULL_HANDLE : _clearColor.data()   // const VkClearValue*    pClearValues
})
{
    MOUCA_PRE_CONDITION(!_frameBuffer.empty());
    MOUCA_PRE_CONDITION(!renderPass.isNull());
}

void CommandBeginRenderPassSurface::execute(const VkCommandBuffer& commandBuffer)
{
    MOUCA_PRE_CONDITION(false); //DEV Issue: Not callable API
}

void CommandBeginRenderPassSurface::execute(const ExecuteCommands& executer)
{
    MOUCA_PRE_CONDITION(executer.idSwap < _frameBuffer.size()); //DEV Issue: Need a valid FrameBuffer
    const auto frameBuffer = _frameBuffer[executer.idSwap].lock();
    MOUCA_PRE_CONDITION(!frameBuffer->isNull());

    // Refresh frameBuffer info
    _renderPassBeginInfo.renderArea.extent = frameBuffer->getResolution();
    _renderPassBeginInfo.framebuffer       = frameBuffer->getInstance();

    vkCmdBeginRenderPass(executer.commandBuffer, &_renderPassBeginInfo, _subpassContent);
}

void CommandEndRenderPass::execute(const VkCommandBuffer& commandBuffer)
{
    vkCmdEndRenderPass(commandBuffer);
}

void CommandEndRenderPass::execute(const ExecuteCommands& executer)
{
    vkCmdEndRenderPass(executer.commandBuffer);
}

void CommandViewport::execute(const VkCommandBuffer& commandBuffer)
{
    vkCmdSetViewport(commandBuffer, 0, 1, &_viewport);
}

void CommandViewport::execute(const ExecuteCommands& executer)
{
    vkCmdSetViewport(executer.commandBuffer, 0, 1, &_viewport);
}

void CommandScissor::execute(const VkCommandBuffer& commandBuffer)
{
    vkCmdSetScissor(commandBuffer, 0, 1, &_scissor);
}

void CommandScissor::execute(const ExecuteCommands& executer)
{
    vkCmdSetScissor(executer.commandBuffer, 0, 1, &_scissor);
}

CommandPipeline::CommandPipeline(const GraphicsPipeline& pipeline, const VkPipelineBindPoint bindPoint):
_pipeline(pipeline), _bindPoint(bindPoint)
{
    MOUCA_POST_CONDITION(!_pipeline.isNull());
}

void CommandPipeline::execute(const VkCommandBuffer& commandBuffer)
{
    vkCmdBindPipeline(commandBuffer, _bindPoint, _pipeline.getInstance());
}

void CommandPipeline::execute(const ExecuteCommands& executer)
{
    vkCmdBindPipeline(executer.commandBuffer, _bindPoint, _pipeline.getInstance());
}

CommandDraw::CommandDraw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance):
_vertexCount(vertexCount), _instanceCount(instanceCount), _firstVertex(firstVertex), _firstInstance(firstInstance)
{}

void CommandDraw::execute(const VkCommandBuffer& commandBuffer)
{
    vkCmdDraw(commandBuffer, _vertexCount, _instanceCount, _firstVertex, _firstInstance);
}

void CommandDraw::execute(const ExecuteCommands& executer)
{
    vkCmdDraw(executer.commandBuffer, _vertexCount, _instanceCount, _firstVertex, _firstInstance);
}

CommandDrawIndexed::CommandDrawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const int32_t vertexOffset, const uint32_t firstInstance) :
_indexCount(indexCount), _instanceCount(instanceCount), _firstIndex(firstIndex), _vertexOffset(vertexOffset), _firstInstance(firstInstance)
{}

void CommandDrawIndexed::execute(const VkCommandBuffer& commandBuffer)
{
    vkCmdDrawIndexed(commandBuffer, _indexCount, _instanceCount, _firstIndex, _vertexOffset, _firstInstance);
}

void CommandDrawIndexed::execute(const ExecuteCommands& executer)
{
    vkCmdDrawIndexed(executer.commandBuffer, _indexCount, _instanceCount, _firstIndex, _vertexOffset, _firstInstance);
}

CommandBindVertexBuffer::CommandBindVertexBuffer(const uint32_t firstBinding, const uint32_t bindingCount, std::vector<VkBuffer>&& buffers, std::vector<VkDeviceSize>&& offsets):
_firstBinding(firstBinding), _bindingCount(bindingCount), _buffersId(std::move(buffers)), _offsets(std::move(offsets))
{
    MOUCA_PRE_CONDITION(_buffersId.size() == _offsets.size());
}

CommandBindVertexBuffer::CommandBindVertexBuffer(const uint32_t firstBinding, const uint32_t bindingCount, std::vector<Vulkan::BufferWPtr>&& buffer, std::vector<VkDeviceSize>&& offsets):
_firstBinding(firstBinding), _bindingCount(bindingCount), _buffers(std::move(buffer)), _buffersId(_buffers.size()), _offsets(std::move(offsets))
{
    MOUCA_PRE_CONDITION(_buffersId.size() == _offsets.size());
}

void CommandBindVertexBuffer::execute(const VkCommandBuffer& commandBuffer)
{
    vkCmdBindVertexBuffers(commandBuffer, _firstBinding, _bindingCount, _buffersId.data(), _offsets.data());
}

void CommandBindVertexBuffer::execute(const ExecuteCommands& executer)
{
    if(!_buffers.empty())
    {
        auto itId = _buffersId.begin();
        for (auto& buffer : _buffers)
        {
            *itId = buffer.lock()->getBuffer();
            ++itId;
        }
    }
    vkCmdBindVertexBuffers(executer.commandBuffer, _firstBinding, _bindingCount, _buffersId.data(), _offsets.data());
}

CommandBindIndexBuffer::CommandBindIndexBuffer(const BufferWPtr buffer, const VkDeviceSize offset, const VkIndexType indexType) :
_buffer(buffer), _bufferId(buffer.lock()->getBuffer()), _offset(offset), _indexType(indexType)
{}

CommandBindIndexBuffer::CommandBindIndexBuffer(const VkBuffer buffer, const VkDeviceSize offset, const VkIndexType indexType):
_bufferId(buffer), _offset(offset), _indexType(indexType)
{}

void CommandBindIndexBuffer::execute(const VkCommandBuffer& commandBuffer)
{
    vkCmdBindIndexBuffer(commandBuffer, _bufferId, _offset, _indexType);
}

void CommandBindIndexBuffer::execute(const ExecuteCommands& executer)
{
    if (!_buffer.expired())
    {
        _bufferId = _buffer.lock()->getBuffer();
    }

    vkCmdBindIndexBuffer(executer.commandBuffer, _bufferId, _offset, _indexType);
}

CommandBindMesh::CommandBindMesh(const Mesh& mesh, const uint32_t bindID, const VkIndexType index):
_index(index),
_indices(mesh.getIndices().getBuffer()),
_vertices(mesh.getVertices().getBuffer()),
_offsets(0),
_bindID(bindID)
{
    MOUCA_PRE_CONDITION(!mesh.isNull());
}

void CommandBindMesh::execute(const VkCommandBuffer& commandBuffer)
{
    vkCmdBindVertexBuffers(commandBuffer, _bindID, 1, &_vertices, &_offsets);
    vkCmdBindIndexBuffer(commandBuffer, _indices, 0, _index);
}

void CommandBindMesh::execute(const ExecuteCommands& executer)
{
    vkCmdBindVertexBuffers(executer.commandBuffer, _bindID, 1, &_vertices, &_offsets);
    vkCmdBindIndexBuffer(executer.commandBuffer, _indices, 0, _index);
}

CommandDrawMeshIndexed::CommandDrawMeshIndexed(const Mesh& mesh, const uint32_t bindID):
CommandBindMesh(mesh, bindID)
{
    MOUCA_PRE_CONDITION(!mesh.isNull());

    _indexed.reserve(mesh.getIndexed().size());
    for(const auto& index : mesh.getIndexed())
    {
        _indexed.emplace_back(std::make_unique<CommandDrawIndexed>(index._nbIndices, 1, index._startIndex, 0, index._id));
    }
}

void CommandDrawMeshIndexed::execute(const VkCommandBuffer& commandBuffer)
{
    CommandBindMesh::execute(commandBuffer);
    for (const auto& index : _indexed)
    {
        index->execute(commandBuffer);
    }
}

void CommandDrawMeshIndexed::execute(const ExecuteCommands& executer)
{
    CommandBindMesh::execute(executer.commandBuffer);
    for (const auto& index : _indexed)
    {
        index->execute(executer.commandBuffer);
    }
}

CommandDrawMesh::CommandDrawMesh(const Mesh& mesh, const uint32_t bindID, const uint32_t instanceCount):
CommandBindMesh(mesh, bindID),
_indicesCount(static_cast<uint32_t>(mesh.getIndexCount())),
_instanceCount(instanceCount)
{
    BT_ASSERT(!mesh.isNull());
}

void CommandDrawMesh::execute(const VkCommandBuffer& commandBuffer)
{
    CommandBindMesh::execute(commandBuffer);
    vkCmdDrawIndexed(commandBuffer, _indicesCount, _instanceCount, 0, 0, 0);
}

void CommandDrawMesh::execute(const ExecuteCommands& executer)
{
    CommandBindMesh::execute(executer.commandBuffer);
    vkCmdDrawIndexed(executer.commandBuffer, _indicesCount, _instanceCount, 0, 0, 0);
}

CommandDrawLines::CommandDrawLines(const Mesh& mesh, const uint32_t bindID, const uint32_t instanceCount, const float widthLine) :
    CommandBindMesh(mesh, bindID),
    _indicesCount(static_cast<uint32_t>(mesh.getIndexCount())),
    _instanceCount(instanceCount),
    _widthLine(widthLine)
{
    BT_ASSERT(!mesh.isNull());
}

void CommandDrawLines::execute(const VkCommandBuffer& commandBuffer)
{
    CommandBindMesh::execute(commandBuffer);
    if(_widthLine > 0.0f)
    {
        vkCmdSetLineWidth(commandBuffer, _widthLine);
    }
    vkCmdDrawIndexed(commandBuffer, _indicesCount, _instanceCount, 0, 0, 0);
}

void CommandDrawLines::execute(const ExecuteCommands& executer)
{
    CommandBindMesh::execute(executer.commandBuffer);
    if (_widthLine > 0.0f)
    {
        vkCmdSetLineWidth(executer.commandBuffer, _widthLine);
    }
    vkCmdDrawIndexed(executer.commandBuffer, _indicesCount, _instanceCount, 0, 0, 0);
}

CommandBindDescriptorSets::CommandBindDescriptorSets(const PipelineLayout& pipelineLayout, const VkPipelineBindPoint bindPoint, const uint32_t firstSet, const std::vector<VkDescriptorSet>& descriptors, std::vector<uint32_t>&& dynamicOffsets):
_pipelineLayoutID(pipelineLayout.getInstance()), _bindPoint(bindPoint), _firstSet(firstSet), _descriptorsID(std::move(descriptors)), _dynamicOffsets(std::move(dynamicOffsets))
{
    BT_ASSERT(!pipelineLayout.isNull());
}

void CommandBindDescriptorSets::execute(const VkCommandBuffer& commandBuffer)
{
    vkCmdBindDescriptorSets(commandBuffer, _bindPoint, _pipelineLayoutID, _firstSet,
                            static_cast<uint32_t>(_descriptorsID.size()), _descriptorsID.data(),
                            static_cast<uint32_t>(_dynamicOffsets.size()), _dynamicOffsets.empty() ? nullptr : _dynamicOffsets.data());
}

void CommandBindDescriptorSets::execute(const ExecuteCommands& executer)
{
    vkCmdBindDescriptorSets(executer.commandBuffer, _bindPoint, _pipelineLayoutID, _firstSet,
                            static_cast<uint32_t>(_descriptorsID.size()), _descriptorsID.data(),
                            static_cast<uint32_t>(_dynamicOffsets.size()), _dynamicOffsets.empty() ? nullptr : _dynamicOffsets.data());
}

CommandCopyImage::CommandCopyImage(const VkImage& source, const VkImage& destination, const VkImageCopy& copyRegion):
_source(source),
_destination(destination),
_copyRegion(copyRegion)
{
    BT_ASSERT(source != VK_NULL_HANDLE);
    BT_ASSERT(destination != VK_NULL_HANDLE);
}

void CommandCopyImage::execute(const VkCommandBuffer& commandBuffer)
{
    vkCmdCopyImage(commandBuffer, 
                   _source,      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   _destination, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   1, &_copyRegion);
}

void CommandCopyImage::execute(const ExecuteCommands& executer)
{
    vkCmdCopyImage(executer.commandBuffer,
                   _source,      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   _destination, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   1, &_copyRegion);
}

CommandCopy::CommandCopy(const Buffer& source, const Buffer& destination, const VkBufferCopy& copyRegion):
_source(source.getBuffer()),
_destination(destination.getBuffer()),
_copyRegion(copyRegion)
{
    BT_ASSERT(!source.isNull());
    BT_ASSERT(!destination.isNull());
    BT_ASSERT(source.getDescriptor().range >= _copyRegion.size);
    BT_ASSERT(destination.getDescriptor().range >= _copyRegion.size);
}

void CommandCopy::execute(const VkCommandBuffer& commandBuffer)
{
    vkCmdCopyBuffer(commandBuffer, _source, _destination, 1, &_copyRegion);
}

void CommandCopy::execute(const ExecuteCommands& executer)
{
    vkCmdCopyBuffer(executer.commandBuffer, _source, _destination, 1, &_copyRegion);
}

CommandCopyBufferToImage::CommandCopyBufferToImage(const Buffer& source, const Image& destination, std::vector<VkBufferImageCopy>&& copyRegion):
_source(source.getBuffer()),
_destination(destination.getImage()),
_copyRegion(std::move(copyRegion))
{
    MOUCA_PRE_CONDITION(!source.isNull());
    MOUCA_PRE_CONDITION(!destination.isNull());

    MOUCA_POST_CONDITION(_destination != VK_NULL_HANDLE);
    MOUCA_POST_CONDITION(_source != VK_NULL_HANDLE);
    MOUCA_POST_CONDITION(!_copyRegion.empty());
}

void CommandCopyBufferToImage::execute(const VkCommandBuffer& commandBuffer)
{
    vkCmdCopyBufferToImage(commandBuffer, _source, _destination, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(_copyRegion.size()), _copyRegion.data());
}

void CommandCopyBufferToImage::execute(const ExecuteCommands& executer)
{
    vkCmdCopyBufferToImage(executer.commandBuffer, _source, _destination, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(_copyRegion.size()), _copyRegion.data());
}

CommandPipelineBarrier::CommandPipelineBarrier(const VkPipelineStageFlags srcStage, const VkPipelineStageFlags dstStage, const VkDependencyFlags dependencyFlags,
                                               MemoryBarriers&& memoryBarriers, BufferMemoryBarriers&& bufferMemoryBarriers, ImageMemoryBarriers&& imageBarriers) :
_srcStage(srcStage), _dstStage(dstStage), _dependencyFlags(dependencyFlags), _memoryBarriers(std::move(memoryBarriers)), _bufferMemoryBarriers(std::move(bufferMemoryBarriers)), _imageBarriers(std::move(imageBarriers))
{}

void CommandPipelineBarrier::execute(const VkCommandBuffer& commandBuffer)
{
    BT_ASSERT(commandBuffer!=VK_NULL_HANDLE);
    vkCmdPipelineBarrier(commandBuffer, _srcStage, _dstStage, _dependencyFlags,
                         static_cast<uint32_t>(_memoryBarriers.size()),
                         _memoryBarriers.empty()       ? nullptr : _memoryBarriers.data(),
                         static_cast<uint32_t>(_bufferMemoryBarriers.size()),
                         _bufferMemoryBarriers.empty() ? nullptr : _bufferMemoryBarriers.data(),
                         static_cast<uint32_t>(_imageBarriers.size()),
                         _imageBarriers.empty()        ? nullptr : _imageBarriers.data());
}

void CommandPipelineBarrier::execute(const ExecuteCommands& executer)
{
    BT_ASSERT(executer.commandBuffer!=VK_NULL_HANDLE);
    vkCmdPipelineBarrier(executer.commandBuffer, _srcStage, _dstStage, _dependencyFlags,
                         static_cast<uint32_t>(_memoryBarriers.size()),
                         _memoryBarriers.empty()       ? nullptr : _memoryBarriers.data(),
                         static_cast<uint32_t>(_bufferMemoryBarriers.size()),
                         _bufferMemoryBarriers.empty() ? nullptr : _bufferMemoryBarriers.data(),
                         static_cast<uint32_t>(_imageBarriers.size()),
                         _imageBarriers.empty()        ? nullptr : _imageBarriers.data());
}

CommandBlit::CommandBlit(const VkImage& source, const VkImage& destination, const VkImageBlit& region):
_region(region), _source(source), _destination(destination)
{}

void CommandBlit::execute(const VkCommandBuffer& commandBuffer)
{
    // Issue the blit command
    vkCmdBlitImage(
        commandBuffer,
        _source,        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        _destination,   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &_region,
        VK_FILTER_NEAREST);
}

void CommandBlit::execute(const ExecuteCommands& executer)
{
    // Issue the blit command
    vkCmdBlitImage(executer.commandBuffer,
                   _source,        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   _destination,   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   1,
                   &_region,
                   VK_FILTER_NEAREST);
}

CommandPushConstants::CommandPushConstants(const PipelineLayout& pipelineLayout, const VkShaderStageFlags stage, const uint32_t memorySize, const void* buffer):
_pipelineLayout(pipelineLayout.getInstance()), _stage(stage), _memorySize(memorySize), _buffer(buffer)
{
    MOUCA_PRE_CONDITION(!pipelineLayout.isNull());
    MOUCA_PRE_CONDITION(_memorySize > 0 && _buffer != nullptr);
}

void CommandPushConstants::execute(const VkCommandBuffer& commandBuffer)
{
    vkCmdPushConstants(commandBuffer, _pipelineLayout, _stage, 0, _memorySize, _buffer);
}

void CommandPushConstants::execute(const ExecuteCommands& executer)
{
    vkCmdPushConstants(executer.commandBuffer, _pipelineLayout, _stage, 0, _memorySize, _buffer);
}

void CommandContainer::execute(const VkCommandBuffer& commandBuffer)
{
    for (const auto& command : _commands)
    {
        command->execute(commandBuffer);
    }
}

void CommandContainer::execute(const ExecuteCommands& executer)
{
    for (const auto& command : _commands)
    {
        command->execute(executer);
    }
}

CommandSwitch::CommandSwitch():
_idNode(0)
{}

void CommandSwitch::execute(const VkCommandBuffer& commandBuffer)
{
    MOUCA_PRE_CONDITION(_idNode < _commands.size());

    _commands[_idNode]->execute(commandBuffer);
}

void CommandSwitch::execute(const ExecuteCommands& executer)
{
    MOUCA_PRE_CONDITION(_idNode < _commands.size());

    _commands[_idNode]->execute(executer);
}

}