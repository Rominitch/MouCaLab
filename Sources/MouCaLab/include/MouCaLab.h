/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibRT/include/RTImage.h>
#include <LibRT/include/RTRenderDialog.h>

#include <MouCaCore/include/CoreSystem.h>
#include <MouCaCore/include/LoaderManager.h>

#include <MouCaGraphicEngine/include/GraphicEngine.h>
#include <MouCaGraphicEngine/include/VulkanManager.h>
#include <MouCaGraphicEngine/include/Engine3DXMLLoader.h>
#include <MouCaGraphicEngine/include/Engine3DTransfer.h>
#include <MouCaGraphicEngine/include/ImGUIManager.h>

#include "include/EventManager3D.h"

namespace RT
{
    class EventManager;
    using EventManagerSPtr = std::shared_ptr<EventManager>;
}

class MouCaLabTest : public ::testing::Test
{
    MOUCA_NOCOPY_NOMOVE(MouCaLabTest);

    public:
        MouCaLabTest();
        ~MouCaLabTest() override = default;

        void SetUp() override;
        void TearDown() override;

        /// 
        /// \note Call before createWindow()
        virtual void configureEventManager();
        void releaseEventManager();

        void loadEngine(MouCaGraphic::VulkanManager& manager, const Core::String& fileName);
        void loadEngine(MouCaGraphic::Engine3DXMLLoader& loader, const Core::String& fileName);

        void mainLoop(MouCaGraphic::VulkanManager& manager, const Core::String& title, const std::function<void(const double timer)>& afterRender = [](const double) {});

        void takeScreenshot(MouCaGraphic::VulkanManager& manager, const Core::Path& imageFile, const size_t nbMaxDefectPixels = 500, const double maxDistance4D = 30);

        void enableFileTracking(MouCaGraphic::VulkanManager& manager);
        void disableFileTracking(MouCaGraphic::VulkanManager& manager);

        void clearDialog(MouCaGraphic::VulkanManager& manager);
    protected:
        MouCaCore::CoreSystem       _core;      ///< [OWNERSHIP] Core managers;
        MouCaGraphic::GraphicEngine _graphic;   ///< [OWNERSHIP] Graphic engine: Vulkan + VR + GLFW + imGui;

        VkPhysicalDeviceFeatures _mandatoryFeatures;
        RT::EventManagerSPtr     _eventManager;
};

    
/*
    RT::RenderDialogWPtr createWindow(const BT::String& title)
    {
#ifndef NDEBUG
        static const RT::Window::Mode mode = RT::Window::StaticDialogMode;
#else
#   ifndef DEVELOPPER_MODE
        static const RT::Window::Mode mode = RT::Window::InternalMode;
#   else
        static const RT::Window::Mode mode = RT::Window::StaticDialogMode;
#   endif
#endif
        const RT::ViewportInt32 viewport(0, 0, 1500, 1000);

        RT::RenderDialogWPtr _window = _environment.get3DEngine().getCanvasManager().createCanvas(viewport, title, mode);
        _window.lock()->initialize(_eventManager, viewport.getSize());

        return _window;
    }

    void releaseWindow(RT::RenderDialogWPtr window)
    {
        _environment.get3DEngine().getCanvasManager().releaseCanvas(window);

        BT_POST_CONDITION(window.expired());
    }
* /

    void clearDialog(MouCa3DEngine::Engine3DManager& manager)
    {
        // Clean allocate dialog
        bool empty = manager.getSurfaces().empty();
        while (!empty)
        {
            auto& surface = *manager.getSurfaces().begin();

            ASSERT_NO_THROW(releaseWindow(surface->_linkWindow));
            empty = manager.getSurfaces().empty();
        }
    }
*/