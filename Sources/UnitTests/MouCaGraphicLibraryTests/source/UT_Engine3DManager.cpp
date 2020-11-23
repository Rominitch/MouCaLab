#include "Dependancies.h"

#include "include/VulkanTest.h"

#include "LibRT/include/RTShaderFile.h"

#include "LibVulkan/include/VKContextDevice.h"
#include "LibVulkan/include/VKContextWindow.h"
#include "LibVulkan/include/VKSequence.h"

#include "UnitTests/include/MouCa3DVulkanTest.h"

namespace MouCa3DEngine
{

class Engine3DManagerTest : public MouCaGameEngine3DTest
{};

/// Simple event catcher to check if event are properly send
struct CheckEvents
{
    CheckEvents() : _edition(false), _resize(false), _close(false)
    {}

    void edition() { _edition = true; }
    void resize() { _resize = true; }
    void close() { _close = true; }

    bool _edition;
    bool _resize;
    bool _close;
};

TEST_F(Engine3DManagerTest, buildDevice)
{
    RT::RenderDialogWPtr dialog = createWindow(BT::String(u8"Engine3DManagerTest::buildDevice"));

    Engine3DManager manager;
    
    // Add dialog to manage (don't care about Engine3DManager initialization)
    BT::uint32 idSurface=0;
    ASSERT_NO_THROW(idSurface = manager.addRenderDialog(dialog));
    EXPECT_EQ(1, manager.getSurfaces().size());

    // Simple initialization manual
    {
        ASSERT_NO_THROW(manager.initialize(g_info, Engine3DManager::getSurfaceExtensions()));

        ASSERT_NO_THROW(manager.createRenderingDevice(Engine3DManager::getDeviceExtensions(), mandatoryFeatures, *manager.getSurfaces().at(idSurface) ));
        EXPECT_EQ(1, manager.getDevices().size());

        ASSERT_NO_THROW(manager.release());
        EXPECT_EQ(0, manager.getDevices().size());
        EXPECT_EQ(1, manager.getSurfaces().size());
    }

    // Simple initialization based on xml
    {
        loadEngine(manager, "Environment.xml");

        ASSERT_NO_THROW(manager.release());
        EXPECT_EQ(0, manager.getDevices().size());
        EXPECT_EQ(1, manager.getSurfaces().size());
    }
    
    // Bad device: unsupported features
    {
        // For my GTX 1080/RTX 5000: this is impossible
        mandatoryFeatures.textureCompressionETC2     = VK_TRUE;
        mandatoryFeatures.textureCompressionASTC_LDR = VK_TRUE;

        ASSERT_NO_THROW(manager.initialize(g_info, Engine3DManager::getSurfaceExtensions()));

        ASSERT_ANY_THROW(manager.createRenderingDevice(Engine3DManager::getDeviceExtensions(), mandatoryFeatures, *manager.getSurfaces().at(idSurface)));

        ASSERT_NO_THROW(manager.release());
        EXPECT_EQ(0, manager.getDevices().size());
        EXPECT_EQ(1, manager.getSurfaces().size());
    }

    // Delete window -> signal to remove into manager
    ASSERT_NO_THROW(releaseWindow(dialog));
    EXPECT_EQ(0, manager.getSurfaces().size()) << "Window is automatically unregistered";
}

TEST_F(Engine3DManagerTest, manualBuildRendering)
{
    RT::RenderDialogWPtr dialog = createWindow(BT::String(u8"Engine3DManagerTest::buildRendering"));

    Engine3DManager manager;

    // Add dialog to manage (don't care about Engine3DManager initialization)
    BT::uint32 idSurface = 0;
    ASSERT_NO_THROW(idSurface = manager.addRenderDialog(dialog));
    EXPECT_EQ(1, manager.getSurfaces().size());

    // Simple initialization manual
    {
        ASSERT_NO_THROW(manager.initialize(g_info, Engine3DManager::getSurfaceExtensions()));

        Vulkan::SurfaceFormat::Configuration configuration;
        configuration._presentMode = VK_PRESENT_MODE_MAILBOX_KHR;

        Vulkan::ContextDeviceWPtr context;
        ASSERT_NO_THROW(context = manager.createRenderingDevice(Engine3DManager::getDeviceExtensions(), mandatoryFeatures, *manager.getSurfaces().at(idSurface)));
        ASSERT_NO_THROW(manager.createWindowSurface(context, idSurface, configuration));
        
        ASSERT_NO_THROW(manager.release());
    }

    // Delete window -> signal to remove into manager
    ASSERT_NO_THROW(releaseWindow(dialog));
    EXPECT_EQ(0, manager.getSurfaces().size()) << "Window is automatically unregistered";
}

TEST_F(Engine3DManagerTest, numberOrLiteral)
{
    Engine3DManager manager;

    // Be sure nothing is broken 
    loadEngine(manager, "NumberOrLiteral.xml");

    EXPECT_EQ(VK_PRESENT_MODE_MAILBOX_KHR, manager.getContextWindows().at(0)->getFormat().getConfiguration()._presentMode);
    EXPECT_EQ(VK_PRESENT_MODE_MAILBOX_KHR, manager.getContextWindows().at(1)->getFormat().getConfiguration()._presentMode);

    // Clean
    ASSERT_NO_THROW(manager.release());
    clearDialog(manager);
}

/// Allocate all objects and all valid cases (but do something incoherent)
TEST_F(Engine3DManagerTest, fullDescriptor)
{
    Engine3DManager manager;

    // Be sure nothing is broken 
    loadEngine(manager, "FullDescriptor.xml");

    // Clean
    ASSERT_NO_THROW(manager.release());
    clearDialog(manager);
}

TEST_F(Engine3DManagerTest, loadWindowCreation)
{
    Engine3DManager manager;

    EXPECT_EQ(0, manager.getSurfaces().size());

    loadEngine(manager, "WindowCreate.xml");

    CheckEvents event;
    manager.getAfterClose().connectMember(&event, &CheckEvents::close);

    // Find new dialog
    auto dialog = manager.getSurfaces().at(0)->_linkWindow;
    
    EXPECT_EQ(1, manager.getSurfaces().size());
    EXPECT_EQ(1, manager.getDevices().size());

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(2s);
    ASSERT_NO_THROW(manager.release());
    EXPECT_EQ(0, manager.getDevices().size());

    // Delete window -> signal to remove into manager
    EXPECT_FALSE(event._close);
    ASSERT_NO_THROW(releaseWindow(dialog));
    EXPECT_TRUE(event._close);

    EXPECT_EQ(0, manager.getSurfaces().size()) << "Window is automatically unregistered";
}

TEST_F(Engine3DManagerTest, loadWindowMonitorCreation)
{
    Engine3DManager manager;

    EXPECT_EQ(0, manager.getSurfaces().size());

    loadEngine(manager, "WindowCreateMonitor.xml");

    // Find new dialog
    auto dialog = manager.getSurfaces().at(0)->_linkWindow;

    EXPECT_EQ(1, manager.getSurfaces().size());
    EXPECT_EQ(1, manager.getDevices().size());

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(2s);
    ASSERT_NO_THROW(manager.release());
    EXPECT_EQ(0, manager.getDevices().size());

    // Delete window -> signal to remove into manager
    ASSERT_NO_THROW(releaseWindow(dialog));
    EXPECT_EQ(0, manager.getSurfaces().size()) << "Window is automatically unregistered";
}

TEST_F(Engine3DManagerTest, Error)
{
    const std::vector<BT::String> files =
    {
        "CommandError.xml",
        "CommandBeginRenderPassError.xml",
        "CommandBeginRenderPassError1.xml",
        "DeviceError.xml",
        "DeviceError1.xml",
        "DeviceSemaphoreError.xml",
        "DeviceSurfaceError.xml",
        "DeviceSurfaceError1.xml",
        "EnvironmentError.xml",
        "FenceError.xml",
        "FormatError.xml",
        "FrameBufferError.xml",
        "FrameBufferError1.xml",
        "FrameBufferError2.xml",
        "GraphicsPipelineError.xml",
        "GraphicsPipelineError1.xml",
        "ImageError.xml",
        "ImageError1.xml",
        "ImageError2.xml",
        "LiteralEnumError.xml",
        "RenderPassError.xml",
        "RenderPassError1.xml",
        "RenderPassError2.xml",
        "RenderPassError3.xml",
        "RenderPassError4.xml",
        "RenderPassError5.xml",
        "RenderPassError6.xml",
        "RenderPassError7.xml",
        "SequenceError.xml",
        "SequenceError1.xml",
        "SequenceError2.xml",
        "SequenceError3.xml",
        "VersionError.xml",
        "WindowError1.xml",
        "WindowError2.xml",
        "WindowError3.xml"
    };

    for (const auto& fileName : files)
    {
        Engine3DManager manager;
        
        // Build path
        std::filesystem::path pathFile(MouCaEnvironment::getInputPath());
        pathFile = pathFile / "Renderer" / fileName;

        // Read resource
        XML::ParserSPtr xmlFile = _environment.getCore()->getResourceManager().openXML(pathFile);
        xmlFile->openXMLFile();
        ASSERT_TRUE(xmlFile->isLoaded()) << "Impossible to read XML";

        // MUST failed
        Engine3DXMLLoader loader(manager);
        MouCa3DEngine::Engine3DXMLLoader::ContextLoading context(_environment.get3DEngine(), *xmlFile);
        ASSERT_ANY_THROW(loader.load(context));

        // Release resource (no needed anymore)
        _environment.getCore()->getResourceManager().releaseResource(xmlFile);

        ASSERT_NO_THROW(manager.release());

        ASSERT_NO_THROW(clearDialog(manager));
    }
}

TEST_F(Engine3DManagerTest, windowEvent)
{
    Engine3DManager manager;

    MouCa3DEngine::Engine3DXMLLoader loader(manager);
    ASSERT_NO_FATAL_FAILURE(loadEngine(loader, u8"TriangleScreenSpace.xml"));

    // Get allocated item
    auto context = manager.getDevices().at(0);
    context->getDevice().waitIdle();

    auto queueSequences = context->getQueueSequences();
    ASSERT_EQ(1, queueSequences.size());

    auto dialog = loader._dialogs.at(0).lock();
    
    CheckEvents event;
    manager.getAfterResize().connectMember(&event, &CheckEvents::resize);

    // Run one frame
    {
        for (const auto& sequence : *queueSequences.at(0))
        {
            ASSERT_EQ(VK_SUCCESS, sequence->execute(context->getDevice()));
        }
        takeScreenshot(manager, L"3DManager-normal.png");
    }

    // Minimized
    {
        EXPECT_FALSE(event._resize);
        dialog->setStateSize(RT::Window::Iconified);

        // Impossible to render
        const auto& sequence = *(*queueSequences.at(0)).begin();
        ASSERT_NE(VK_SUCCESS, sequence->execute(context->getDevice()));
    }
    
    // Restore
    {
        dialog->setStateSize(RT::Window::Normal);
        dialog->setVisibility(false); //Not mandatory but avoid to popup dialog on test machine

        for (const auto& sequence : *queueSequences.at(0))
        {
            ASSERT_EQ(VK_SUCCESS, sequence->execute(context->getDevice()));
        }
        takeScreenshot(manager, L"3DManager-normal.png");

        EXPECT_TRUE(event._resize);
    }

    // New size
    {
        loader._dialogs.at(0).lock()->setSize(400, 400);

        for (const auto& sequence : *queueSequences.at(0))
        {
            ASSERT_EQ(VK_SUCCESS, sequence->execute(context->getDevice()));
        }
        takeScreenshot(manager, L"3DManager-400.png");
    }

    manager.getAfterResize().disconnect(&event);

    dialog.reset(); // Release lock !

    // Clean
    ASSERT_NO_THROW(manager.release());

    // Clean allocate dialog
    ASSERT_NO_THROW(clearDialog(manager));
}

TEST_F(Engine3DManagerTest, shaderEvent)
{
    const auto source = MouCaEnvironment::getOutputPath() / u8"f_ShaderEdition.frag";
    {
        // Copy shader
        const auto folderShader = _environment.getCore()->getResourceManager().getResourceFolder(MouCaCore::IResourceManager::Shaders);
        const auto folderSource = _environment.getCore()->getResourceManager().getResourceFolder(MouCaCore::IResourceManager::ShadersSource);

        const auto spirV  = MouCaEnvironment::getOutputPath() / u8"f_ShaderEdition.spv";
        ASSERT_TRUE(std::filesystem::copy_file(folderSource / u8"f_triangleSS.frag", source,
                    std::filesystem::copy_options::overwrite_existing));

        // Compile to produce SPIR-V
        RT::ShaderFile shader(spirV, RT::ShaderKind::Fragment, source);
        shader.compile();

        ASSERT_TRUE(std::filesystem::exists(spirV));
        ASSERT_TRUE(std::filesystem::exists(source));
    }

    Engine3DManager manager;

    // Set current path to output
    std::filesystem::current_path(MouCaEnvironment::getOutputPath());

    MouCa3DEngine::Engine3DXMLLoader loader(manager);
    ASSERT_NO_FATAL_FAILURE(loadEngine(loader, u8"ShaderEdition.xml"));

    // Get allocated item
    auto context = manager.getDevices().at(0);
    context->getDevice().waitIdle();

    // Run one frame
    {
        for (const auto& sequence : *context->getQueueSequences().at(0))
        {
            ASSERT_EQ(VK_SUCCESS, sequence->execute(context->getDevice()));
        }
        takeScreenshot(manager, L"3DManager-normal.png");
    }

    enableFileTracking(manager);

    // Create simple sync
    CheckEvents sync;
    auto& signalSync = manager.getAfterShaderChanged();
    signalSync.connectMember(&sync, &CheckEvents::edition);

    // Edit tracked shader
    const std::string newSource = u8"#version 450\n#extension GL_ARB_separate_shader_objects : enable\nlayout(location = 0) in vec3 fragColor;\nlayout(location = 0) out vec4 outColor;\nvoid main()\n{\noutColor = vec4(1.0, 1.0, 0.0, 1.0);\n}";
    BT::File editShader(source);
    editShader.open(L"w+");
    editShader.write(newSource.c_str(), newSource.size());
    editShader.close();
    
    // Wait sync
    while(!sync._edition)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    signalSync.disconnect(&sync);

    // Run one frame
    {
        for (const auto& sequence : *context->getQueueSequences().at(0))
        {
            ASSERT_EQ(VK_SUCCESS, sequence->execute(context->getDevice()));
        }
        takeScreenshot(manager, L"3DManager-shaderEdit.png");
    }

    disableFileTracking(manager);

    // Clean
    ASSERT_NO_THROW(manager.release());

    // Clean allocate dialog
    ASSERT_NO_THROW(clearDialog(manager));
}

}