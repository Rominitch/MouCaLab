#include "Dependencies.h"

#include <LibRT/include/RTCamera.h>
#include <LibRT/include/RTCameraComportement.h>
#include <LibRT/include/RTObject.h>

TEST(RTCameraFlying, movement)
{
    const float epsilon = 1.1e-3f;
    RT::CameraSPtr camera = std::make_shared<RT::Camera>();
    camera->getOrientation().setPosition(RT::Point3(0.0f, 0.0f, 10.0f));

    RT::CameraFlying flying;
    ASSERT_NO_THROW(flying.attachCamera(camera));

    // Rotate
    flying.rotate(RT::Vector2(Core::Maths::PI<float>*0.5f, Core::Maths::PI<float>*0.5f));
    {
        const RT::Matrix4& mat = camera->getOrientation().getWorldToLocal().convert();
        EXPECT_VEC4_NEAR(RT::Point4(0.0f,  0.0f,  -1.0f, 0.0f), mat[0], epsilon);
        EXPECT_VEC4_NEAR(RT::Point4(-1.0f, 0.0f,   0.0f, 0.0f), mat[1], epsilon);
        EXPECT_VEC4_NEAR(RT::Point4(0.0f,  1.0f,   0.0f, 0.0f), mat[2], epsilon);
        EXPECT_VEC4_NEAR(RT::Point4(0.0f, -10.0f,  0.0f, 1.0f), mat[3], epsilon);
    }

    // Zoom
    flying.move(10.0f);
    {
        const RT::Matrix4& mat = camera->getOrientation().getWorldToLocal().convert();
        EXPECT_VEC4_NEAR(RT::Point4(0.0f,   0.0f, -1.0f,  0.0f), mat[0], epsilon);
        EXPECT_VEC4_NEAR(RT::Point4(-1.0f,  0.0f,  0.0f,  0.0f), mat[1], epsilon);
        EXPECT_VEC4_NEAR(RT::Point4(0.0f,   1.0f,  0.0f,  0.0f), mat[2], epsilon);
        EXPECT_VEC4_NEAR(RT::Point4(0.0f, -10.0f,  10.0f, 1.0f), mat[3], epsilon);
    }

    // Move To
    flying.moveTo(RT::Point4(1.0f, 20.0f, 30.0f, 1.0f));
    {
        const RT::Matrix4& mat = camera->getOrientation().getWorldToLocal().convert();
        EXPECT_VEC4_NEAR(RT::Point4( 0.0f,  0.0f, -1.0f, 0.0f), mat[0], epsilon);
        EXPECT_VEC4_NEAR(RT::Point4(-1.0f,  0.0f,  0.0f, 0.0f), mat[1], epsilon);
        EXPECT_VEC4_NEAR(RT::Point4( 0.0f,  1.0f,  0.0f, 0.0f), mat[2], epsilon);
        EXPECT_VEC4_NEAR(RT::Point4(20.0f, -30.0f, 1.0f, 1.0f), mat[3], epsilon);
    }
}