#include "Dependencies.h"

#include <LibRT/include/RTCanvas.h>

TEST(RTCanvas, construction)
{
    const RT::Array2ui resolution(1920, 1080);
    RT::EventManagerWPtr eventManager;

    RT::Canvas canvas;
    EXPECT_TRUE(canvas.isNull());
    ASSERT_NO_THROW(canvas.initialize(eventManager, resolution));
    EXPECT_FALSE(canvas.isNull());

    EXPECT_EQ(resolution, canvas.getResolution());

    ASSERT_NO_THROW(canvas.release());
    EXPECT_TRUE(canvas.isNull());
}