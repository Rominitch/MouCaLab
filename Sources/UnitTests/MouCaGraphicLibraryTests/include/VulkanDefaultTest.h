#pragma once
/*
#include <LibGLFW/include/GLFWWindow.h>

#include <LibVulkan/include/VKDeviceContext.h>
#include <LibVulkan/include/VKImage.h>

#include <LibRT/include/RTScene.h>
#include <LibRT/include/RTViewer.h>

#include <MouCaGraphicEngine/include/Engine3DLoader.h>

#include "include/EventManager3D.h"
#include "include/VulkanTest.h"

namespace RT
{
    class Camera;
    typedef std::shared_ptr<Camera> CameraSPtr;

    class CameraManipulator;
    typedef std::shared_ptr<CameraManipulator> CameraManipulatorSPtr;
}

typedef std::shared_ptr<EventManager3DOld> EventManager3DOldSPtr;

class VulkanDefaultTest : public VulkanTest
{
    public:
        GLFW::WindowSPtr            window;
        EventManager3DOldSPtr          _eventManager;
        RT::CameraManipulatorSPtr   _trackball;
        RT::CameraManipulatorSPtr   _flying;
        RT::CameraSPtr              _camera;

        RT::Scene                   _scene;

        // Just link
        Vulkan::DeviceContext _deviceContext;
        Vulkan::RendererSPtr  _renderer;
        Vulkan::ImageOldSPtr  _colorMap;

        VulkanDefaultTest(const std::string& windowName = "VulkanTest") :
        VulkanTest(windowName)
        {}

        virtual void createRenderer();

        void createMultiSampledMesh();

        virtual void SetUp();

        virtual void TearDown();

        virtual void setVulkanPhysicalDeviceFeatures(VkPhysicalDeviceFeatures&)
        {}

        void takeScreenshot(const Core::Path& imageFile, const size_t nbMaxDefectPixels = 30, const double maxDistance4D = 3.0);
};
*/