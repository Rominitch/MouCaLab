/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include "LibRT/include/RTWindow.h"

#include "LibVulkan/include/VKEnvironment.h"
#include "LibVulkan/include/VKDevice.h"
#include "LibVulkan/include/VKSurface.h"

namespace Vulkan
{

Surface::Surface():
_surface(VK_NULL_HANDLE)
{}

Surface::~Surface()
{
    BT_ASSERT(_surface == VK_NULL_HANDLE);
}

void Surface::initialize(const Environment& environment, const RT::Window& window)
{
    BT_ASSERT(_surface == VK_NULL_HANDLE);

#ifdef VK_USE_PLATFORM_WIN32_KHR
    //Prepare data
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo =
    {
        VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,  // VkStructureType                  sType
        nullptr,                                          // const void                      *pNext
        0,                                                // VkWin32SurfaceCreateFlagsKHR     flags
        reinterpret_cast<HINSTANCE>(window.getInstance()),// HINSTANCE                        hinstance
        reinterpret_cast<HWND>(window.getHandle())        // HWND                             hwnd
    };

    //Allocate surface
    if(vkCreateWin32SurfaceKHR(environment.getInstance(), &surfaceCreateInfo, nullptr, &_surface) != VK_SUCCESS)
    {
        BT_THROW_ERROR(u8"Vulkan", u8"SurfaceError");
    }
/*
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    VkXcbSurfaceCreateInfoKHR surface_create_info = {
        VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,    // VkStructureType                  sType
        nullptr,                                          // const void                      *pNext
        0,                                                // VkXcbSurfaceCreateFlagsKHR       flags
        Window.Connection,                                // xcb_connection_t*                connection
        Window.Handle                                     // xcb_window_t                     window
    };

    if (vkCreateXcbSurfaceKHR(Vulkan.Instance, &surface_create_info, nullptr, &_surface) == VK_SUCCESS) {
        return true;
    }

#elif defined(VK_USE_PLATFORM_XLIB_KHR)
    VkXlibSurfaceCreateInfoKHR surface_create_info = {
        VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,   // VkStructureType                sType
        nullptr,                                          // const void                    *pNext
        0,                                                // VkXlibSurfaceCreateFlagsKHR    flags
        Window.DisplayPtr,                                // Display                       *dpy
        Window.Handle                                     // Window                         window
    };
    if (vkCreateXlibSurfaceKHR(Vulkan.Instance, &surface_create_info, nullptr, &_surface) == VK_SUCCESS) {
        return true;
    }
*/
#else
    BT_THROW_ERROR(u8"Vulkan", L"SurfaceError");
#endif
}

void Surface::release(const Environment& environment)
{
    BT_ASSERT(_surface != VK_NULL_HANDLE);

    vkDestroySurfaceKHR(environment.getInstance(), _surface, nullptr);
    _surface = VK_NULL_HANDLE;
}

void Surface::computeSurfaceFormat(const Device& device, const SurfaceFormat::Configuration& userPreferences, SurfaceFormat& surfaceFormat)
{
    BT_ASSERT(!device.isNull());                 //DEV Issue: Need a valid device !

    //Read surface information
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    if(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.getPhysicalDevice(), _surface, &surfaceCapabilities) != VK_SUCCESS)
    {
        BT_THROW_ERROR(u8"Vulkan", u8"ReadSurfaceCapabilityError");
    }

    uint32_t nbFormats=0;
    if(vkGetPhysicalDeviceSurfaceFormatsKHR(device.getPhysicalDevice(), _surface, &nbFormats, nullptr) != VK_SUCCESS)
    {
        BT_THROW_ERROR(u8"Vulkan", u8"ReadSurfaceFormatError");
    }

    std::vector<VkSurfaceFormatKHR> surfaceFormats(nbFormats);
    if(vkGetPhysicalDeviceSurfaceFormatsKHR(device.getPhysicalDevice(), _surface, &nbFormats, &surfaceFormats[0]) != VK_SUCCESS)
    {
        BT_THROW_ERROR(u8"Vulkan", u8"ReadSurfaceFormatError");
    }

    uint32_t nbPresentModes=0;
    if(vkGetPhysicalDeviceSurfacePresentModesKHR(device.getPhysicalDevice(), _surface, &nbPresentModes, nullptr) != VK_SUCCESS)
    {
        BT_THROW_ERROR(u8"Vulkan", u8"ReadSurfacePresentModesError");
    }

    std::vector<VkPresentModeKHR> presentModes(nbPresentModes);
    if(vkGetPhysicalDeviceSurfacePresentModesKHR(device.getPhysicalDevice(), _surface, &nbPresentModes, &presentModes[0]) != VK_SUCCESS)
    {
        BT_THROW_ERROR(u8"Vulkan", u8"ReadSurfacePresentModesError");
    }

    // Analyze and adapt to surface requirement
    surfaceFormat.initialize(userPreferences, surfaceCapabilities, surfaceFormats, presentModes);
}

}