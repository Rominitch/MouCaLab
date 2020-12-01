#include "Dependencies.h"

#include <LibRT/include/RTEnum.h>

TEST(RTEnum, computeSizeOf)
{
    EXPECT_EQ(1, RT::computeSizeOf(RT::Type::Char));
    EXPECT_EQ(1, RT::computeSizeOf(RT::Type::UnsignedChar));

    EXPECT_EQ(2, RT::computeSizeOf(RT::Type::Short));
    EXPECT_EQ(2, RT::computeSizeOf(RT::Type::UnsignedShort));

    EXPECT_EQ(4, RT::computeSizeOf(RT::Type::Int));
    EXPECT_EQ(4, RT::computeSizeOf(RT::Type::UnsignedInt));

    EXPECT_EQ(8, RT::computeSizeOf(RT::Type::Int64));
    EXPECT_EQ(8, RT::computeSizeOf(RT::Type::UnsignedInt64));

    EXPECT_EQ(4, RT::computeSizeOf(RT::Type::Float));

    EXPECT_EQ(8, RT::computeSizeOf(RT::Type::Double));

    EXPECT_ANY_THROW(RT::computeSizeOf(RT::Type::NbTypes));
}

TEST(RTEnum, SMouseState)
{
    const RT::Point2 pos1(1.73f, 2.1f);
    const RT::Point2 pos2(14.23f, -2.0f);

    RT::SMouseState mouse;
    mouse._mousePositionSN     = pos1;
    mouse._lastMousePositionSN = pos2;

    ASSERT_NO_THROW(mouse.update());

    EXPECT_EQ(pos1, mouse._mousePositionSN);
    EXPECT_EQ(pos1, mouse._lastMousePositionSN);
}