/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace MouCaVR
{
    struct TrackedDevice
    {
        MOUCA_NOCOPY_NOMOVE(TrackedDevice);

        bool                    _isConnected;
        vr::ETrackedDeviceClass _kind;
        Core::String              _name;
        Core::String              _model;
        RT::Transform           _transform;

        vr::VRControllerState_t _state;

        TrackedDevice():
        _isConnected(false), _kind(vr::TrackedDeviceClass_Invalid)
        {}
    };
}