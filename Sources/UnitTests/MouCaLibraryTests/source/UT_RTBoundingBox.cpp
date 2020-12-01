#include "Dependencies.h"

#include <LibRT/include/RTBoundingBox.h>

namespace RT
{

TEST( RTBoundingBox, create )
{
    BoundingBox bbox;
    EXPECT_FALSE( bbox.isValid() );
    const RT::Point3 pts( 1.0f, 2.0f, 3.0f );

    // Expand
    {
        const std::vector<RT::Point3> points =
        {
            pts
        };

        ASSERT_NO_THROW( bbox.expand( points ) );
        EXPECT_TRUE( bbox.isValid() );

        EXPECT_EQ( RT::Array3( 0.0f, 0.0f, 0.0f ), bbox.getSize() );
        EXPECT_EQ( pts,                            bbox.getCenter() );
        EXPECT_EQ( 0.0f,                           bbox.getRadius() );
    }

    // Continue to expand
    {
        const std::vector<RT::Point3> points =
        {
            -pts
        };
        ASSERT_NO_THROW( bbox.expand( points ) );
        EXPECT_TRUE( bbox.isValid() );
        EXPECT_EQ( 2.0f * pts,                     bbox.getSize() );
        EXPECT_EQ( RT::Array3( 0.0f, 0.0f, 0.0f ), bbox.getCenter() );
        EXPECT_EQ( 3.74165739f,                    bbox.getRadius() ); // sqrt(dot(pts))
    }

    // Continue to expand with another API
    {
        const RT::Point3 limit( 3.0f, 3.0f, 3.0f );

        std::array<RT::Point3, 4> points =
        {
            limit,
            -limit,
            -limit * 0.5f,
            limit * 0.5f
        };

        ASSERT_NO_THROW( bbox.expand( points.size(), points.data() ) );
        EXPECT_TRUE( bbox.isValid() );
        EXPECT_EQ( 2.0f * limit, bbox.getSize() );
        EXPECT_EQ( RT::Array3( 0.0f, 0.0f, 0.0f ), bbox.getCenter() );
        EXPECT_EQ( 5.19615242f, bbox.getRadius() ); // sqrt(dot(limit))
    }
}

}