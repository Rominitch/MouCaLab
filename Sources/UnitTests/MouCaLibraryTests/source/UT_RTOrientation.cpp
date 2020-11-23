#include "Dependancies.h"

#include <LibRT/include/RTOrientation.h>

namespace RT
{

TEST(RTOrientation, identity)
{
    Orientation identity;
    EXPECT_EQ( Matrix4( 1.0f ), identity.getWorldToLocal().convert() );
    EXPECT_EQ( Matrix4( 1.0f ), identity.getLocalToWorld().convert() );

    identity.print();
}

// cppcheck-suppress syntaxError
/*
TEST( RTOrientation, matrix )
{
    const float cpi_4 = cos(Core::Maths::PI<float> / 4.0f);
    const Point4 xVector(cpi_4, -cpi_4, 0.0f, 0.0f);
    const Point4 yVector(cpi_4, cpi_4, 0.0f, 0.0f);
    const Point4 zVector(0.0f, 0.0f, 1.0f, 0.0f);
    const Point4 pos(-4.0f, 3.0f, 5.0f, 1.0f);

    Matrix4 mat = glm::rotate(Matrix4(1.0f), Core::Maths::PI<float> / 4, Point3(0.0f, 0.0f, 1.0f));
    mat = glm::translate(mat, -Point3(pos));
    Matrix4 imat = glm::inverse(mat);

    Orientation orientation;
    ASSERT_NO_THROW(orientation.setMatrixW2L(mat));

    EXPECT_EQ(mat,  orientation.getWorldToLocal());
    EXPECT_EQ(imat, orientation.getLocalToWorld());

    EXPECT_VEC4_NEAR(xVector, orientation.getLocalToWorld()[0], 1e-4f);
    EXPECT_VEC4_NEAR(yVector, orientation.getLocalToWorld()[1], 1e-4f);
    EXPECT_VEC4_NEAR(zVector, orientation.getLocalToWorld()[2], 1e-4f);
    EXPECT_VEC4_NEAR(pos,     orientation.getLocalToWorld()[3], 1e-4f);

    orientation.print();
}
*/

TEST(RTOrientation, transform)
{
    const float cpi_4 = cos(Core::Maths::PI<float> / 4.0f);
    const Point4 xVector(cpi_4, -cpi_4, 0.0f, 0.0f);
    const Point4 yVector(cpi_4, cpi_4, 0.0f, 0.0f);
    const Point4 zVector(0.0f, 0.0f, 1.0f, 0.0f);
    const Point3 pos(-4.0f, 3.0f, 5.0f);

    Matrix4 mat = glm::rotate(Matrix4(1.0f), Core::Maths::PI<float> / 4, Point3(0.0f, 0.0f, 1.0f));
    Transform t;
    t._rotation = glm::quat_cast(mat);
    t._position = pos;
    
    Transform it = Transform::inverse(t);

    Orientation orientation;
    ASSERT_NO_THROW(orientation.setWorldToLocal(t));

    EXPECT_EQ(t,  orientation.getWorldToLocal());
    EXPECT_EQ(it, orientation.getLocalToWorld());

    orientation.print();
}

}