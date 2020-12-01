/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibVulkan/include/VKScreenshot.h"

#include "LibRT/include/RTBufferCPU.h"
#include "LibRT/include/RTImage.h"

#include "LibVulkan/include/VKContextDevice.h"
#include "LibVulkan/include/VKContextWindow.h"
#include "LibVulkan/include/VKCommand.h"
#include "LibVulkan/include/VKCommandBuffer.h"
#include "LibVulkan/include/VKFence.h"
#include "LibVulkan/include/VKSequence.h"
#include "LibVulkan/include/VKSubmitInfo.h"
#include "LibVulkan/include/VKSurfaceFormat.h"

namespace Vulkan
{

void GPUImageReader::initialize(const ContextWindow& context, const RT::ComponentDescriptor& component)
{
    // Get format properties for the swapchain color format
    VkFormatProperties formatProps;

    // Check blit support for source and destination
    _supportsBlit = true;

    VkFormat imageFormat = VK_FORMAT_UNDEFINED;
    if (component.getNbComponents() == 4 && component.getFormatType() == RT::Type::UnsignedChar)
    {
        imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
    }
    else if (component.getNbComponents() == 1 && component.getFormatType() == RT::Type::Int)
    {
        imageFormat = VK_FORMAT_R32_SINT;
    }
    MOUCA_ASSERT(imageFormat != VK_FORMAT_UNDEFINED); //DEV Issue: unsupported format.
    
    const auto& device = context.getContextDevice().getDevice();

    // Check if the device supports blitting from optimal images (the swapchain images are in optimal format)
    device.readFormatProperties(context.getFormat().getConfiguration()._format.format, formatProps);
    if (!(formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT))
    {
        _supportsBlit = false;
        //MOUCA_THROW_ERROR("Vulkan", "blitSourceError");
    }
    else
    {
        // Check if the device supports blitting to linear images
        device.readFormatProperties(imageFormat, formatProps);
        if (!(formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT))
        {
            _supportsBlit = false;
            //MOUCA_THROW_ERROR("Vulkan", "blitDestinationError");
        }
    }

    // Source for the copy is the last rendered swapchain image
    Image::Size size;
    size._extent = 
    {
        context.getFormat().getConfiguration()._extent.width,
        context.getFormat().getConfiguration()._extent.height,
        1
    };

    // Descriptor must be equal to imageFormat
    _descriptor.addDescriptor(component);

    // Create local image
    _image.initialize(device, size, VK_IMAGE_TYPE_2D, imageFormat,
                      VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                      VK_SHARING_MODE_EXCLUSIVE, VK_IMAGE_LAYOUT_UNDEFINED,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    _fenceSync.initialize(device, 16000000000ull, 0);
}

void GPUImageReader::release(const ContextWindow& context)
{
    const auto& device = context.getContextDevice().getDevice();

    // Clean local image
    _image.release(device);
    _fenceSync.release(device);
}

void GPUImageReader::extractTo(const VkImage& srcImage, const ContextDevice& contextDevice, const RT::Array3ui& positionSrc, const RT::Array3ui& positionDst, const RT::Array3ui& sizes)
{
    MOUCA_PRE_CONDITION(!contextDevice.isNull());  //DEV Issue: Need a valid context.

    const auto& device = contextDevice.getDevice();
    auto commandBuffer = std::make_shared<CommandBuffer>();
    auto pool = std::make_shared<CommandPool>();
    pool->initialize(device, device.getQueueFamilyGraphicId());
    commandBuffer->initialize(device, pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 0);

    Commands commands;
    // Do the actual blit from the swapchain image to our host visible destination image
    CommandPipelineBarrier::ImageMemoryBarriers barrierDst
    {
        {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,     //VkStructureType            sType;
            nullptr,                                    //const void*                pNext;
            0,                                          //VkAccessFlags              srcAccessMask;
            VK_ACCESS_TRANSFER_WRITE_BIT,               //VkAccessFlags              dstAccessMask;
            VK_IMAGE_LAYOUT_UNDEFINED,                  //VkImageLayout              oldLayout;
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,       //VkImageLayout              newLayout;
            VK_QUEUE_FAMILY_IGNORED,                    //uint32_t                   srcQueueFamilyIndex;
            VK_QUEUE_FAMILY_IGNORED,                    //uint32_t                   dstQueueFamilyIndex;
            _image.getImage(),                          //VkImage                    image;
            { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }   //VkImageSubresourceRange    subresourceRange;
        }
    };
    commands.emplace_back(std::make_unique<CommandPipelineBarrier>(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
        CommandPipelineBarrier::MemoryBarriers(), CommandPipelineBarrier::BufferMemoryBarriers(), std::move(barrierDst)));

    CommandPipelineBarrier::ImageMemoryBarriers barrierSrc
    {
        {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,     //VkStructureType            sType;
            nullptr,                                    //const void*                pNext;
            VK_ACCESS_MEMORY_READ_BIT,                  //VkAccessFlags              srcAccessMask;
            VK_ACCESS_TRANSFER_READ_BIT,                //VkAccessFlags              dstAccessMask;
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,            //VkImageLayout              oldLayout;
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,       //VkImageLayout              newLayout;
            VK_QUEUE_FAMILY_IGNORED,                    //uint32_t                   srcQueueFamilyIndex;
            VK_QUEUE_FAMILY_IGNORED,                    //uint32_t                   dstQueueFamilyIndex;
            srcImage,                                   //VkImage                    image;
            { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }   //VkImageSubresourceRange    subresourceRange;
        }
    };
    commands.emplace_back(std::make_unique<CommandPipelineBarrier>(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
        CommandPipelineBarrier::MemoryBarriers(), CommandPipelineBarrier::BufferMemoryBarriers(), std::move(barrierSrc)));

    //     // Define the region to blit (we will blit the whole swapchain image)
    //     const VkOffset3D offset{ static_cast<int32_t>(sizes.x), static_cast<int32_t>(sizes.y), static_cast<int32_t>(sizes.z) };
    //     const VkImageBlit imageBlitRegion
    //     {
    //         { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 }, // VkImageSubresourceLayers    srcSubresource;
    //         { { 0, 0, 0 }, offset },                // VkOffset3D                  srcOffsets[2];
    //         { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 }, // VkImageSubresourceLayers    dstSubresource;
    //         { { 0, 0, 0 }, offset }                 // VkOffset3D                  dstOffsets[2];
    //     };
    //     CommandBlit blit(srcImage, _image, imageBlitRegion);

        // Otherwise use image copy (requires us to manually flip components)
    const VkImageCopy imageCopyRegion
    {
        { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
        { static_cast<int32_t>(positionSrc.x), static_cast<int32_t>(positionSrc.y), static_cast<int32_t>(positionSrc.z) },
        { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
        { static_cast<int32_t>(positionDst.x), static_cast<int32_t>(positionDst.y), static_cast<int32_t>(positionDst.z) },
        { sizes.x, sizes.y, sizes.z }
    };
    commands.emplace_back(std::make_unique<CommandCopyImage>(srcImage, _image.getImage(), imageCopyRegion));

    //Command* copyCommand = &copyImage;

    //     if (_supportsBlit)
    //         copyCommand = &blit;

        // Transition destination image to general layout, which is the required layout for mapping the image memory later on
    CommandPipelineBarrier::ImageMemoryBarriers barrierTransitionDst
    {
        {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,     //VkStructureType            sType;
            nullptr,                                    //const void*                pNext;
            VK_ACCESS_TRANSFER_WRITE_BIT,               //VkAccessFlags              srcAccessMask;
            VK_ACCESS_MEMORY_READ_BIT,                  //VkAccessFlags              dstAccessMask;
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,       //VkImageLayout              oldLayout;
            VK_IMAGE_LAYOUT_GENERAL,                    //VkImageLayout              newLayout;
            VK_QUEUE_FAMILY_IGNORED,                    //uint32_t                   srcQueueFamilyIndex;
            VK_QUEUE_FAMILY_IGNORED,                    //uint32_t                   dstQueueFamilyIndex;
            _image.getImage(),                          //VkImage                    image;
            { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }   //VkImageSubresourceRange    subresourceRange;
        }
    };
    commands.emplace_back(std::make_unique<CommandPipelineBarrier>(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
        CommandPipelineBarrier::MemoryBarriers(), CommandPipelineBarrier::BufferMemoryBarriers(), std::move(barrierTransitionDst)));

    // Transition back the swap chain image after the blit is done
    CommandPipelineBarrier::ImageMemoryBarriers barrierRestoreSrc
    {
        {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,     //VkStructureType            sType;
            nullptr,                                    //const void*                pNext;
            VK_ACCESS_TRANSFER_READ_BIT,                //VkAccessFlags              srcAccessMask;
            VK_ACCESS_MEMORY_READ_BIT,                  //VkAccessFlags              dstAccessMask;
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,       //VkImageLayout              oldLayout;
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,            //VkImageLayout              newLayout;
            VK_QUEUE_FAMILY_IGNORED,                    //uint32_t                   srcQueueFamilyIndex;
            VK_QUEUE_FAMILY_IGNORED,                    //uint32_t                   dstQueueFamilyIndex;
            srcImage,                                   //VkImage                    image;
            { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }   //VkImageSubresourceRange    subresourceRange;
        }
    };
    commands.emplace_back(std::make_unique<CommandPipelineBarrier>(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
        CommandPipelineBarrier::MemoryBarriers(), CommandPipelineBarrier::BufferMemoryBarriers(), std::move(barrierRestoreSrc)));

    commandBuffer->registerCommands(std::move(commands));
    commandBuffer->execute();

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
    fence->initialize(device, Vulkan::Fence::infinityTimeout, 0);

    // Execute sequence
    {
        // Submit
        Vulkan::SequenceSubmit submit(std::move(infos), fence);
        submit.execute(device);

        // Wait fence
        const std::vector<Vulkan::FenceWPtr> fences{ fence };
        Vulkan::SequenceWaitFence waitFence(fences, Vulkan::Fence::infinityTimeout, VK_TRUE);
        waitFence.execute(device);

        // Sync device
        device.waitIdle();
    }

    fence->release(device);
    commandBuffer->release(device);
    pool->release(device);
}

void GPUImageReader::extractTo(const VkImage& srcImage, const ContextWindow& context, const RT::Array3ui& positionSrc, const RT::Array3ui& positionDst, const RT::Array3ui& sizes, RT::Image& diskImage)
{
    const auto& contextDevice = context.getContextDevice();
    MOUCA_PRE_CONDITION(!contextDevice.isNull());          //DEV Issue: Need a valid context.
    MOUCA_PRE_CONDITION(diskImage.isNull());               //DEV Issue: 

    extractTo(srcImage, contextDevice, positionSrc, positionDst, sizes);

    const auto& device = contextDevice.getDevice();
    // Get layout of the image (including row pitch)
    const VkImageSubresource subResource
    { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
    VkSubresourceLayout subResourceLayout;
    _image.readSubresourceLayout(device, subResource, subResourceLayout);

    // Map image memory so we can start copying from it
    _image.getMemory().map(device);
    uint8_t* data = _image.getMemory().getMappedMemory<uint8_t>();
    MOUCA_ASSERT(data != nullptr);
    data += subResourceLayout.offset;

    // Copy to image buffer CPU
    RT::BufferLinkedCPU buffer(u8"linkedScreenshot");
    const size_t memorySize = static_cast<size_t>(sizes.x) * static_cast<size_t>(sizes.y);
    buffer.create(_descriptor, memorySize, data, subResourceLayout.rowPitch);

    diskImage.createFill(buffer, sizes.x, sizes.y);

    _image.getMemory().unmap(device);
}

void GPUImageReader::extractTo(const VkImage& srcImage, const ContextDevice& contextDevice, const RT::Array3ui& positionSrc, const RT::Array3ui& positionDst, const RT::Array3ui& sizes, RT::BufferCPU& output)
{
    MOUCA_PRE_CONDITION(!contextDevice.isNull());          //DEV Issue: Need a valid context.
    MOUCA_PRE_CONDITION(output.getData() != nullptr);      //DEV Issue: 

    extractTo(srcImage, contextDevice, positionSrc, positionDst, sizes);
    const auto& device = contextDevice.getDevice();

    // Get layout of the image (including row pitch)
    const VkImageSubresource subResource
    { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
    VkSubresourceLayout subResourceLayout;
    _image.readSubresourceLayout(device, subResource, subResourceLayout);

    // Map image memory so we can start copying from it
    _image.getMemory().map(device);
    char* data = _image.getMemory().getMappedMemory<char>();
    MOUCA_ASSERT(data != nullptr);
    data += subResourceLayout.offset;

    int32_t* widgetID = reinterpret_cast<int32_t*>(data);
    int32_t* dst = reinterpret_cast<int32_t*>(output.lock());

    // Remove padding
    for (uint64_t y = 0; y < sizes.y; ++y)
    {
        for (uint64_t x = 0; x < sizes.x; ++x)
        {
            const uint64_t idxSrc = y * (subResourceLayout.rowPitch / _descriptor.getByteSize()) + x;
            const uint64_t idxDst = y * sizes.x + x;
            dst[idxDst] = widgetID[idxSrc];
        }
    }
    output.unlock();

    _image.getMemory().unmap(device);
}

}