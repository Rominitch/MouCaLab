#include <Dependancies.h>

#include "include/VulkanTest.h"

#include <LibCore/include/CoreFile.h>

#include <LibGLFW/include/GLFWPlatform.h>
#include <LibGLFW/include/GLFWWindow.h>

#include <LibVulkan/include/VKCommandBuffer.h>
#include <LibVulkan/include/VKCommandPool.h>
#include <LibVulkan/include/VKEnvironment.h>
#include <LibVulkan/include/VKDevice.h>
#include <LibVulkan/include/VKFrameBuffer.h>
#include <LibVulkan/include/VKImage.h>
#include <LibVulkan/include/VKRenderPass.h>
#include <LibVulkan/include/VKGraphicsPipeline.h>
#include <LibVulkan/include/VKPipelineLayout.h>
#include <LibVulkan/include/VKPipelineStates.h>
#include <LibVulkan/include/VKShaderProgram.h>
#include <LibVulkan/include/VKSurface.h>
#include <LibVulkan/include/VKSurfaceFormat.h>
#include <LibVulkan/include/VKSwapChain.h>
/*
class VulkanCmdBufferTest : public ::testing::Test
{
protected:
    static GLFW::Platform              _platform;
    static GLFW::WindowWPtr            _window;
    static Vulkan::Environment         _environment;
    static Vulkan::Device              _device;
    static Vulkan::Surface             _surface;
    static Vulkan::SurfaceFormatPtr    _format;
    static Vulkan::SwapChain           _swapChain;
    static Vulkan::PipelineLayout      _pipelineLayout;
    static Vulkan::GraphicsPipeline     _pipelineGraphic;
    static VkExtent2D                  _resolution;
    static VkRect2D                    _renderArea;
    static Vulkan::RenderPass          _renderPass;
    static Vulkan::FrameBuffer         _frameBuffer;

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
    
        RT::ViewportInt32 viewport(0, 0, 10, 10);
        _resolution = {viewport.getWidth(), viewport.getHeight()};
        _renderArea = {{0, 0}, _resolution};
        VkViewport vkViewport = { static_cast<float>(_renderArea.offset.x), static_cast<float>(_renderArea.offset.y),
                                static_cast<float>(_resolution.width), static_cast<float>(_resolution.height), 0.0f, 1.0f};

        ASSERT_NO_THROW(_platform.initialize());

        _window = _platform.createWindow(viewport, std::string("VulkanDemo"), RT::Window::InternalMode);
        {
            GLFW::WindowSPtr lockedWindow = _window.lock();
            ASSERT_NO_THROW(_surface.initialize(_environment, *lockedWindow.get()));
        }

        std::vector<const char*> deviceExtensions =
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
        ASSERT_NO_THROW(_device.initializeBestGPU(_environment, deviceExtensions, &_surface));

        Vulkan::ShaderProgram programVertex;
        {
            Core::File shaderFile(MouCaEnvironment::getInputPath() / L"libraries"/L"vertex.spv");
            ASSERT_NO_THROW(shaderFile.open());
            ASSERT_NO_THROW(programVertex.initialize(_device, shaderFile, std::string("main")));
        }

        Vulkan::ShaderProgram programFragment;
        {
            Core::File shaderFile(MouCaEnvironment::getInputPath() / L"libraries"/L"fragment.spv");
            ASSERT_NO_THROW(shaderFile.open());
            ASSERT_NO_THROW(programFragment.initialize(_device, shaderFile, std::string("main")));
        }        

        _pipelineGraphic.getInfo().initialize(Vulkan::PipelineStateCreateInfo::ViewportArea);
        _pipelineGraphic.getInfo().getViewport().addViewport(vkViewport, _renderArea);
        _pipelineGraphic.getInfo().getStages().addShaderVertex(programVertex);
        _pipelineGraphic.getInfo().getStages().addShaderFragment(programFragment);
        _pipelineGraphic.getInfo().getDynamic().addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
        _pipelineGraphic.getInfo().getDynamic().addDynamicState(VK_DYNAMIC_STATE_SCISSOR);

        Vulkan::SurfaceFormat::Configuration configuration;
        configuration._presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
        _format = std::make_shared<Vulkan::SurfaceFormat>();
        ASSERT_NO_THROW(_surface.computeSurfaceFormat(_device, configuration, *_format.get()));

        ASSERT_NO_THROW(_swapChain.initialize(_device, _surface, *_format.get()));

        ASSERT_NO_THROW(_swapChain.generateImages(_device, *_format.get()));
       
        ASSERT_NO_THROW(_renderPass.initialize(_device, *_format.get()));

        ASSERT_NO_THROW(_frameBuffer.initialize(_device, _renderPass, _resolution, _swapChain));

        ASSERT_NO_THROW(_pipelineLayout.initialize(_device));

        ASSERT_NO_THROW(_pipelineGraphic.initialize(_device, _renderPass, _pipelineLayout));

        programVertex.release(_device);
        programFragment.release(_device);
    }

    static void TearDownTestSuite()
    {
        ASSERT_NO_THROW(_pipelineLayout.release(_device));

        ASSERT_NO_THROW(_pipelineGraphic.release(_device));

        ASSERT_NO_THROW(_frameBuffer.release(_device));

        ASSERT_NO_THROW(_renderPass.release(_device));

        ASSERT_NO_THROW(_swapChain.release(_device));

        ASSERT_NO_THROW(_surface.release(_environment));

        ASSERT_NO_THROW(_device.release());

        ASSERT_NO_THROW(_environment.release());
        
        ASSERT_NO_THROW(_platform.releaseWindow(_window));
        
        ASSERT_NO_THROW(_platform.release());
    }

};

GLFW::Platform              VulkanCmdBufferTest::_platform;
GLFW::WindowWPtr            VulkanCmdBufferTest::_window;

VkExtent2D                  VulkanCmdBufferTest::_resolution;
VkRect2D                    VulkanCmdBufferTest::_renderArea;

Vulkan::Device              VulkanCmdBufferTest::_device;
Vulkan::Environment         VulkanCmdBufferTest::_environment;
Vulkan::FrameBuffer         VulkanCmdBufferTest::_frameBuffer;
Vulkan::RenderPass          VulkanCmdBufferTest::_renderPass;
Vulkan::GraphicsPipeline    VulkanCmdBufferTest::_pipelineGraphic;
Vulkan::PipelineLayout      VulkanCmdBufferTest::_pipelineLayout;
Vulkan::Surface             VulkanCmdBufferTest::_surface;
Vulkan::SurfaceFormatPtr    VulkanCmdBufferTest::_format;
Vulkan::SwapChain           VulkanCmdBufferTest::_swapChain;

TEST_F(VulkanCmdBufferTest, commandPoolInitialize)
{
    Vulkan::CommandPool commandPool;

    ASSERT_TRUE(commandPool.isNull());

    ASSERT_NO_THROW(commandPool.initialize(_device, _device.getQueueFamilyGraphicId()));

    ASSERT_FALSE(commandPool.isNull());
    ASSERT_TRUE(commandPool.getInstance() != VK_NULL_HANDLE);

    ASSERT_NO_THROW(commandPool.release(_device));

    ASSERT_TRUE(commandPool.isNull());
}
*/
