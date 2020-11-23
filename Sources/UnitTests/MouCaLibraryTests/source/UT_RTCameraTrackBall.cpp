#include "Dependancies.h"

#include <LibRT/include/RTCamera.h>
#include <LibRT/include/RTCameraComportement.h>
#include <LibRT/include/RTObject.h>

TEST(CameraTrackBall, movement)
{
    const float epsilon = 1.1e-3f;
    RT::CameraSPtr camera = std::make_shared<RT::Camera>();
    camera->getOrientation().setPosition(RT::Point3(0.0f, 0.0f, 10.0f));

    RT::CameraTrackBall trackball;
    ASSERT_NO_THROW(trackball.attachCamera(camera));

    // Rotate
    trackball.rotate(RT::Vector2(Core::Maths::PI<float>*0.5f, Core::Maths::PI<float>*0.5f));
    {
        const RT::Matrix4& mat = camera->getOrientation().getWorldToLocal().convert();
        EXPECT_VEC4_NEAR(RT::Point4(1.0f,  0.0f,   0.0f, 0.0f), mat[0], epsilon);
        EXPECT_VEC4_NEAR(RT::Point4(0.0f,  0.0f,   1.0f, 0.0f), mat[1], epsilon);
        EXPECT_VEC4_NEAR(RT::Point4(0.0f, -1.0f,   0.0f, 0.0f), mat[2], epsilon);
        EXPECT_VEC4_NEAR(RT::Point4(0.0f,  0.0f, -10.0f, 1.0f), mat[3], epsilon);
    }

    // Zoom
    trackball.zoom(10.0f);
    {
        const RT::Matrix4& mat = camera->getOrientation().getWorldToLocal().convert();
        EXPECT_VEC4_NEAR(RT::Point4(1.0f,  0.0f,   0.0f, 0.0f), mat[0], epsilon);
        EXPECT_VEC4_NEAR(RT::Point4(0.0f,  0.0f,   1.0f, 0.0f), mat[1], epsilon);
        EXPECT_VEC4_NEAR(RT::Point4(0.0f, -1.0f,   0.0f, 0.0f), mat[2], epsilon);
        EXPECT_VEC4_NEAR(RT::Point4(0.0f,  0.0f, -20.0f, 1.0f), mat[3], epsilon);
    }

    // Move To
    RT::CameraTrackBall::CameraOrientation orientation = {{ 0.0f, 0.0f, 10.0f }};
    trackball.moveTo(orientation);
    {
        const RT::Matrix4& mat = camera->getOrientation().getWorldToLocal().convert();
        EXPECT_VEC4_NEAR(RT::Point4(1.0f, 0.0f,   0.0f, 0.0f), mat[0], epsilon);
        EXPECT_VEC4_NEAR(RT::Point4(0.0f, 1.0f,   0.0f, 0.0f), mat[1], epsilon);
        EXPECT_VEC4_NEAR(RT::Point4(0.0f, 0.0f,   1.0f, 0.0f), mat[2], epsilon);
        EXPECT_VEC4_NEAR(RT::Point4(0.0f, 0.0f, -10.0f, 1.0f), mat[3], epsilon);
    }
}