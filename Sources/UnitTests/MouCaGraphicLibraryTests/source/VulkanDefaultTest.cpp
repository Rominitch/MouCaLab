#include "Dependancies.h"
/*
#include <LibCore/include/CoreFile.h>

#include <LibRT/include/RTCamera.h>
#include <LibRT/include/RTCameraManipulator.h>
#include <LibRT/include/RTEventManager.h>
#include <LibRT/include/RTImage.h>
#include <LibRT/include/RTMesh.h>

#include <LibGLFW/include/GLFWWindow.h>

#include <LibVulkan/include/VKContextManager.h>
#include <LibVulkan/include/VKCommandBuffer.h>
#include <LibVulkan/include/VKCommandPool.h>
#include <LibVulkan/include/VKDeviceContext.h>
#include <LibVulkan/include/VKDevice.h>
#include <LibVulkan/include/VKFrameBuffer.h>
#include <LibVulkan/include/VKRenderPass.h>
#include <LibVulkan/include/VKRendererMultisample.h>
#include <LibVulkan/include/VKPipelineCache.h>
#include <LibVulkan/include/VKGraphicsPipeline.h>
#include <LibVulkan/include/VKPipelineLayout.h>
#include <LibVulkan/include/VKPipelineStates.h>
#include <LibVulkan/include/VKScreenFrame.h>
#include <LibVulkan/include/VKScreenshot.h>
#include <LibVulkan/include/VKShaderProgram.h>
#include <LibVulkan/include/VKSurface.h>
#include <LibVulkan/include/VKSurfaceFormat.h>
#include <LibVulkan/include/VKSwapChain.h>

#include <MouCaCore/interface/ILoaderManager.h>
#include <MouCaCore/interface/IResourceManager.h>

#include "include/VulkanDefaultTest.h"

void VulkanDefaultTest::SetUp()
{
    VulkanTest::SetUp();

    _eventManager = std::make_shared<EventManager3DOld>();

    const RT::ViewportInt32 cameraDef(0, 0, _resolution.x, _resolution.y);

    // Build camera
    _camera = std::make_shared<RT::Camera>();
    _camera->computePerspectiveCamera(cameraDef);
    _camera->getOrientation().setPosition(RT::Point3(2.5f, 2.5f, -7.5f));
    // Give ownership to scene
    _scene.addCamera(_camera);
    
    _trackball = std::make_shared<RT::CameraManipulator>();
    _trackball->initialize(_camera);
    _trackball->installComportement(RT::CameraManipulator::TrackBallCamera);

    _flying = std::make_shared<RT::CameraManipulator>();
    _flying->initialize(_camera);
    _flying->installComportement(RT::CameraManipulator::FlyingCamera);

    // Register into event Manager
    ASSERT_NO_THROW(_eventManager->initialize(_trackball, _camera));
    ASSERT_NO_THROW(_eventManager->addManipulator(_flying));

    // Read Window data
    window = _platform.getWindow(0);
    window->initialize(_eventManager, _resolution);

    VkPhysicalDeviceFeatures enabled = {};
    setVulkanPhysicalDeviceFeatures(enabled);

    // Attach rendering system to window
    _deviceContext.initialize(_environment, window, enabled);

#ifdef VULKAN_DEBUG
    _environment.setDeviceInReport(_deviceContext.getDevice().getInstance());
#endif
}

void VulkanDefaultTest::createRenderer()
{
    MOUCA_ASSERT(!_scene.getCameras().empty());
    // Create renderer
    {
        auto render = std::make_shared<Vulkan::RendererMultiSample>(&_deviceContext, MouCaEnvironment::getInputPath());
        ASSERT_NO_THROW(render->computeRenderer(_deviceContext, { _resolution.x, _resolution.y }));
        ASSERT_FALSE(render->isNull());

        // Insert renderer into viewer/context
        _renderer = Vulkan::RendererSPtr(render);
        ASSERT_NO_THROW(_deviceContext.addMainRenderer(_renderer, _resolution, 1));
        ASSERT_NO_THROW(_eventManager->setRenderer(_renderer));
    }

    // Sync device (now data and all are ready to render)
    _deviceContext.getDevice().waitIdle();
}

void VulkanDefaultTest::TearDown()
{
    ASSERT_NO_THROW(_scene.release());

    // Leave pointer to viewer (manage delete).
    _renderer.reset();

    // Clean image
    if(_colorMap != nullptr)
    {
        _colorMap->release(_deviceContext.getDevice());
    }

    // Simulate manual closing
    if(window != nullptr)
    {
        GLFWwindow* hander = window->getGLFWHandler();
        ASSERT_NO_THROW(window.reset()); // Need to release local instance now !
        ASSERT_NO_THROW(GLFW::Window::onDemandClose(hander));
    }

    //Clean context
    ASSERT_NO_THROW(_deviceContext.release(_environment));

    ASSERT_NO_THROW(_eventManager.reset());
    ASSERT_NO_THROW(_trackball.reset());
    ASSERT_NO_THROW(_flying.reset());

    VulkanTest::TearDown();
}

void VulkanDefaultTest::takeScreenshot(const Core::Path& imageFile, const size_t nbMaxDefectPixels, const double maxDistance4D)
{
    Vulkan::Screenshot screenshot;
    ASSERT_NO_THROW(screenshot.initialize(_deviceContext));

    {
        auto& resourceManager = _core->getResourceManager();
        auto& loaderManager   = _core->getLoaderManager();

        const size_t latestImage = 0;
        auto diskImage = resourceManager.createImage();
        ASSERT_NO_THROW(screenshot.saveTo(*diskImage->getImage().lock(), MouCaEnvironment::getOutputPath() / imageFile, _deviceContext, latestImage));

        ASSERT_NO_THROW(screenshot.release(_deviceContext.getDevice()));

        auto refImage = resourceManager.openImage(MouCaEnvironment::getInputPath() / L"references" / imageFile);
        {
            MouCaCore::LoadingItems items =
            {
                MouCaCore::LoadingItem(refImage, MouCaCore::LoadingItem::Direct),
            };
            // Start loading
            ASSERT_NO_THROW(loaderManager.loadResources(items));
        }

        // Compare images
        size_t nbDefect;
        double maxFoundDistance;
        const bool compare = diskImage->getImage().lock()->compare(*refImage->getImage().lock(), nbMaxDefectPixels, maxDistance4D, &nbDefect, &maxFoundDistance);
        if (!compare) // Have you update the reference ? Or bug ?
        {
            const std::filesystem::path sourceFile(MouCaEnvironment::getOutputPath() / imageFile);
            const std::filesystem::path targetParent(MouCaEnvironment::getOutputPath() / L".." / L".." / L".." / L"Report");
            const std::filesystem::path targetFile(targetParent / imageFile);

            // Duplicate result into failure folder
            if (!std::filesystem::exists(targetParent))
                ASSERT_NO_THROW(std::filesystem::create_directories(targetParent)); // Recursively create target directory if not existing.
            ASSERT_NO_THROW(std::filesystem::copy_file(sourceFile, targetFile, std::filesystem::copy_options::overwrite_existing));

            EXPECT_TRUE(compare) << u8"Image comparison failed: " << imageFile << u8"\n"
                                 << u8"Defect pixel: " << nbDefect << " > " << nbMaxDefectPixels << u8"\n"
                                 << u8"With tolerance of " << maxDistance4D << u8"(max found: " << maxFoundDistance << u8")";
        }

        resourceManager.releaseResource(diskImage);
        resourceManager.releaseResource(refImage);
    }
}
*/