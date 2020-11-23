#include <Dependancies.h>

#include <LibGLFW/include/GLFWPlatform.h>
#include <LibGLFW/include/GLFWWindow.h>

#include <LibVulkan/include/VKEnvironment.h>
#include <LibVulkan/include/VKDevice.h>
#include <LibVulkan/include/VKSurface.h>
#include <LibVulkan/include/VKSurfaceFormat.h>

#include "include/VulkanTest.h"


class VulkanSurfaceTest : public VulkanTest
{
protected:
    static Vulkan::Environment  _environment;
    static Vulkan::Device       _device;

    static GLFW::Platform       _platform;
    static GLFW::WindowWPtr     _window;

    static void SetUpTestSuite()
    {
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

        ASSERT_NO_THROW( _environment.initialize(g_info, extensions) );
    
        std::vector<const char*> deviceExtensions =
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
        ASSERT_NO_THROW( _device.initializeBestGPU(_environment, deviceExtensions) );

        ASSERT_NO_THROW(_platform.initialize());
    }

    static void TearDownTestSuite()
    {
        ASSERT_NO_THROW( _device.release() );

        ASSERT_NO_THROW( _environment.release() );

        ASSERT_NO_THROW(_platform.release());
    }
    
    VulkanSurfaceTest():
    VulkanTest("VulkanSurfaceTest")
    {}

    void SetUp() final
    {  
        RT::ViewportInt32 viewport(0, 0, 10, 10);
        _window = _platform.createWindow(viewport, std::string("Vulkan Surface"), RT::Window::InternalMode);
    }

    void TearDown() final
    {
        ASSERT_NO_THROW(_platform.releaseWindow(_window));
    }

};

GLFW::Platform      VulkanSurfaceTest::_platform;
GLFW::WindowWPtr    VulkanSurfaceTest::_window;

Vulkan::Environment VulkanSurfaceTest::_environment;
Vulkan::Device      VulkanSurfaceTest::_device;

TEST_F(VulkanSurfaceTest, initialize)
{
    Vulkan::Surface surface;
    ASSERT_TRUE(surface.isNull());

    GLFW::WindowSPtr lockedWindow = _window.lock();
    ASSERT_NO_THROW(surface.initialize(_environment, *lockedWindow.get()));

    ASSERT_FALSE(surface.isNull());

    ASSERT_NO_THROW(surface.release(_environment));
}

TEST_F(VulkanSurfaceTest, createSurfaceFormat)
{
    Vulkan::Surface surface;
    ASSERT_TRUE(surface.isNull());
    GLFW::WindowSPtr lockedWindow = _window.lock();
    ASSERT_NO_THROW(surface.initialize(_environment, *lockedWindow.get()));

    auto format = std::make_shared<Vulkan::SurfaceFormat>();

    Vulkan::SurfaceFormat::Configuration configuration;
    // Case 1: default value
    {
        ASSERT_NO_THROW(surface.computeSurfaceFormat(_device, configuration, *format.get()));
        EXPECT_TRUE(format->isSupported());

        // Currently TRUE on: Windows 10 + GTX 1080 / RTX 5000
        EXPECT_EQ(VK_PRESENT_MODE_MAILBOX_KHR,           format->getConfiguration()._presentMode);
        EXPECT_EQ(VK_FORMAT_B8G8R8A8_UNORM,              format->getConfiguration()._format.format);
        EXPECT_TRUE((VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT & format->getConfiguration()._usage) == VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
        EXPECT_EQ(VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, format->getConfiguration()._transform);
    }

    // Case 2: Customize
    {
        configuration._presentMode   = VK_PRESENT_MODE_FIFO_KHR;
        configuration._format.format = VK_FORMAT_B8G8R8A8_UNORM;
        configuration._usage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        configuration._transform     = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

        ASSERT_NO_THROW(surface.computeSurfaceFormat(_device, configuration, *format.get()));
        EXPECT_TRUE(format->isSupported());

        // Currently TRUE on: Windows 10 + GTX 1080 / RTX 5000
        EXPECT_EQ(configuration._presentMode,   format->getConfiguration()._presentMode);
        EXPECT_EQ(configuration._format.format, format->getConfiguration()._format.format);
        EXPECT_TRUE((configuration._usage & format->getConfiguration()._usage) == configuration._usage);
        EXPECT_EQ(configuration._transform,     format->getConfiguration()._transform);
    }

    // Case 3: Bad config
    {
        configuration._presentMode   = VK_PRESENT_MODE_MAX_ENUM_KHR;
        configuration._format.format = VK_FORMAT_MAX_ENUM;
        configuration._usage         = VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM;
        configuration._transform     = VK_SURFACE_TRANSFORM_FLAG_BITS_MAX_ENUM_KHR;

        ASSERT_NO_THROW(surface.computeSurfaceFormat(_device, configuration, *format.get()));
        EXPECT_TRUE(format->isSupported());

        // Currently TRUE on: Windows 10 + GTX 1080 / RTX 5000
        EXPECT_EQ(VK_PRESENT_MODE_FIFO_KHR,              format->getConfiguration()._presentMode);
        EXPECT_EQ(VK_FORMAT_B8G8R8A8_UNORM,              format->getConfiguration()._format.format);
        EXPECT_TRUE((VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT & format->getConfiguration()._usage) == VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
        EXPECT_EQ(VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, format->getConfiguration()._transform);
    }

    ASSERT_NO_THROW(surface.release(_environment));
}
