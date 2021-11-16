/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibRT/include/RTEventManager.h>

// Forward declaration
namespace RT
{
    class Camera;
    using CameraSPtr = std::shared_ptr<Camera>;

    class CameraComportement;
    class CameraManipulator;
    using CameraManipulatorSPtr = std::shared_ptr<CameraManipulator>;
}

class EventManager3D : public RT::EventManager
{
    public:
        using Manipulators = std::vector<RT::CameraManipulatorSPtr>;

        EventManager3D();
        ~EventManager3D() override = default;

        void addManipulator(RT::CameraManipulatorSPtr& cameraManipulator);

        void onClose(RT::Canvas* pCanvas) override;

        void onSize(RT::Canvas* pCanvas, const RT::Array2ui& size) override;

        void onMouseMove(RT::Canvas* pCanvas, const RT::VirtualMouse& mouse) override;

        void onMousePress(RT::Canvas* pCanvas, const RT::VirtualMouse& mouse, const RT::VirtualMouse::Buttons eButton) override;
        void onMouseRelease(RT::Canvas* pCanvas, const RT::VirtualMouse& mouse, const RT::VirtualMouse::Buttons eButton) override;

        void onMouseWheel(RT::Canvas* pCanvas, const RT::VirtualMouse& mouse, const int positionRelease) override;

        void onKeyPress(RT::Canvas* pCanvas, int key, int scancode, int action, int mods) override;

        void setSpeedWheel(float speedWheel)
        {
            MouCa::preCondition(speedWheel > 0.0f);
            _speedWheel = speedWheel;
        }

    protected:
        float _speedWheel = 0.01f;
        RT::CameraComportement* getComportement() const;

        Manipulators            _manipulators;
        Manipulators::iterator  _current;
};

using EventManager3DSPtr = std::shared_ptr<EventManager3D>;
using EventManager3DWPtr = std::weak_ptr<EventManager3D>;

class EventManager3DText : public EventManager3D
{
public:
    EventManager3DText();
    ~EventManager3DText() override = default;

    void onMouseMove(RT::Canvas* pCanvas, const RT::VirtualMouse& mouse) override;
};

using EventManager3DTextSPtr = std::shared_ptr<EventManager3DText>;
using EventManager3DTextWPtr = std::weak_ptr<EventManager3DText>;
