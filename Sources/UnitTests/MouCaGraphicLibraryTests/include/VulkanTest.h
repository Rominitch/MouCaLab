/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibVulkan/include/VKCommandBuffer.h>
#include <LibVulkan/include/VKCommandPool.h>
#include <LibVulkan/include/VKEnvironment.h>
#include <LibVulkan/include/VKContextDevice.h>
#include <LibVulkan/include/VKContextWindow.h>
#include <LibVulkan/include/VKSurface.h>

#include <LibGLFW/include/GLFWPlatform.h>

#include <LibRT/include/RTEnum.h>

// Forward declaration
namespace Vulkan
{
    class Surface;
    using SurfaceUPtr = std::unique_ptr<Surface>;
}

static const RT::ApplicationInfo g_info
{
    u8"MouCaUnitTest",
    0x00000001,
    u8"MouCaEngine",
    0x00000001
};

/// A simply RenderPass parameter sample for test
struct RenderPassParameters
{
    RenderPassParameters():
    attachments
    ({ {
            0,
            VK_FORMAT_B8G8R8A8_SNORM,
            VK_SAMPLE_COUNT_1_BIT,
            VK_ATTACHMENT_LOAD_OP_CLEAR,        VK_ATTACHMENT_STORE_OP_STORE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,    VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_UNDEFINED,          VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        } }),
    references
    ({
        { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }
    }),
    subpasses( { {
        0,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        0, nullptr,
        static_cast<uint32_t>(references.size()), references.data(), nullptr, nullptr,
        0, nullptr
    } }),
    dependencies
    ({
        { VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT },
        { 0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,          VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0, VK_DEPENDENCY_BY_REGION_BIT }
    })
    {}
    
    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkAttachmentReference>   references;
    std::vector<VkSubpassDescription>    subpasses;
    std::vector<VkSubpassDependency>     dependencies;
};

/// Vulkan test environment: Window + Vulkan environment
class VulkanTest : public ::testing::Test
{
    protected:
#ifndef NDEBUG
        //static const RT::Window::Mode       _mode = RT::Window::StaticDialogMode;
        static const RT::Window::Mode       _mode = RT::Window::InternalMode;
#else
        static const RT::Window::Mode       _mode = RT::Window::InternalMode;
#endif
        // Platform / Window
        static GLFW::Platform               _platform;
        static RT::ViewportInt32            _viewport;
        static RT::Array2ui                 _resolution;
        // Vulkan
        static Vulkan::Environment          _environment;
        static VkRect2D                     _renderArea;
        static const VkSampleCountFlagBits  _sample = VK_SAMPLE_COUNT_8_BIT;

        GLFW::WindowWPtr    _window;
        Core::String        _windowName;

        explicit VulkanTest(const Core::String& windowName):
        _windowName(windowName)
        {}

        static void SetUpTestSuite();

        static void TearDownTestSuite();

        void SetUp() override;

        void TearDown() override;
};

/// Vulkan test environment: window + default device
class VulkanDeviceTest : public VulkanTest
{
    protected:
        Vulkan::ContextDevice _contextDevice;
        Vulkan::ContextWindow _contextWindow;
        Vulkan::SurfaceUPtr   _surface;         ///< [OWNERSHIP] Vulkan surface.

        explicit VulkanDeviceTest(const Core::String& windowName) :
        VulkanTest(windowName)
        {}

        void SetUp() override;

        void TearDown() override;

};