#include "Dependencies.h"

#include <LibCore/include/CoreElapser.h>

#include <LibRT/include/RTMaths.h>

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


TEST(Bezier3, intersectionX)
{
    std::array<float, 4> x{0.0f, 1.0f, 2.0f, 3.0f};
    std::array<float, 6> i;
    uint32_t nbSolutions;
    
    i = RT::Maths::Bezier3::computeIntersections(x, { 0.5f, 2.0f, 2.0f, 0.5f }, { -1.0f, 4.0f }, { 0.0f, 0.0f }, nbSolutions);
    EXPECT_EQ(0ul, nbSolutions);
    EXPECT_NEAR(0.0f, i[0], 1e-5f);
    EXPECT_NEAR(0.0f, i[1], 1e-5f);
    EXPECT_NEAR(0.0f, i[2], 1e-5f);
    EXPECT_NEAR(0.0f, i[3], 1e-5f);
    EXPECT_NEAR(0.0f, i[4], 1e-5f);
    EXPECT_NEAR(0.0f, i[5], 1e-5f);

    // 1 sol
    i = RT::Maths::Bezier3::computeIntersections(x, {0.5f, -2.0f, -2.0f, -0.5f}, { -1.0f, 4.0f }, {0.0f, 0.0f}, nbSolutions);
    EXPECT_EQ(1ul, nbSolutions);
    EXPECT_NEAR(0.0f,      i[0], 1e-5f);
    EXPECT_NEAR(0.0f,      i[1], 1e-5f);
    EXPECT_NEAR(0.215302f, i[2], 1e-5f);
    EXPECT_NEAR(0.0f,      i[3], 1e-5f);
    EXPECT_NEAR(0.0f,      i[4], 1e-5f);
    EXPECT_NEAR(0.0f,      i[5], 1e-5f);

    // 2 sol
    i = RT::Maths::Bezier3::computeIntersections(x, {0.5f, -2.0f, 2.0f, 0.5f}, { -1.0f, 4.0f }, {0.0f, 0.0f}, nbSolutions);
    EXPECT_EQ(2ul, nbSolutions);
    EXPECT_NEAR(0.0f,      i[0], 1e-5f);
    EXPECT_NEAR(0.0f,      i[1], 1e-5f);
    EXPECT_NEAR(0.252321f, i[2], 1e-5f);
    EXPECT_NEAR(0.0f,      i[3], 1e-5f);
    EXPECT_NEAR(1.371291f, i[4], 1e-5f);
    EXPECT_NEAR(0.0f,      i[5], 1e-5f);

    // 3 sol
    i = RT::Maths::Bezier3::computeIntersections(x, {0.5f, -2.0f, 2.0f, -0.5f}, { -1.0f, 4.0f }, {0.0f, 0.0f}, nbSolutions);
    EXPECT_EQ(3ul, nbSolutions);
    EXPECT_NEAR(2.748075f, i[0], 1e-5f);
    EXPECT_NEAR(0.0f,      i[1], 1e-5f);
    EXPECT_NEAR(0.251924f, i[2], 1e-5f);
    EXPECT_NEAR(0.0f,      i[3], 1e-5f);
    EXPECT_NEAR(1.5f,      i[4], 1e-5f);
    EXPECT_NEAR(0.0f,      i[5], 1e-5f);
}

TEST(Bezier3, intersectionY)
{
    std::array<float, 4> y{0.0f, 1.0f, 2.0f, 3.0f};
    std::array<float, 6> i;
    uint32_t nbSolutions;
    
    i = RT::Maths::Bezier3::computeIntersections({ 0.5f, 2.0f, 2.0f, 0.5f }, y, { 0.0f, 0.0f }, { -1.0f, 4.0f }, nbSolutions);
    EXPECT_EQ(0ul, nbSolutions);
    EXPECT_NEAR(0.0f, i[0], 1e-5f);
    EXPECT_NEAR(0.0f, i[1], 1e-5f);
    EXPECT_NEAR(0.0f, i[2], 1e-5f);
    EXPECT_NEAR(0.0f, i[3], 1e-5f);
    EXPECT_NEAR(0.0f, i[4], 1e-5f);
    EXPECT_NEAR(0.0f, i[5], 1e-5f);

    // 1 sol
    i = RT::Maths::Bezier3::computeIntersections({0.5f, -2.0f, -2.0f, -0.5f}, y, { 0.0f, 0.0f }, { -1.0f, 4.0f }, nbSolutions);
    EXPECT_EQ(1ul, nbSolutions);
    EXPECT_NEAR(0.0f,      i[0], 1e-5f);
    EXPECT_NEAR(0.0f,      i[1], 1e-5f);
    EXPECT_NEAR(0.0f,      i[2], 1e-5f);
    EXPECT_NEAR(0.215302f, i[3], 1e-5f);
    EXPECT_NEAR(0.0f,      i[4], 1e-5f);
    EXPECT_NEAR(0.0f,      i[5], 1e-5f);

    // 2 sol
    i = RT::Maths::Bezier3::computeIntersections({0.5f, -2.0f, 2.0f, 0.5f}, y, { 0.0f, 0.0f }, { -1.0f, 4.0f }, nbSolutions);
    EXPECT_EQ(2ul, nbSolutions);
    EXPECT_NEAR(0.0f,      i[0], 1e-5f);
    EXPECT_NEAR(0.0f,      i[1], 1e-5f);
    EXPECT_NEAR(0.0f,      i[2], 1e-5f);
    EXPECT_NEAR(0.252321f, i[3], 1e-5f);
    EXPECT_NEAR(0.0f,      i[4], 1e-5f);
    EXPECT_NEAR(1.371291f, i[5], 1e-5f);

    // 3 sol
    i = RT::Maths::Bezier3::computeIntersections({0.5f, -2.0f, 2.0f, -0.5f}, y, { 0.0f, 0.0f }, { -1.0f, 4.0f }, nbSolutions);
    EXPECT_EQ(3ul, nbSolutions);
    EXPECT_NEAR(0.0f,      i[0], 1e-5f);
    EXPECT_NEAR(2.748075f, i[1], 1e-5f);
    EXPECT_NEAR(0.0f,      i[2], 1e-5f);
    EXPECT_NEAR(0.251924f, i[3], 1e-5f);
    EXPECT_NEAR(0.0f,      i[4], 1e-5f);
    EXPECT_NEAR(1.5f,      i[5], 1e-5f);
}

TEST(Bezier3, PERFORMANCE_1000)
{
    const uint32_t nbLoop  = 1000;
    const uint32_t nbCases = 16;

    const std::array<float, 4> y{0.0f, 1.0f, 2.0f, 3.0f};
    std::array<int64_t, 4> timeCumulate{ 0,0,0,0 };
    std::array<int64_t, 4> count{ 0,0,0,0 };

    for( uint32_t loop = 0; loop < nbLoop; ++loop)
    {
        for (uint32_t cases=0; cases < nbCases; ++cases)
        {
            std::array<float, 4> x = { -0.5f, -2.0f, -2.0f, -0.5f };
            x[0] *= cases & 0x1 ? 1.0f : -1.0f;
            x[1] *= cases & 0x2 ? 1.0f : -1.0f;
            x[2] *= cases & 0x4 ? 1.0f : -1.0f;
            x[3] *= cases & 0x8 ? 1.0f : -1.0f;
            
            uint32_t sol;        
            Core::Elapser t;
            RT::Maths::Bezier3::computeIntersections(x, y, { 0.0f, 0.0f }, { -1.0f, 4.0f }, sol);
            timeCumulate[sol] += t.tick<std::chrono::microseconds>();
            ++count[sol];
        }
    }

    const int64_t total = std::accumulate(timeCumulate.begin(), timeCumulate.end(), 0ull);
    std::cout << "Performance mean: " << static_cast<double>(total) / static_cast<double>(nbLoop * nbCases) << " \xE6s" << std::endl;
    for(uint32_t sol = 0; sol < timeCumulate.size(); ++sol)
    {
        const double time = static_cast<double>(timeCumulate[sol]) / static_cast<double>(count[sol]);
        std::cout << "   "<<sol<<" Solution: "<< count[sol] << " cases, mean time:" << time << " \xE6s" << std::endl;
    }    
}

}