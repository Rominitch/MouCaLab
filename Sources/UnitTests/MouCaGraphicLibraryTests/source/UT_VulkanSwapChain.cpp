#include <Dependencies.h>

#include "include/VulkanTest.h"

#include <LibGLFW/include/GLFWPlatform.h>
#include <LibGLFW/include/GLFWWindow.h>

#include <LibVulkan/include/VKEnvironment.h>
#include <LibVulkan/include/VKDevice.h>
#include <LibVulkan/include/VKSurface.h>
#include <LibVulkan/include/VKSurfaceFormat.h>
#include <LibVulkan/include/VKSwapChain.h>

#include "include/VulkanTest.h"

/*
class VulkanSwapChainTest : public VulkanBaseTest
{
protected:
    static GLFW::Platform              _platform;
    static GLFW::WindowWPtr            _window;
    static Vulkan::Environment         _environment;
    static Vulkan::Device              _device;
    static Vulkan::Surface             _surface;
    static Vulkan::SurfaceFormatSPtr   _format;

    static void SetUpTestSuite()
    {
        // Start Vulkan environment
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

        // Launch window
        ASSERT_NO_THROW(_platform.initialize());

        RT::ViewportInt32 viewport(0, 0, 10, 10);
        _window = _platform.createWindow(viewport, std::string("Vulkan SwapChain"), RT::Window::InternalMode);
        GLFW::WindowSPtr lockedWindow = _window.lock();

        ASSERT_NO_THROW(_surface.initialize(_environment, *lockedWindow.get()));

        // Build device with surface -> swapchain mandatory
        std::vector<const char*> deviceExtensions =
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
        ASSERT_NO_THROW( _device.initializeBestGPU(_environment, deviceExtensions, &_surface) );

        Vulkan::SurfaceFormat::Configuration configuration;
        configuration._presentMode = VK_PRESENT_MODE_MAILBOX_KHR;

        _format = std::make_shared<Vulkan::SurfaceFormat>();
        ASSERT_NO_THROW(_surface.computeSurfaceFormat(_device, configuration, *_format.get()));
    }

    static void TearDownTestSuite()
    {
        ASSERT_NO_THROW(_surface.release(_environment));

        ASSERT_NO_THROW(_device.release());

        ASSERT_NO_THROW(_environment.release());

        ASSERT_NO_THROW(_platform.releaseWindow(_window));

        ASSERT_NO_THROW(_platform.release());
    }
    
    virtual void SetUp() final
    {}

    virtual void TearDown() final
    {}

};

GLFW::Platform              VulkanSwapChainTest::_platform;
GLFW::WindowWPtr            VulkanSwapChainTest::_window;
Vulkan::Environment         VulkanSwapChainTest::_environment;
Vulkan::Device              VulkanSwapChainTest::_device;
Vulkan::Surface             VulkanSwapChainTest::_surface;
Vulkan::SurfaceFormatSPtr   VulkanSwapChainTest::_format;

TEST_F(VulkanSwapChainTest, initialize)
{
    // Create and initialize swapchain
    Vulkan::SwapChain swapChain;

    ASSERT_TRUE(swapChain.isNull());
    ASSERT_NO_THROW(swapChain.initialize(_device, _surface, *_format.get()));
    ASSERT_FALSE(swapChain.isNull());

    // Clean
    ASSERT_NO_THROW(swapChain.release(_device));
    ASSERT_TRUE(swapChain.isNull());
}
*/