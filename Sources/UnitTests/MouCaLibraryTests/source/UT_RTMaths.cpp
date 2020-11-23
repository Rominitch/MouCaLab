#include "Dependancies.h"

#include "LibRT/include/RTMaths.h"

namespace RT
{
static const float epsilon = 1e-6f;

TEST(RTMaths, lerp)
{
    const glm::vec2 a(1.0, 5.3);
    const glm::vec2 b(7.0, -1.1);

    EXPECT_VEC2_NEAR(a, RT::Maths::lerp(a, b, 0.0f), epsilon);
    EXPECT_VEC2_NEAR(b, RT::Maths::lerp(a, b, 1.0f), epsilon);

    EXPECT_VEC2_NEAR((a+b)*0.5f, RT::Maths::lerp(a, b, 0.5f), epsilon);
}

TEST(RTMaths, signedDistanceToLine)
{
    const glm::vec2 a(1.0, 5.3);
    const glm::vec2 b(7.0, -1.1);

    glm::vec2 p0(RT::Maths::lerp(a, b, 0.713f));

    // On line
    EXPECT_NEAR(0.0f, RT::Maths::signedDistanceToLine(a, b, a),  epsilon);
    EXPECT_NEAR(0.0f, RT::Maths::signedDistanceToLine(a, b, b),  epsilon);
    EXPECT_NEAR(0.0f, RT::Maths::signedDistanceToLine(a, b, p0), epsilon);

    const glm::vec2 dir(b - a);
    const glm::vec2 normal = glm::normalize(glm::vec2(-dir.y, dir.x));
    const float val0 = 20.1f;
    EXPECT_NEAR(val0, RT::Maths::signedDistanceToLine(a, b, a - normal * val0), epsilon);
    const float val1 = -4.121f;
    EXPECT_NEAR(val1, RT::Maths::signedDistanceToLine(a, b, p0 - normal * val1), epsilon);
}

TEST(RTMaths, positionOnLine)
{
    const glm::vec2 a(1.0, 5.3);
    const glm::vec2 b(7.0, -1.1);

    // On line
    EXPECT_NEAR(0.0f, RT::Maths::positionOnLine(a, b, a), epsilon);
    EXPECT_NEAR(1.0f, RT::Maths::positionOnLine(a, b, b), epsilon);
    const float val0 = 0.713f;
    const glm::vec2 p0(RT::Maths::lerp(a, b, val0));
    EXPECT_NEAR(val0, RT::Maths::positionOnLine(a, b, p0), epsilon);

    // Normal case
    const glm::vec2 dir(b - a);
    const glm::vec2 normal = glm::normalize(glm::vec2(-dir.y, dir.x));
    EXPECT_NEAR(0.0f, RT::Maths::positionOnLine(a, b, a + normal * 20.1f), epsilon);
    EXPECT_NEAR(val0, RT::Maths::positionOnLine(a, b, p0 + normal * -4.121f), epsilon);

    // Limit
    EXPECT_NEAR(0.0f, RT::Maths::positionOnLine(a, b, a + dir * -150.0f), epsilon);
    EXPECT_NEAR(1.0f, RT::Maths::positionOnLine(a, b, a + dir * 150.0f), epsilon);
}

TEST(RTMaths, upperPowOfTwo)
{
    EXPECT_EQ(4u,       RT::Maths::upperPowOfTwo<uint32_t>(3));
    EXPECT_EQ(4u,       RT::Maths::upperPowOfTwo<uint32_t>(4));
    EXPECT_EQ(8u,       RT::Maths::upperPowOfTwo<uint32_t>(5));

    EXPECT_EQ(128u,     RT::Maths::upperPowOfTwo<uint32_t>(70));

    EXPECT_EQ(65536u,   RT::Maths::upperPowOfTwo<uint32_t>(45123));

    EXPECT_EQ(65536ull, RT::Maths::upperPowOfTwo<uint64_t>(45123));
}

}