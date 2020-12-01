/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibVulkan/include/VKDevice.h"
#include "LibVulkan/include/VKSequence.h"
#include "LibVulkan/include/VKSurface.h"
#include "LibVulkan/include/VKSurfaceFormat.h"
#include "LibVulkan/include/VKSwapChain.h"

namespace Vulkan
{

SwapChain::SwapChain():
_swapChain(VK_NULL_HANDLE), _currentImage(0), _ready(false)
{
    MOUCA_PRE_CONDITION(isNull());
}

SwapChain::~SwapChain()
{
    MOUCA_PRE_CONDITION(isNull());
}

void SwapChain::initialize(const Device& device, const Surface& surface, const SurfaceFormat& format)
{
    MOUCA_PRE_CONDITION(isNull());
    MOUCA_PRE_CONDITION(!device.isNull());
    MOUCA_PRE_CONDITION(!surface.isNull());

    const SurfaceFormat::Configuration& configuration = format.getConfiguration();
    MOUCA_ASSERT(format.isSupported());

    // Mandatory for screenshot
    const VkImageUsageFlags imageUsage = static_cast<VkImageUsageFlags>( configuration._usage | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    VkSwapchainCreateInfoKHR swapChainCreateInfo =
    {
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,	// VkStructureType                sType
        nullptr,										// const void                    *pNext
        0,												// VkSwapchainCreateFlagsKHR      flags
        surface.getInstance(),							// VkSurfaceKHR                   surface
        configuration._nbImages,						// uint32_t                       minImageCount
        configuration._format.format,					// VkFormat                       imageFormat
        configuration._format.colorSpace,				// VkColorSpaceKHR                imageColorSpace
        configuration._extent,							// VkExtent2D                     imageExtent
        1,												// uint32_t                       imageArrayLayers
        imageUsage,							            // VkImageUsageFlags              imageUsage
        VK_SHARING_MODE_EXCLUSIVE,						// VkSharingMode                  imageSharingMode
        0,												// uint32_t                       queueFamilyIndexCount
        nullptr,										// const uint32_t                *pQueueFamilyIndices
        configuration._transform,						// VkSurfaceTransformFlagBitsKHR  preTransform
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,				// VkCompositeAlphaFlagBitsKHR    compositeAlpha
        configuration._presentMode,					    // VkPresentModeKHR               presentMode
        VK_TRUE,										// VkBool32                       clipped
        _swapChain										// VkSwapchainKHR                 oldSwapchain
    };

    VkResult result = vkCreateSwapchainKHR(device.getInstance(), &swapChainCreateInfo, nullptr, &_swapChain);
    if(result != VK_SUCCESS)
    {
        MOUCA_THROW_ERROR("Vulkan", "SwapChainError");
    }

    MOUCA_POST_CONDITION(!isNull());
}

void SwapChain::generateImages(const Device& device, const SurfaceFormat& format)
{
    MOUCA_PRE_CONDITION(!isNull());
    MOUCA_PRE_CONDITION(!device.isNull());
    MOUCA_PRE_CONDITION(_images.empty());

    const SurfaceFormat::Configuration& configuration = format.getConfiguration();
    MOUCA_ASSERT(format.isSupported());

    //Create images
    uint32_t imageCount = 0;
    if(vkGetSwapchainImagesKHR(device.getInstance(), _swapChain, &imageCount, nullptr) != VK_SUCCESS)
    {
        MOUCA_THROW_ERROR(u8"Vulkan", u8"LinkCommandBufferToSwapChainError");
    }

    std::vector<VkImage> swapChainImages;
    swapChainImages.resize(static_cast<size_t>(imageCount));
    if(vkGetSwapchainImagesKHR(device.getInstance(), _swapChain, &imageCount, swapChainImages.data()) != VK_SUCCESS)
    {
        MOUCA_THROW_ERROR(u8"Vulkan", u8"LinkCommandBufferToSwapChainError");
    }

    _images.resize(swapChainImages.size());
    auto itImage = _images.begin();
    for(auto& image : swapChainImages)
    {
        itImage->initialize(device, configuration._format.format, image);
        ++itImage;
    }

    // refresh attached sequence
    for(auto& sequence : _linkSequences)
    {
        sequence.lock()->update();
    }

    // Set ready to used
    _ready = true;
}

void SwapChain::release(const Device& device)
{
    MOUCA_PRE_CONDITION(!isNull());
    MOUCA_PRE_CONDITION(!device.isNull());
    
    /*
#ifdef _DEBUG
    //DEV Issue: defined sequences but not anymore valid swapchain ???
    for(auto& sequence : _linkSequences)
    {
        MOUCA_ASSERT(!sequence.expired());
    }
#endif
    _linkSequences.clear();
    */

    for(auto& image : _images)
    {
        image.release(device);
    }
    _images.clear();

    vkDestroySwapchainKHR(device.getInstance(), _swapChain, nullptr);
    _swapChain = VK_NULL_HANDLE;

    _ready = false;

    MOUCA_POST_CONDITION(isNull());
}


void SwapChain::registerSequence(SequenceWPtr sequence)
{
    MOUCA_PRE_CONDITION(std::find_if(_linkSequences.cbegin(), _linkSequences.cend(),
                    [&](const auto current)
                    {
                        return current.lock() == sequence.lock();
                    }) == _linkSequences.cend()); // DEV Issue: check multiple insertion

    _linkSequences.emplace_back(sequence);
}

void SwapChain::unregisterSequence(SequenceWPtr sequence)
{
    // Find sequence into list
    auto itSequence = std::find_if(_linkSequences.begin(), _linkSequences.end(),
                                    [&](const auto current)
                                    {
                                        return current.lock() == sequence.lock();
                                    });
    MOUCA_PRE_CONDITION(itSequence != _linkSequences.end());
    // Remove item
    _linkSequences.erase(itSequence);
}

SwapChainImage::SwapChainImage():
_image(VK_NULL_HANDLE),
_view(VK_NULL_HANDLE)
{
    MOUCA_PRE_CONDITION(isNull());
}

SwapChainImage::~SwapChainImage()
{
    MOUCA_PRE_CONDITION(isNull());
}

void SwapChainImage::initialize(const Device& device, const VkFormat& format, const VkImage& image)
{
    MOUCA_PRE_CONDITION(isNull());
    MOUCA_PRE_CONDITION(image != VK_NULL_HANDLE);
    MOUCA_PRE_CONDITION(!device.isNull());
    MOUCA_PRE_CONDITION(format != VK_NULL_HANDLE);

    _image = image;

    const VkImageViewCreateInfo imageViewCreateInfo
    {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,   // VkStructureType                sType
        nullptr,                                    // const void                    *pNext
        0,                                          // VkImageViewCreateFlags         flags
        _image,                                     // VkImage                        image
        VK_IMAGE_VIEW_TYPE_2D,                      // VkImageViewType                viewType
        format,                                     // VkFormat                       format
        {                                           // VkComponentMapping             components
            VK_COMPONENT_SWIZZLE_R,                         // VkComponentSwizzle             r
            VK_COMPONENT_SWIZZLE_G,                         // VkComponentSwizzle             g
            VK_COMPONENT_SWIZZLE_B,                         // VkComponentSwizzle             b
            VK_COMPONENT_SWIZZLE_A                          // VkComponentSwizzle             a
        },
        {                                           // VkImageSubresourceRange        subresourceRange
            VK_IMAGE_ASPECT_COLOR_BIT,                  // VkImageAspectFlags             aspectMask
            0,                                          // uint32_t                       baseMipLevel
            1,                                          // uint32_t                       levelCount
            0,                                          // uint32_t                       baseArrayLayer
            1                                           // uint32_t                       layerCount
        }
    };

    if(vkCreateImageView(device.getInstance(), &imageViewCreateInfo, nullptr, &_view) != VK_SUCCESS)
    {
        MOUCA_THROW_ERROR(u8"Vulkan", u8"ImageViewCreationError");
    }
    
    MOUCA_POST_CONDITION(!isNull());
}

void SwapChainImage::release(const Device& device)
{
    MOUCA_PRE_CONDITION(!isNull());

    vkDestroyImageView(device.getInstance(), _view, nullptr);
    _view = VK_NULL_HANDLE;

    _image = VK_NULL_HANDLE;

    MOUCA_POST_CONDITION(isNull());
}

}