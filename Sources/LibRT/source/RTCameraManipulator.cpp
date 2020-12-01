/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibRT/include/RTCamera.h"
#include "LibRT/include/RTCameraComportement.h"
#include "LibRT/include/RTCameraManipulator.h"

namespace RT
{

CameraManipulator::~CameraManipulator()
{
    delete _comportement;
}

void CameraManipulator::installComportement(const EComportment eState)
{
    delete _comportement;
    switch(eState)
    {
        case StaticCamera:
        {
            _comportement = new CameraStatic();
        }
        break;

        case TrackBallCamera:
        {
            CameraTrackBall* comportement = new CameraTrackBall();
            comportement->attachCamera(_linkCamera.lock());
            _comportement = comportement;
        }
        break;

        case FlyingCamera:
        {
            _comportement = new CameraFlying();
            _comportement->attachCamera(_linkCamera.lock());
        }
        break;

        case AvatarCamera:
        {
            _comportement = new CameraAvatar();
        }
        break;

        default:
        {
            _comportement = new CameraStatic();
        }
    }
}

}