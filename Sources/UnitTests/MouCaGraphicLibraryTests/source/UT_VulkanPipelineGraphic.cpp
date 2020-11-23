#include <Dependancies.h>

#include "include/VulkanTest.h"

#include <LibCore/include/BTFile.h>

#include <LibGLFW/include/GLFWPlatform.h>
#include <LibGLFW/include/GLFWWindow.h>

#include <LibVulkan/include/VKEnvironment.h>
#include <LibVulkan/include/VKDescriptorSet.h>
#include <LibVulkan/include/VKDevice.h>
#include <LibVulkan/include/VKPipelineCache.h>
#include <LibVulkan/include/VKGraphicsPipeline.h>
#include <LibVulkan/include/VKPipelineLayout.h>
#include <LibVulkan/include/VKPipelineStates.h>
#include <LibVulkan/include/VKRenderPass.h>
#include <LibVulkan/include/VKShaderProgram.h>
#include <LibVulkan/include/VKSurface.h>
#include <LibVulkan/include/VKSurfaceFormat.h>

/*
class VulkanPipeline : public ::testing::Test
{
protected:
    static GLFW::Platform              _platform;
    static GLFW::WindowWPtr            _window;
    static Vulkan::Environment         _environment;
    static Vulkan::Device              _device;
    static Vulkan::Surface             _surface;
    static Vulkan::SurfaceFormatSPtr   _format;
    static Vulkan::RenderPass          _renderPass;
    static Vulkan::ShaderProgram       _vertexProgram;
    static Vulkan::ShaderProgram       _fragmentProgram;

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

        std::vector<const char*> deviceExtensions =
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
        ASSERT_NO_THROW(_device.initializeBestGPU(_environment, deviceExtensions));

        ASSERT_NO_THROW(_platform.initialize());

        RT::ViewportInt32 viewport(0, 0, 10, 10);
        _window = _platform.createWindow(viewport, std::string("Vulkan Demo"), RT::Window::Windowed);
        GLFW::WindowSPtr lockedWindow = _window.lock();

        ASSERT_NO_THROW(_surface.initialize(_environment, *lockedWindow.get()));

        Vulkan::SurfaceFormat::Configuration configuration;
        configuration._presentMode = VK_PRESENT_MODE_MAILBOX_KHR;

        _format = std::make_shared<Vulkan::SurfaceFormat>();
        ASSERT_NO_THROW(_surface.computeSurfaceFormat(_device, configuration, *_format.get()));

        ASSERT_NO_THROW(_renderPass.initialize(_device, *_format.get()));

        //Prepare Vertex
        Core::File vertexShader(MouCaEnvironment::getInputPath() / L"libraries" / L"vertex.spv");
        ASSERT_NO_THROW(vertexShader.open());
        ASSERT_NO_THROW(_vertexProgram.initialize(_device, vertexShader, std::string("main")));

        //Prepare fragment
        Core::File fragmentShader(MouCaEnvironment::getInputPath() / L"libraries" / L"fragment.spv");
        ASSERT_NO_THROW(fragmentShader.open());
        ASSERT_NO_THROW(_fragmentProgram.initialize(_device, fragmentShader, std::string("main")));
    }

    static void TearDownTestSuite()
    {
        ASSERT_NO_THROW(_vertexProgram.release(_device));
        ASSERT_NO_THROW(_fragmentProgram.release(_device));

        ASSERT_NO_THROW(_renderPass.release(_device));

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

GLFW::Platform              VulkanPipeline::_platform;
GLFW::WindowWPtr            VulkanPipeline::_window;

Vulkan::Environment         VulkanPipeline::_environment;
Vulkan::Device              VulkanPipeline::_device;
Vulkan::Surface             VulkanPipeline::_surface;
Vulkan::SurfaceFormatSPtr   VulkanPipeline::_format;
Vulkan::RenderPass          VulkanPipeline::_renderPass;
Vulkan::ShaderProgram       VulkanPipeline::_vertexProgram;
Vulkan::ShaderProgram       VulkanPipeline::_fragmentProgram;

TEST_F(VulkanPipeline, initializeLayout)
{
    Vulkan::PipelineLayout layout;

    ASSERT_TRUE(layout.isNull());

    ASSERT_NO_THROW(layout.initialize(_device));

    ASSERT_FALSE(layout.isNull());
    ASSERT_TRUE(layout.getInstance() != VK_NULL_HANDLE);

    ASSERT_NO_THROW(layout.release(_device));
    ASSERT_TRUE(layout.isNull());
}

TEST_F(VulkanPipeline, initialize)
{
    VkRect2D   renderArea = {{0, 0},{10, 10}};
    VkViewport viewport = {static_cast<float>(renderArea.offset.x), static_cast<float>(renderArea.offset.y),
        static_cast<float>(renderArea.extent.width), static_cast<float>(renderArea.extent.height, 0.0f), 1.0f};

    //Build stages
    Vulkan::GraphicsPipeline pipeline;
    pipeline.getInfo().initialize(Vulkan::PipelineStateCreateInfo::ViewportArea);
    pipeline.getInfo().getViewport().addViewport(viewport, renderArea);
    pipeline.getInfo().getStages().addShaderVertex(_vertexProgram);
    pipeline.getInfo().getStages().addShaderFragment(_fragmentProgram);
    pipeline.getInfo().getDynamic().addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
    pipeline.getInfo().getDynamic().addDynamicState(VK_DYNAMIC_STATE_SCISSOR);

    Vulkan::PipelineLayout layout;
    ASSERT_NO_THROW(layout.initialize(_device));

    ASSERT_TRUE(pipeline.isNull());

    ASSERT_NO_THROW(pipeline.initialize(_device, _renderPass, layout));

    ASSERT_FALSE(pipeline.isNull());
    ASSERT_TRUE(pipeline.getInstance() != VK_NULL_HANDLE);

    ASSERT_NO_THROW(pipeline.release(_device));
    ASSERT_TRUE(pipeline.isNull());

    ASSERT_NO_THROW(layout.release(_device));
}

TEST_F(VulkanPipeline, initializeCompleteStages)
{
    VkRect2D   renderArea = {{0, 0},{10, 10}};
    VkViewport viewport = {static_cast<float>(renderArea.offset.x), static_cast<float>(renderArea.offset.y),
        static_cast<float>(renderArea.extent.width), static_cast<float>(renderArea.extent.height, 0.0f), 1.0f};

    Core::File dvShader(MouCaEnvironment::getInputPath() / L".." / L"SpirV" / L"PipelineCompleteStage.vert.spv");
    ASSERT_NO_THROW(dvShader.open());
    Core::File dfShader(MouCaEnvironment::getInputPath() / L".." / L"SpirV" / L"PipelineCompleteStage.frag.spv");
    ASSERT_NO_THROW(dfShader.open());
    Vulkan::ShaderModuleSPtr vertex = std::make_shared<Vulkan::ShaderModule>();
    ASSERT_NO_THROW(vertex->initialize(_device, dvShader.extractString(), "main", VK_SHADER_STAGE_VERTEX_BIT));
    Vulkan::ShaderModuleSPtr fragment = std::make_shared<Vulkan::ShaderModule>();
    ASSERT_NO_THROW(fragment->initialize(_device, dfShader.extractString(), "main", VK_SHADER_STAGE_FRAGMENT_BIT));

    // Use specialization constants to pass number of samples to the shader (used for MSAA resolve)
    const VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_8_BIT;
    
    Vulkan::DescriptorSetLayout descriptorLayout;
    // Binding 0 : Vertex shader uniform buffer
    descriptorLayout.addUniformBinding(VK_SHADER_STAGE_VERTEX_BIT);
    // Binding 1 : Position texture target / Scene colormap
    descriptorLayout.addImageSamplerBinding(VK_SHADER_STAGE_FRAGMENT_BIT);
    // Binding 2 : Normals texture target
    descriptorLayout.addImageSamplerBinding(VK_SHADER_STAGE_FRAGMENT_BIT);
    // Binding 3 : Albedo texture target
    descriptorLayout.addImageSamplerBinding(VK_SHADER_STAGE_FRAGMENT_BIT);
    // Binding 4 : Fragment shader uniform buffer
    descriptorLayout.addUniformBinding(VK_SHADER_STAGE_FRAGMENT_BIT);

    // Create layout
    descriptorLayout.initialize(_device);

    // Create layout pipeline
    const std::vector<VkDescriptorSetLayout> descriptorLayouts =
    {
        descriptorLayout.getInstance()
    };

    //Build stages
    Vulkan::GraphicsPipeline pipeline;

    pipeline.getInfo().getDynamic().addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
    pipeline.getInfo().getDynamic().addDynamicState(VK_DYNAMIC_STATE_SCISSOR);
    pipeline.getInfo().initialize(Vulkan::PipelineStateCreateInfo::Renderable);
    {
        Vulkan::ShaderSpecialization specialization;
        pipeline.getInfo().getStages().addShaderModule(vertex, std::move(specialization));
    }
    {
        Vulkan::ShaderSpecialization specialization;
        specialization.addDataInfo(&sampleCount, sizeof(sampleCount));
        VkSpecializationMapEntry entry =
        {
            0,                  // uint32_t    constantID;
            0,                  // uint32_t    offset;
            sizeof(uint32_t)    // size_t      size;
        };
        specialization.addMapEntry(std::move(entry));
        pipeline.getInfo().getStages().addShaderModule(fragment, std::move(specialization));
    }

    Vulkan::PipelineCache cache;
    ASSERT_NO_THROW(cache.initialize(_device));

    Vulkan::PipelineLayout layout;
    ASSERT_NO_THROW(layout.initialize(_device, descriptorLayouts));

    ASSERT_TRUE(pipeline.isNull());

    ASSERT_NO_THROW(pipeline.initialize(_device, _renderPass, layout, cache));

    ASSERT_FALSE(pipeline.isNull());
    ASSERT_TRUE(pipeline.getInstance() != VK_NULL_HANDLE);

    ASSERT_NO_THROW(pipeline.release(_device));
    ASSERT_TRUE(pipeline.isNull());

    ASSERT_NO_THROW(layout.release(_device));
    ASSERT_NO_THROW(cache.release(_device));

    ASSERT_NO_THROW(vertex->release(_device));
    ASSERT_NO_THROW(fragment->release(_device));

    ASSERT_NO_THROW(descriptorLayout.release(_device));
}
*/