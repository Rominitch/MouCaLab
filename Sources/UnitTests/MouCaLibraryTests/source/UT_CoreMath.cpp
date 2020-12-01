#include "Dependencies.h"

#include <LibCore/include/CoreMaths.h>

TEST(CoreMaths, PI)
{
    EXPECT_EQ(3.14159265358979323846f, Core::Maths::PI<float>);
    EXPECT_EQ(3.14159265358979323846,  Core::Maths::PI<double>);
}

TEST(CoreMaths, toRad)
{
    EXPECT_EQ(Core::Maths::PI<float>,  Core::Maths::toRad<float>(180.0f));
    EXPECT_EQ(Core::Maths::PI<double>, Core::Maths::toRad<double>(180.0));
}

TEST(CoreMaths, toDeg)
{
    EXPECT_EQ(180.0f, Core::Maths::toDeg<float>(Core::Maths::PI<float>));
    EXPECT_EQ(180.0,  Core::Maths::toDeg<double>(Core::Maths::PI<double>));
}

TEST(CoreMaths, clamp) // From Core::Maths to std (c++17)
{
    EXPECT_EQ(  0.0f, std::clamp<float>( 0.0f,  -50.0f, 50.0f));
    EXPECT_EQ(-50.0f, std::clamp<float>(-60.0f, -50.0f, 50.0f));
    EXPECT_EQ( 50.0f, std::clamp<float>( 70.0f, -50.0f, 50.0f));

    EXPECT_EQ(  0.0, std::clamp<double>(  0.0, -50.0, 50.0));
    EXPECT_EQ(-50.0, std::clamp<double>(-60.0, -50.0, 50.0));
    EXPECT_EQ( 50.0, std::clamp<double>( 70.0, -50.0, 50.0));

    EXPECT_EQ(  0, std::clamp<int32_t>(  0, -50, 50));
    EXPECT_EQ(-50, std::clamp<int32_t>(-60, -50, 50));
    EXPECT_EQ( 50, std::clamp<int32_t>( 70, -50, 50));
}

TEST(CoreMaths, angleCyclic)
{
    const float epsilon = 1e5f;
    const float angle = Core::Maths::PI<float>;
    // In range
    EXPECT_EQ(angle, Core::Maths::angleCyclic<float>(angle));

    // Over range
    EXPECT_NEAR(angle, Core::Maths::angleCyclic<float>(3.0f * angle),  epsilon);
    EXPECT_NEAR(angle, Core::Maths::angleCyclic<float>(-3.0f * angle), epsilon);
    EXPECT_NEAR(angle, Core::Maths::angleCyclic<float>(-angle),        epsilon);

    //Limit
    const float PIPI = 2.0f * Core::Maths::PI<float>;
    const float limit = 1e5f;
    EXPECT_NEAR(0.0f,         Core::Maths::angleCyclic<float>(0.0f),         epsilon);
    EXPECT_NEAR(PIPI - limit, Core::Maths::angleCyclic<float>(-limit),       epsilon);
    EXPECT_NEAR(limit,        Core::Maths::angleCyclic<float>(limit + PIPI), epsilon);
}

TEST(CoreMaths, modPositive)
{
    EXPECT_EQ(0, Core::Maths::modPositive(0, 15));
    EXPECT_EQ(3, Core::Maths::modPositive(3, 4));

    EXPECT_EQ(2, Core::Maths::modPositive(6, 4));
    EXPECT_EQ(2, Core::Maths::modPositive(-6, 4));
}

TEST(CoreMaths, isPowerOfTwo)
{
    EXPECT_TRUE(Core::Maths::isPowerOfTwo(1));
    EXPECT_TRUE(Core::Maths::isPowerOfTwo(2));
    EXPECT_TRUE(Core::Maths::isPowerOfTwo(64));

    EXPECT_FALSE(Core::Maths::isPowerOfTwo(0));
    EXPECT_FALSE(Core::Maths::isPowerOfTwo(3));
    EXPECT_FALSE(Core::Maths::isPowerOfTwo(341));

    EXPECT_FALSE(Core::Maths::isPowerOfTwo(-1));
    EXPECT_FALSE(Core::Maths::isPowerOfTwo(-64));
    EXPECT_FALSE(Core::Maths::isPowerOfTwo(-341));
}
