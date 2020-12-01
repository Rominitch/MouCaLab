#include <Dependencies.h>

#include "include/VulkanTest.h"

#include <LibGLFW/include/GLFWPlatform.h>
#include <LibGLFW/include/GLFWWindow.h>

#include <LibVulkan/include/VKCommandBuffer.h>
#include <LibVulkan/include/VKEnvironment.h>
#include <LibVulkan/include/VKDevice.h>
#include <LibVulkan/include/VKFrameBuffer.h>
#include <LibVulkan/include/VKImage.h>
#include <LibVulkan/include/VKRenderPass.h>
#include <LibVulkan/include/VKSurface.h>
#include <LibVulkan/include/VKSurfaceFormat.h>
#include <LibVulkan/include/VKSwapChain.h>
#include <LibVulkan/include/VKImage.h>
/*
namespace Vulkan
{

class FrameBufferTest : public ::testing::Test
{
protected:
    static GLFW::Platform      _platform;
    static GLFW::WindowWPtr    _window;

    static Environment         _environment;
    static Device              _device;
    static Surface             _surface;
    static SurfaceFormatPtr    _format;
    static RenderPassSPtr      _renderPass;
    static SwapChain           _swapChain;

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

        ASSERT_NO_THROW(_environment.initialize(g_info, extensions));

        ASSERT_NO_THROW(_platform.initialize());

        RT::ViewportInt32 viewport(0, 0, 10, 10);
        _window = _platform.createWindow(viewport, std::string("Vulkan SwapChain"), RT::Window::InternalMode);
        GLFW::WindowSPtr lockedWindow = _window.lock();

        ASSERT_NO_THROW(_surface.initialize(_environment, *lockedWindow.get()));

        std::vector<const char*> deviceExtensions =
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
        ASSERT_NO_THROW(_device.initializeBestGPU(_environment, deviceExtensions, &_surface));
        
        SurfaceFormat::Configuration configuration;
        configuration._presentMode = VK_PRESENT_MODE_MAILBOX_KHR;

        _format = std::make_shared<SurfaceFormat>();
        ASSERT_NO_THROW(_surface.computeSurfaceFormat(_device, configuration, *_format.get()));
        
        _renderPass = std::make_shared<RenderPass>();
        ASSERT_NO_THROW(_renderPass->initialize(_device, *_format.get()));
        
        ASSERT_NO_THROW(_swapChain.initialize(_device, _surface, *_format.get()));

        ASSERT_NO_THROW(_swapChain.generateImages(_device, *_format.get()));
    }

    static void TearDownTestSuite()
    {
        ASSERT_NO_THROW(_swapChain.release(_device));

        ASSERT_NO_THROW(_renderPass->release(_device));
        _renderPass.reset();

        ASSERT_NO_THROW(_surface.release(_environment));

        ASSERT_NO_THROW(_device.release());

        ASSERT_NO_THROW(_environment.release());

        ASSERT_NO_THROW(_platform.releaseWindow(_window));

        ASSERT_NO_THROW(_platform.release());
    }

    virtual void SetUp() final
    {
        
    }

    virtual void TearDown() final
    {
        
    }

};

GLFW::Platform     FrameBufferTest::_platform;
GLFW::WindowWPtr   FrameBufferTest::_window;

Environment        FrameBufferTest::_environment;
Device             FrameBufferTest::_device;
Surface            FrameBufferTest::_surface;
SurfaceFormatPtr   FrameBufferTest::_format;
RenderPassSPtr     FrameBufferTest::_renderPass;
SwapChain          FrameBufferTest::_swapChain;

TEST_F(FrameBufferTest, initialize)
{
    const VkExtent2D resolution = { 10, 10 };

    FrameBuffer frameBuffer;

    ASSERT_TRUE(frameBuffer.isNull());

    const FrameBuffer::Attachments attachments
    {
        _swapChain.getImages().at(0).getView()
    };

    ASSERT_NO_THROW(frameBuffer.initialize(_device, _renderPass, resolution, attachments));
    ASSERT_FALSE(frameBuffer.isNull());

    ASSERT_NO_THROW(frameBuffer.release(_device));

    ASSERT_TRUE(frameBuffer.isNull());
}

}

*/