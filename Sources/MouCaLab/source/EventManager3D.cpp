/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
/// 
#include "Dependencies.h"

#include "include/EventManager3D.h"

#include <LibRT/include/RTCamera.h>
#include <LibRT/include/RTCameraManipulator.h>
#include <LibRT/include/RTCameraComportement.h>
#include <LibRT/include/RTCanvas.h>
#include <LibRT/include/RTViewport.h>
#include <LibRT/include/RTVirtualMouse.h>

// DisableCodeCoverage

EventManager3D::EventManager3D():
_current(_manipulators.end())
{}

void EventManager3D::addManipulator(RT::CameraManipulatorSPtr& cameraManipulator)
{
    _manipulators.emplace_back(cameraManipulator);
    _current = _manipulators.begin();
}

void EventManager3D::onClose(RT::Canvas* pCanvas)
{
    MOUCA_PRE_CONDITION(pCanvas != nullptr);
    BT_UNUSED(pCanvas);
}

RT::CameraComportement* EventManager3D::getComportement() const
{
    MOUCA_PRE_CONDITION(!_manipulators.empty());
    MOUCA_PRE_CONDITION(_current != _manipulators.end());

    return (*_current)->getComportement();
}

void EventManager3D::onSize(RT::Canvas* pCanvas, const RT::Array2ui& size)
{
    MOUCA_PRE_CONDITION(pCanvas != nullptr);
    /*
    RT::ViewportInt32 viewport;
    viewport.setSize(size.x, size.y);
    
    RT::Layers& vDrawables = pCanvas->getEditLayerList();
    //Update drawable
    auto itDrawable = vDrawables.begin();
    while (itDrawable != vDrawables.end())
    {
        itDrawable->setDrawingArea(viewport);
        ++itDrawable;
    }
    */
}

void EventManager3D::onMouseWheel(RT::Canvas* pCanvas, const RT::VirtualMouse& mouse, const int deltaWheel)
{
    MOUCA_PRE_CONDITION(!_manipulators.empty());
    MOUCA_PRE_CONDITION(pCanvas != nullptr);
    BT_UNUSED(pCanvas);
    BT_UNUSED(mouse);

    RT::CameraTrackBall* pTrackBall = dynamic_cast<RT::CameraTrackBall*>(getComportement());
    if (pTrackBall != nullptr)
    {
        const float range = pTrackBall->getLimitation()[RT::CameraTrackBall::MaxDepth] - pTrackBall->getLimitation()[RT::CameraTrackBall::MinDepth];
        const float fZoom = static_cast<float>(deltaWheel) * range * _speedWheel;
        pTrackBall->zoom(fZoom);
    }
}

void EventManager3D::onMouseMove(RT::Canvas* pCanvas, const RT::VirtualMouse& mouse)
{
    MOUCA_PRE_CONDITION(!_manipulators.empty());
    MOUCA_PRE_CONDITION(pCanvas != nullptr);
    BT_UNUSED(pCanvas);

    _mouseState._mousePositionSN = mouse.getNormScreenPosition();

    if (mouse.isPressed(RT::VirtualMouse::Buttons::Left))
    {
        const RT::Point2 translate((float)(_mouseState._mousePositionSN.x - _mouseState._lastMousePositionSN.x), (float)(_mouseState._mousePositionSN.y - _mouseState._lastMousePositionSN.y));

        RT::CameraTrackBall* pTrackBall = dynamic_cast<RT::CameraTrackBall*>(getComportement());
        if (pTrackBall != nullptr)
        {
            pTrackBall->rotate(translate);
        }
        else
        {
            RT::CameraFlying* flying = dynamic_cast<RT::CameraFlying*>(getComportement());
            if (flying != nullptr)
            {

                flying->rotate(translate);
            }
        }
    }
    else
    {
        if (mouse.isPressed(RT::VirtualMouse::Buttons::Middle))
        {
            RT::CameraTrackBall* pTrackBall = dynamic_cast<RT::CameraTrackBall*>(getComportement());
            if (pTrackBall != nullptr)
            {

                const float fZoom = (float)(_mouseState._mousePositionSN.y - _mouseState._lastMousePositionSN.y) * 0.1f;
                pTrackBall->zoom(fZoom);
            }
        }
    }

    _mouseState.update();
}

void EventManager3D::onMousePress(RT::Canvas* pCanvas, const RT::VirtualMouse& mouse, const RT::VirtualMouse::Buttons eButton)
{
    MOUCA_PRE_CONDITION(pCanvas != nullptr);
    BT_UNUSED(pCanvas);

    if (RT::VirtualMouse::isPressed(eButton, RT::VirtualMouse::Buttons::Left))
    {
        _mouseState._pickMousePositionSN = mouse.getNormScreenPosition();
    }
}

void EventManager3D::onMouseRelease(RT::Canvas* pCanvas, const RT::VirtualMouse& mouse, const RT::VirtualMouse::Buttons eButton)
{
    MOUCA_PRE_CONDITION(pCanvas != nullptr);
    BT_UNUSED(pCanvas);
    BT_UNUSED(mouse);
    BT_UNUSED(eButton);
}

void EventManager3D::onKeyPress(RT::Canvas* pCanvas, int key, int scancode, int action, int mods)
{
    MOUCA_PRE_CONDITION(pCanvas != nullptr);
    BT_UNUSED(pCanvas);
    BT_UNUSED(scancode);
    BT_UNUSED(action);
    BT_UNUSED(mods);

    RT::CameraFlying* flying = dynamic_cast<RT::CameraFlying*>(getComportement());

    switch (key)
    {
    case GLFW_KEY_0:
    case GLFW_KEY_KP_0:
        _current = _manipulators.begin();
        break;
    case GLFW_KEY_1:
    case GLFW_KEY_KP_1:
        if (_manipulators.size() > 1)
        {
            _current = _manipulators.begin();
            std::advance(_current, 1);
        }   
        break;

    case GLFW_KEY_W:
        if (flying != nullptr)
        {
            flying->move(2.0f);
        }
        break;
    case GLFW_KEY_S:
        if (flying != nullptr)
        {
            flying->move(-2.0f);
        }
        break;
    case GLFW_KEY_D:
        if (flying != nullptr)
        {
            flying->strafing(2.0f);
        }
        break;
    case GLFW_KEY_A:
        if (flying != nullptr)
        {
            flying->strafing(-2.0f);
        }
        break;
    }
}

EventManager3DText::EventManager3DText()
{}

void EventManager3DText::onMouseMove(RT::Canvas* pCanvas, const RT::VirtualMouse& mouse)
{
    if (mouse.isPressed(RT::VirtualMouse::Buttons::Right))
    {
        RT::CameraTrackBall* pTrackBall = dynamic_cast<RT::CameraTrackBall*>(getComportement());
        if (pTrackBall != nullptr)
        {
            _mouseState._mousePositionSN = mouse.getNormScreenPosition();
            auto support = pTrackBall->getSupport().lock();

            const RT::Point2 translate((float)(_mouseState._mousePositionSN.x - _mouseState._lastMousePositionSN.x), (float)(_mouseState._mousePositionSN.y - _mouseState._lastMousePositionSN.y));

            auto position = support->getOrientation().getPosition() + RT::Point3(-translate.x, translate.y, 0.0f);
            support->getOrientation().setPosition(position);
            pTrackBall->refresh();
        }

        _mouseState.update();
    }
    else
        EventManager3D::onMouseMove(pCanvas, mouse);
}

// EnableCodeCoverage