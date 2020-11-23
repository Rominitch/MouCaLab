#pragma once

#include <LibRT/include/RTScene.h>

#include "include/MouCa3DEngineTest.h"

namespace RT
{
    class RenderDialog;
    using RenderDialogWPtr = std::weak_ptr<RenderDialog>;
}

class MouCaBasicScene : public MouCa3DDeferredTest
{
protected:
    RT::Scene               _scene;     ///< Main scene.
    RT::RenderDialogWPtr    _window;    ///< Weak link to window.

public:
    MouCaBasicScene() = default;

    ~MouCaBasicScene() override;

    void SetUp() override;

    void TearDown() override;

    ///------------------------------------------------------------------------
    /// \brief  Replace default renderer by deferred.
    /// 
    void createRenderer();

    ///------------------------------------------------------------------------
    /// \brief  Build scene
    /// 
    void makeScene(const BT::StringOS& mainFolder);
};