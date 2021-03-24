#include "Dependencies.h"

#include "MouCaGraphicEngine/include/Engine3DTransfer.h"

#include "LibRT/include/RTImage.h"

#include "LibVulkan/include/VKBuffer.h"
#include "LibVulkan/include/VKCommand.h"
#include "LibVulkan/include/VKCommandBuffer.h"
#include "LibVulkan/include/VKContextDevice.h"
#include "LibVulkan/include/VKFence.h"
#include "LibVulkan/include/VKImage.h"
#include "LibVulkan/include/VKSequence.h"
#include "LibVulkan/include/VKSubmitInfo.h"

namespace MouCaGraphic
{

Engine3DTransfer::Engine3DTransfer(const Vulkan::ContextDevice& context):
_context(context)
{
    MOUCA_PRE_CONDITION(!context.isNull());        //DEV Issue: Need a valid context
}

Engine3DTransfer::~Engine3DTransfer()
{
    // Release all tmp buffers
    for(auto& buffer : _indirectCopyBuffers)
    {
        buffer->release(_context.getDevice());
    }
    _indirectCopyBuffers.clear();
}

void Engine3DTransfer::immediatCopyCPUToGPU(const RT::BufferCPUBase& from, Vulkan::Buffer& to)
{
    MOUCA_PRE_CONDITION(from.getByteSize() > 0);   //DEV Issue: Need allocated buffer
    MOUCA_PRE_CONDITION(!to.isNull());             //DEV Issue: Need allocated buffer

    to.getMemory().copy(_context.getDevice(), from.getByteSize(), from.getData());
}

void Engine3DTransfer::indirectCopyCPUToGPU(Vulkan::CommandBufferWPtr commandBuffer, const RT::BufferCPUBase& from, Vulkan::Buffer& to)
{
    auto transferCPUGPU = std::make_unique<Vulkan::Buffer>(std::make_unique<Vulkan::MemoryBuffer>(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
    transferCPUGPU->initialize(_context.getDevice(), 0, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                               from.getByteSize(), from.getData());

    const VkBufferCopy copyInfo
    {
        0,
        0,
        from.getByteSize()
    };

    commandBuffer.lock()->addCommand(std::make_unique<Vulkan::CommandCopy>(*transferCPUGPU, to, copyInfo));

    _indirectCopyBuffers.emplace_back(std::move(transferCPUGPU));
}

void Engine3DTransfer::transfer(Vulkan::CommandBufferWPtr commandBuffer) const
{
    // Prepare commands
    commandBuffer.lock()->execute();

    // Build submit data
    Vulkan::SubmitInfos infos;
    infos.resize(1);
    infos[0] = std::make_unique<Vulkan::SubmitInfo>();
    std::vector<Vulkan::ICommandBufferWPtr> commandBuffers
    {
        commandBuffer
    };
    infos[0]->initialize(std::move(commandBuffers));

    // Build Fence
    auto fence = std::make_shared<Vulkan::Fence>();
    fence->initialize(_context.getDevice(), Vulkan::Fence::infinityTimeout, 0);

    // Execute sequence
    {
        // Submit
        Vulkan::SequenceSubmit submit(std::move(infos), fence);
        submit.execute(_context.getDevice());

        // Wait fence
        const std::vector<Vulkan::FenceWPtr> fences{ fence };
        Vulkan::SequenceWaitFence waitFence(fences, Vulkan::Fence::infinityTimeout, VK_TRUE);
        waitFence.execute(_context.getDevice());

        // Sync device
        _context.getDevice().waitIdle();
    }

    fence->release(_context.getDevice());
}

void Engine3DTransfer::indirectCopyCPUToGPU(Vulkan::CommandBufferWPtr commandBuffer, const RT::Image& from, Vulkan::Image& to)
{
    if (from.getLayers() > 1)
    {
        image2DArrayCPUToGPU(*commandBuffer.lock(), from, to);
    }
    else
    {
        image2DCPUToGPU(*commandBuffer.lock(), from, to);
    }
}

void Engine3DTransfer::image2DArrayCPUToGPU(Vulkan::CommandBuffer& commandBuffer, const RT::Image& from, Vulkan::Image& to)
{
    MOUCA_PRE_CONDITION(!commandBuffer.isNull());

    const auto& device = _context.getDevice();

    const auto& size = to.getExtent();
    MOUCA_PRE_CONDITION(size.width        == static_cast<uint32_t>(from.getExtents(0).x));
    MOUCA_PRE_CONDITION(size.height       == static_cast<uint32_t>(from.getExtents(0).y));
    MOUCA_PRE_CONDITION(to.getArraySize() == static_cast<uint32_t>(from.getLayers()));

    //Create a buffer with data
    auto stagingBuffer = std::make_unique<Vulkan::Buffer>(std::make_unique<Vulkan::MemoryBuffer>(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
    stagingBuffer->initialize(device, 0, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, from.getMemorySize(), from.getRAWData(0, 0));

    // Setup buffer copy regions for each mip level
    std::vector<VkBufferImageCopy> bufferCopyRegions;
    bufferCopyRegions.reserve(from.getLevels() * from.getLayers());

    for (uint32_t layer = 0; layer < from.getLayers(); layer++)
    {
        for (uint32_t mipLevel = 0; mipLevel < from.getLevels(); mipLevel++)
        {
            const VkBufferImageCopy bufferCopyRegion =
            {
                from.getMemoryOffset(layer, mipLevel),  // VkDeviceSize                bufferOffset;
                0,                                      // uint32_t                    bufferRowLength;
                0,                                      // uint32_t                    bufferImageHeight;
                {
                    VK_IMAGE_ASPECT_COLOR_BIT,          // VkImageAspectFlags          aspectMask;
                    mipLevel,                           // uint32_t                    mipLevel;
                    layer,                              // uint32_t                    baseArrayLayer;
                    1                                   // uint32_t                    layerCount;
                },                                      // VkImageSubresourceLayers    imageSubresource;
                { 0, 0, 0 },                            // VkOffset3D                  imageOffset;
                {                                                                      
                    from.getExtents(mipLevel).x,
                    from.getExtents(mipLevel).y,
                    1
                }                                       // VkExtent3D                  imageExtent;
            };
            bufferCopyRegions.emplace_back(bufferCopyRegion);
        }
    }

    VkImageSubresourceRange subresourceRange =
    {
        VK_IMAGE_ASPECT_COLOR_BIT,                  //VkImageAspectFlags    aspectMask;
        0,                                          //uint32_t              baseMipLevel;
        static_cast<uint32_t>(from.getLevels()),    //uint32_t              levelCount;
        0,                                          //uint32_t              baseArrayLayer;
        static_cast<uint32_t>(from.getLayers())     //uint32_t              layerCount;
    };

    // Image barrier for optimal image (target)
    // Optimal image will be used as destination for the copy
    Vulkan::CommandPipelineBarrier::ImageMemoryBarriers barrierCopy
    {
        {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr,
            0, VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
            to.getImage(), subresourceRange
        }
    };
    commandBuffer.addCommand(std::make_unique<Vulkan::CommandPipelineBarrier>(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_DEPENDENCY_BY_REGION_BIT,
                                                                               Vulkan::CommandPipelineBarrier::MemoryBarriers(), Vulkan::CommandPipelineBarrier::BufferMemoryBarriers(), std::move(barrierCopy)));

    // Copy mip levels from staging buffer
    commandBuffer.addCommand(std::make_unique<Vulkan::CommandCopyBufferToImage>(*stagingBuffer, to, std::move(bufferCopyRegions)));

    // Change texture image layout to shader read after all mip levels have been copied
    Vulkan::CommandPipelineBarrier::ImageMemoryBarriers barrierSync
    {
        {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr,
            VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
            to.getImage(), subresourceRange
        }
    };
    commandBuffer.addCommand(std::make_unique<Vulkan::CommandPipelineBarrier>(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_DEPENDENCY_BY_REGION_BIT,
                                                      Vulkan::CommandPipelineBarrier::MemoryBarriers(), Vulkan::CommandPipelineBarrier::BufferMemoryBarriers(), std::move(barrierSync)));

    // Keep buffer until done/clean
    _indirectCopyBuffers.emplace_back(std::move(stagingBuffer));
}

void Engine3DTransfer::image2DCPUToGPU(Vulkan::CommandBuffer& commandBuffer, const RT::Image& from, Vulkan::Image& to)
{
    MOUCA_PRE_CONDITION(!commandBuffer.isNull());

    const auto& device = _context.getDevice();

    MOUCA_PRE_CONDITION(to.getExtent().width  == from.getExtents(0).x);
    MOUCA_PRE_CONDITION(to.getExtent().height == from.getExtents(0).y);

    //Create a buffer with data
    auto stagingBuffer = std::make_unique<Vulkan::Buffer>(std::make_unique<Vulkan::MemoryBuffer>(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
    stagingBuffer->initialize(device, 0, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, from.getMemorySize(), from.getRAWData(0, 0));

    // Setup buffer copy regions for each mip level
    std::vector<VkBufferImageCopy> bufferCopyRegions;
    bufferCopyRegions.reserve(from.getLevels() * from.getLayers());

    const uint32_t layer = 0;
    for (uint32_t mipLevel = 0; mipLevel < from.getLevels(); mipLevel++)
    {
        const VkBufferImageCopy bufferCopyRegion =
        {
            from.getMemoryOffset(layer, mipLevel),  // VkDeviceSize                bufferOffset;
            0,                                      // uint32_t                    bufferRowLength;
            0,                                      // uint32_t                    bufferImageHeight;
            {
                VK_IMAGE_ASPECT_COLOR_BIT,          // VkImageAspectFlags          aspectMask;
                mipLevel,                           // uint32_t                    mipLevel;
                layer,                              // uint32_t                    baseArrayLayer;
                1                                   // uint32_t                    layerCount;
            },                                      // VkImageSubresourceLayers    imageSubresource;
            { 0, 0, 0 },                            // VkOffset3D                  imageOffset;
            {
                from.getExtents(mipLevel).x,
                from.getExtents(mipLevel).y,
                1
            }                                       // VkExtent3D                  imageExtent;
        };
        bufferCopyRegions.emplace_back(bufferCopyRegion);
    }

    VkImageSubresourceRange subresourceRange =
    {
        VK_IMAGE_ASPECT_COLOR_BIT,                  //VkImageAspectFlags    aspectMask;
        0,                                          //uint32_t              baseMipLevel;
        static_cast<uint32_t>(from.getLevels()),    //uint32_t              levelCount;
        0,                                          //uint32_t              baseArrayLayer;
        static_cast<uint32_t>(from.getLayers())     //uint32_t              layerCount;
    };

    // Image barrier for optimal image (target)
    // Optimal image will be used as destination for the copy
    Vulkan::CommandPipelineBarrier::ImageMemoryBarriers barrierCopy
    {
        {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr,
            0, VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
            to.getImage(), subresourceRange
        }
    };

    commandBuffer.addCommand(std::make_unique<Vulkan::CommandPipelineBarrier>(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_DEPENDENCY_BY_REGION_BIT,
                                                      Vulkan::CommandPipelineBarrier::MemoryBarriers(), Vulkan::CommandPipelineBarrier::BufferMemoryBarriers(), std::move(barrierCopy)));

    // Copy mip levels from staging buffer
    commandBuffer.addCommand(std::make_unique<Vulkan::CommandCopyBufferToImage>(*stagingBuffer, to, std::move(bufferCopyRegions)));

    // Change texture image layout to shader read after all mip levels have been copied
    Vulkan::CommandPipelineBarrier::ImageMemoryBarriers barrierSync
    {
        {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr,
            VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
            to.getImage(), subresourceRange
        }
    };
    commandBuffer.addCommand(std::make_unique<Vulkan::CommandPipelineBarrier>(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_DEPENDENCY_BY_REGION_BIT,
                                                                              Vulkan::CommandPipelineBarrier::MemoryBarriers(), Vulkan::CommandPipelineBarrier::BufferMemoryBarriers(), std::move(barrierSync)));

    // Keep buffer until done/clean
    _indirectCopyBuffers.emplace_back(std::move(stagingBuffer));
}

}