/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibVulkan/include/VKSurfaceFormat.h"

namespace Vulkan
{

bool SurfaceFormat::isSupported() const
{
    return _configuration._nbImages > 0
        && _configuration._format.format != VK_FORMAT_UNDEFINED;
}

void SurfaceFormat::initialize(const Configuration& preferred, const VkSurfaceCapabilitiesKHR& surfaceCapabilities, const std::vector<VkSurfaceFormatKHR>& surfaceFormats, const std::vector<VkPresentModeKHR>& presentModes)
{
    MouCa::assertion(!surfaceFormats.empty()); //DEV Issue: Need a valid list !
    MouCa::assertion(!presentModes.empty());   //DEV Issue: Need a valid list !

    _configuration = preferred;

    // Use preferred surface
    _configuration._nbImages = std::min(surfaceCapabilities.minImageCount + 1, surfaceCapabilities.maxImageCount);
    
    //---------------------------------------------------------------------------------------------
    // Search format
    auto itSurface = std::find_if(surfaceFormats.cbegin(), surfaceFormats.cend(),
                                  [&](const auto& surface) { return surface.format == _configuration._format.format
                                                                 && surface.colorSpace == _configuration._format.colorSpace; });
    if(itSurface == surfaceFormats.cend())
    {
        _configuration._format.format = VK_FORMAT_UNDEFINED;
    }

    // Default is first
    if(_configuration._format.format == VK_FORMAT_UNDEFINED)
    {
        _configuration._format = surfaceFormats.at(0);
    }

    //---------------------------------------------------------------------------------------------
    // Use preferred extent
    _configuration._extent = surfaceCapabilities.currentExtent;
    
    //---------------------------------------------------------------------------------------------
    // Usage
    if(_configuration._usage == 0)
    {
        _configuration._usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if((surfaceCapabilities.supportedUsageFlags & _configuration._usage) != _configuration._usage)
    {
        _configuration._usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    
    //---------------------------------------------------------------------------------------------
    // Take current transform (supposed supported)
    _configuration._transform = surfaceCapabilities.currentTransform;
    MouCa::assertion(surfaceCapabilities.supportedTransforms & surfaceCapabilities.currentTransform);
    
    //---------------------------------------------------------------------------------------------
    // Check presentation exists or use first
    const auto& presentMode = std::find(presentModes.cbegin(), presentModes.cend(), _configuration._presentMode);
    if (presentMode == presentModes.cend())
    {
        _configuration._presentMode = presentModes.at(0);
    }
}

}