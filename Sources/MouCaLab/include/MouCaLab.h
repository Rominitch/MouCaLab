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

        void updateCommandBuffers(MouCaGraphic::Engine3DXMLLoader& loader, const VkCommandBufferResetFlags reset = VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
        void updateCommandBuffersSurface(MouCaGraphic::Engine3DXMLLoader& loader, const VkCommandBufferResetFlags reset=VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

        void clearDialog(MouCaGraphic::VulkanManager& manager);
    protected:
        MouCaCore::CoreSystem       _core;      ///< [OWNERSHIP] Core managers;
        MouCaGraphic::GraphicEngine _graphic;   ///< [OWNERSHIP] Graphic engine: Vulkan + VR + GLFW + imGui;

        RT::EventManagerSPtr     _eventManager;
};