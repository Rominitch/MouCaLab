#include "Dependancies.h"

#include <LibRT/include/RTRenderDialog.h>
#include <MouCa3DEngine/interface/IRendererManager.h>

#include "include/EventManager3D.h"
#include "include/MouCa3DEngineTest.h"

class RendererManager : public MouCa3DEngineTest
{
    public:
        RendererManager() = default;
        ~RendererManager() override = default;
};

TEST_F(RendererManager, registerRendering)
{
    const RT::Array2ui resolution(200, 200);
    RT::RenderDialogWPtr canvas = _gameEnv.get3DEngine().getCanvasManager().createCanvas(RT::ViewportInt32(0, 0, resolution.x, resolution.y), u8"RendererManager.RegisterRendering", RT::Window::InternalMode);

    auto eventManager = std::make_shared<EventManager3DOld>();
    canvas.lock()->initialize(eventManager, resolution);

    MouCa3DEngine::IRendererManager& rendererManager = _gameEnv.get3DEngine().getRendererManager();

    MouCa3DEngine::IRenderingSystemWPtr system;
    ASSERT_NO_THROW(system = rendererManager.registerRenderingSystem(canvas));

    {
        auto systemLock = system.lock();

        auto rendererId = MouCa3DEngine::IRenderingSystem::Deferred;
        RT::RendererWPtr renderer;
        ASSERT_NO_THROW(renderer = systemLock->buildRenderer(rendererId, resolution));
        ASSERT_ANY_THROW(systemLock->buildRenderer(MouCa3DEngine::IRenderingSystem::NbRenderers, resolution));

        ASSERT_NO_THROW(systemLock->releaseRenderer(rendererId));
    }

    ASSERT_NO_THROW(rendererManager.unregisterRenderingSystem(system));

    ASSERT_NO_THROW( _gameEnv.get3DEngine().getCanvasManager().releaseCanvas(canvas));
}