#include "Dependancies.h"

#include <LibRT/include/RTViewport.h>

TEST(RTViewport, constructorDefault)
{
    RT::ViewportInt32 viewport;

    EXPECT_TRUE(viewport.isNull());

    EXPECT_EQ(0, viewport.getX());
    EXPECT_EQ(0, viewport.getY());
    EXPECT_EQ(0ul, viewport.getWidth());
    EXPECT_EQ(0ul, viewport.getHeight());

    EXPECT_EQ(RT::Array2i(0, 0),  viewport.getOffset());
    EXPECT_EQ(RT::Array2ui(0, 0), viewport.getSize());
}

TEST(RTViewport, constructor)
{
    const int32_t x =  10;
    const int32_t y = -20;
    const uint32_t w = 1920;
    const uint32_t h = 1200;
    RT::ViewportInt32 viewport(x, y, w, h);

    EXPECT_FALSE(viewport.isNull());
    
    EXPECT_EQ(x, viewport.getX());
    EXPECT_EQ(y, viewport.getY());
    EXPECT_EQ(w, viewport.getWidth());
    EXPECT_EQ(h, viewport.getHeight());

    EXPECT_EQ(RT::Array2i(x, y),  viewport.getOffset());
    EXPECT_EQ(RT::Array2ui(w, h), viewport.getSize());

    ASSERT_NO_THROW(viewport.setSize(0, 0));
    EXPECT_TRUE(viewport.isNull());
}