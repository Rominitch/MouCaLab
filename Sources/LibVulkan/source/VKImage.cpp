/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibRT/include/RTBufferCPU.h"
#include "LibRT/include/RTImage.h"

#include "LibVulkan/include/VKDevice.h"
#include "LibVulkan/include/VKCommand.h"
#include "LibVulkan/include/VKCommandBuffer.h"
#include "LibVulkan/include/VKContextDevice.h"
#include "LibVulkan/include/VKFence.h"
#include "LibVulkan/include/VKImage.h"
#include <LibVulkan/include/VKSampler.h>

namespace Vulkan
{

Image::Image() :
_image(VK_NULL_HANDLE), _imageCreateInfo({})
{
    MOUCA_PRE_CONDITION(isNull());
}

Image::~Image()
{
    MOUCA_POST_CONDITION(isNull()); ///DEV Issue: you call initialize() but never release() !
}

void Image::initialize(const Device& device, 
                       const Size& size,
                       const VkImageType imageType, const VkFormat format,
                       const VkSampleCountFlagBits sampleCount, 
                       const VkImageTiling tiling,                       
                       const VkImageUsageFlags imageUsageFlags,
                       const VkSharingMode sharingMode,
                       const VkImageLayout initialLayout,
                       const VkMemoryPropertyFlags memoryProperty)
{
    MOUCA_PRE_CONDITION(isNull());
    MOUCA_PRE_CONDITION(!device.isNull());
    MOUCA_PRE_CONDITION(size.isValid());

    _imageCreateInfo =
    {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,    // VkStructureType          sType;
        nullptr,                                // const void*              pNext;
        0,                                      // VkImageCreateFlags       flags;
        imageType,                              // VkImageType              imageType;
        format,                                 // VkFormat                 format;
        size._extent,                           // VkExtent3D               extent;
        size._mipLevels,                        // uint32_t                 mipLevels;
        size._arrayLayers,                      // uint32_t                 arrayLayers;
        sampleCount,                            // VkSampleCountFlagBits    samples;
        tiling,                                 // VkImageTiling            tiling;
        imageUsageFlags,                        // VkImageUsageFlags        usage;
        sharingMode,                            // VkSharingMode            sharingMode;
        0,                                      // uint32_t                 queueFamilyIndexCount;
        nullptr,                                // const uint32_t*          pQueueFamilyIndices;
        initialLayout                           // VkImageLayout            initialLayout;
    };

    // Create image
    if(vkCreateImage(device.getInstance(), &_imageCreateInfo, nullptr, &_image) != VK_SUCCESS)
    {
        MOUCA_THROW_ERROR(u8"Vulkan", u8"ViewCreationError");
    }

    // Allocate memory (MANDATORY to create view)
    _memory.initialize(device, _image, memoryProperty);

    MOUCA_POST_CONDITION(!isNull());
}

void Image::release(const Device& device, const bool removeView)
{
    MOUCA_PRE_CONDITION(!isNull());
    MOUCA_PRE_CONDITION(!device.isNull());

    // Remove all views
    for(auto& view : _views)
    {
        view->release(device);
    }
    // Lost all link
    if (removeView)
    {
        _views.clear();
    }
    
    // Release memory
    _memory.release(device);
    // Release image
    vkDestroyImage(device.getInstance(), _image, nullptr);
    _image = VK_NULL_HANDLE;

    MOUCA_POST_CONDITION(isNull());
}

void Image::readSubresourceLayout(const Device& device, const VkImageSubresource& subResource, VkSubresourceLayout& subResourceLayout) const
{
    MOUCA_PRE_CONDITION(!isNull());

    vkGetImageSubresourceLayout(device.getInstance(), _image, &subResource, &subResourceLayout);
}

void Image::createSampler(const Device& device, Sampler& sampler) const
{
    MOUCA_PRE_CONDITION(!isNull());
    MOUCA_PRE_CONDITION(!device.isNull());

    sampler.initialize(device, static_cast<float>(_imageCreateInfo.mipLevels));
}

ImageViewWPtr Image::createView(const Device& device, const VkImageViewType viewType, const VkFormat format, const VkComponentMapping& components, const VkImageSubresourceRange& subresourceRange)
{
    MOUCA_PRE_CONDITION(!device.isNull());

    // Build + initialize
    auto view = std::make_shared<ImageView>();
    view->initialize(device, *this, viewType, format, components, subresourceRange);

    // Register
    _views.emplace_back(view);
    return view;
}

void Image::removeView(const Device& device, ImageViewWPtr view)
{
    MOUCA_PRE_CONDITION(!device.isNull());
    MOUCA_PRE_CONDITION(!view.expired());

    // Search view into list
    auto itView = std::find_if(_views.begin(), _views.end(),
                               [&](const auto& currentView)
                               { return currentView.get() == view.lock().get(); });
    MOUCA_ASSERT(itView != _views.end()); ///DEV Issue: Not existing ! Already delete ? Bad Image ?

    // Release
    (*itView)->release(device);
    // Unregister
    _views.erase(itView);
}

void Image::resize(const Device& device, const VkExtent3D& newSize)
{
    MOUCA_PRE_CONDITION(!isNull());

    // Clean Vulkan memory
    release(device, false);
    MOUCA_ASSERT(isNull());

    // Change size
    _imageCreateInfo.extent = newSize;

    // Create image
    if (vkCreateImage(device.getInstance(), &_imageCreateInfo, nullptr, &_image) != VK_SUCCESS)
    {
        MOUCA_THROW_ERROR(u8"Vulkan", u8"ViewCreationError");
    }

    // Allocate memory (MANDATORY to create view)
    _memory.initialize(device, _image, _memory.getFlags());
    MOUCA_ASSERT(!isNull());

    // Reallocation of view
    for (auto& view : _views)
    {
        view->initialize(device, *this, view->getInfo().viewType, view->getInfo().format, view->getInfo().components, view->getInfo().subresourceRange);
        MOUCA_ASSERT(!view->isNull());
    }

    MOUCA_POST_CONDITION(!isNull());
}

ImageView::ImageView():
_view(VK_NULL_HANDLE), _image(nullptr)
{
    MOUCA_PRE_CONDITION(isNull());
}

ImageView::~ImageView()
{
    MOUCA_POST_CONDITION(isNull());
}

void ImageView::initialize(const Device& device, const Image& image,
                           const VkImageViewType viewType, const VkFormat format,
                           const VkComponentMapping& components,
                           const VkImageSubresourceRange& subresourceRange)
{
    MOUCA_PRE_CONDITION(isNull());
    MOUCA_PRE_CONDITION(!device.isNull());
    MOUCA_PRE_CONDITION(!image.isNull());

    _viewInfo = 
    {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,   // VkStructureType            sType;
        nullptr,                                    // const void*                pNext;
        0,                                          // VkImageViewCreateFlags     flags;
        image.getImage(),                           // VkImage                    image;
        viewType,                                   // VkImageViewType            viewType;
        (format == VK_FORMAT_UNDEFINED) ? image.getFormat() : format, // VkFormat                   format;
        components,                                 // VkComponentMapping         components;
        subresourceRange                            // VkImageSubresourceRange    subresourceRange;
    };
    //Generate View
    if (vkCreateImageView(device.getInstance(), &_viewInfo, nullptr, &_view) != VK_SUCCESS)
    {
        MOUCA_THROW_ERROR(u8"Vulkan", u8"ViewCreationError");
    }

    _image = &image;
}

void ImageView::release(const Device& device)
{
    MOUCA_PRE_CONDITION(!isNull());
    MOUCA_PRE_CONDITION(!device.isNull());

    //Release View
    vkDestroyImageView(device.getInstance(), _view, nullptr);
    _view = VK_NULL_HANDLE;

    MOUCA_POST_CONDITION(isNull());
}

}