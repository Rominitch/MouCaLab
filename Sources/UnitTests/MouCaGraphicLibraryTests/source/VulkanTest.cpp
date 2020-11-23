/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include <LibGLFW/include/GLFWHardware.h>
#include <LibGLFW/include/GLFWWindow.h>

#include <LibRT/include/RTImage.h>

#include <LibVulkan/include/VKScreenshot.h>

#include <include/VulkanTest.h>

void VulkanTest::SetUpTestSuite()
{
    ASSERT_NO_THROW(_platform.initialize());

    const std::vector<const char*> extensions =
    {
        VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_XCB_KHR)
        VK_KHR_XCB_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
        VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#endif
    };

    ASSERT_NO_THROW(_environment.initialize(g_info, extensions));

    _viewport.setSize(1280, 720);

    _resolution = { _viewport.getWidth(), _viewport.getHeight() };
    _renderArea = { { 0, 0 },{ _resolution.x, _resolution.y } };
    VkViewport viewport =
    {
        static_cast<float>(_renderArea.offset.x), static_cast<float>(_renderArea.offset.y),
        static_cast<float>(_resolution.x), static_cast<float>(_resolution.y),
        0.0f, 1.0f
    };
}

void VulkanTest::TearDownTestSuite()
{
    ASSERT_NO_THROW(_environment.release());

    ASSERT_NO_THROW(_platform.release());
}

void VulkanTest::SetUp()
{
    ASSERT_NO_THROW(_window = _platform.createWindow(_viewport, _windowName, _mode));
}

void VulkanTest::TearDown()
{
    ASSERT_NO_THROW(_platform.releaseWindow(_window));
    ASSERT_EQ(0, _platform.getNumberWindows());
}

GLFW::Platform              VulkanTest::_platform;
RT::ViewportInt32           VulkanTest::_viewport;

RT::Array2ui                VulkanTest::_resolution;
VkRect2D                    VulkanTest::_renderArea;

Vulkan::Environment         VulkanTest::_environment;

//-------------------------------------------------------------------------------------------------
//                                          VulkanTestDevice
//-------------------------------------------------------------------------------------------------
void VulkanDeviceTest::SetUp()
{
    VulkanTest::SetUp();
    ASSERT_TRUE(!_window.expired());

    // Build surface from window
    ASSERT_NO_THROW(_surface = std::make_unique<Vulkan::Surface>());
    ASSERT_NO_THROW(_surface->initialize(_environment, *_window.lock()));

    // Build default device
    std::vector<const char*> deviceExtensions =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    VkPhysicalDeviceFeatures features = {};
    ASSERT_NO_THROW(_contextDevice.initialize(_environment, deviceExtensions, features, _surface.get()));
}

void VulkanDeviceTest::TearDown()
{
    _contextDevice.release();

    _surface->release(_environment);
    _surface.reset();

    VulkanTest::TearDown();
}