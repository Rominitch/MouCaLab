#pragma once

#include <LibRT/include/RTEventManager.h>

class EventManagerEmpty : public RT::EventManager
{
public:
    void onClose(RT::Canvas*) override
    {}

    void onSize(RT::Canvas*, const RT::Array2ui&) override
    {}

    void onMouseMove(RT::Canvas*, const RT::VirtualMouse&) override
    {}

    void onMousePress(RT::Canvas*, const RT::VirtualMouse&, const RT::VirtualMouse::Buttons) override
    {}

    void onMouseRelease(RT::Canvas*, const RT::VirtualMouse&, const RT::VirtualMouse::Buttons) override
    {}

    void onMouseWheel(RT::Canvas*, const RT::VirtualMouse&, const int) override
    {}

    void onKeyPress(RT::Canvas*, int, int, int, int) override
    {}
};
