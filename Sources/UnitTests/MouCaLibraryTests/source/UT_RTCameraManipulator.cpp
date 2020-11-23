#include "Dependancies.h"

#include <LibRT/include/RTCamera.h>
#include <LibRT/include/RTCameraManipulator.h>

namespace RT
{

TEST(RTCameraManipulator, install)
{
    CameraSPtr camera = std::make_shared<Camera>();
    {
        CameraManipulator manipulator;

        manipulator.initialize(camera);

        manipulator.installComportement(CameraManipulator::StaticCamera);

        manipulator.installComportement(CameraManipulator::TrackBallCamera);

        manipulator.installComportement(CameraManipulator::FlyingCamera);

        manipulator.installComportement(CameraManipulator::AvatarCamera);
    }
}

}