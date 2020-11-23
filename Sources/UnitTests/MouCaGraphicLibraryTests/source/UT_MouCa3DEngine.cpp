#include "Dependancies.h"

#include <MouCaCore/interface/ICoreSystem.h>

#include <MouCa3DEngine/interface/ICanvasManager.h>
#include <MouCa3DEngine/interface/IEngine3DLoader.h>
#include <MouCa3DEngine/interface/IGraphicConfiguration.h>

#include <include/MouCa3DEngineTest.h>


TEST(MouCa3DEngine, loadDLL)
{
    //Create Core application
    std::shared_ptr<MouCaCore::ICoreSystem> core(MouCaCore::ICoreSystem::launchCore());

    std::shared_ptr<MouCaCore::IEnvironment> environment(new MouCaCore::IEnvironment());
    ASSERT_NO_THROW(environment->initialize(core.get(), MouCaCore::IEnvironment::NbPlugIn));

    //Load 3D Engine plugin
    MouCa3DEngine::IEngine3DLoader* engine3D = nullptr;
    ASSERT_NO_THROW(engine3D = dynamic_cast<MouCa3DEngine::IEngine3DLoader*>(core->getPlugInManager().loadDynamicLibrary(L"MouCa3DEngine.dll")));

    // Test methods
    const int argc=0;
    const char** argv = nullptr;
    ASSERT_NO_THROW(engine3D->setApplicationArguments(argc, argv));

    ASSERT_NO_THROW(engine3D->setupEnvironment(environment));

    ASSERT_NO_THROW(engine3D->initialize());

    EXPECT_EQ(environment, engine3D->getEnvironment());

    ASSERT_NO_THROW(engine3D->getCanvasManager());
    
    ASSERT_NO_THROW(engine3D->release());
}

TEST_F(MouCa3DEngineTest, canvasManager)
{
    MouCa3DEngine::ICanvasManager& manager = _gameEnv.get3DEngine().getCanvasManager();

    const RT::ViewportInt32 viewport(0, 0, 200, 200);
    RT::RenderDialogWPtr canvas;
    ASSERT_NO_THROW(canvas = manager.createCanvas(viewport, "GoogleTest - CanvasManager", RT::Window::InternalMode));

    manager.releaseCanvas(canvas);

    ASSERT_TRUE(canvas.expired());
}

TEST_F(MouCa3DEngineTest, getDefaultConfiguration)
{
    MouCa3DEngine::IGraphicConfiguration configuration;
    _gameEnv.get3DEngine().getDefaultConfiguration(configuration);

#ifndef NDEBUG
    EXPECT_EQ(RT::Window::StaticDialogMode,   configuration.getMode());
#else
    EXPECT_EQ(RT::Window::FullscreenWindowed, configuration.getMode());
#endif
    EXPECT_FALSE(configuration.getWindowConfiguration().empty());
}