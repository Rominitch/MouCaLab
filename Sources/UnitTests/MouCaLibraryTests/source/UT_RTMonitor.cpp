#include "Dependencies.h"

#include <LibRT/include/RTMonitor.h>

TEST(RTMonitor, getter)
{
    RT::Monitor monitor;

    const Core::String name("Virtual Monitor");
    const RT::ViewportInt32 viewport(0, 0, 1920, 1200);
    const RT::Array2 physic(200.0f, 200.0f);
    const uint32_t refresh = 60;
    void* handle = reinterpret_cast<void*>(0x1234);
    
    ASSERT_NO_THROW(monitor.initialize(name, viewport, physic, refresh, handle));
    EXPECT_EQ(name, monitor.getName());
    EXPECT_EQ(physic, monitor.getPhysicSize());
    EXPECT_VIEWPORT_EQ(viewport, monitor.getMonitorArea());
    EXPECT_EQ(refresh, monitor.getRefreshRate());
    EXPECT_EQ(handle, monitor.getHandle());
}

TEST(RTMonitor, screenRatio)
{
    void* handle = reinterpret_cast<void*>(0x1234);

    RT::Monitor monitor;
    ASSERT_NO_THROW(monitor.initialize("Virtual Monitor", RT::ViewportInt32(0, 0, 1920, 1200), RT::Array2(200.0f, 200.0f), 60, handle));
    EXPECT_EQ(RT::Monitor::Screen16_10, monitor.getScreenRatio());

    ASSERT_NO_THROW(monitor.initialize("Virtual Monitor", RT::ViewportInt32(0, 0, 1920, 1080), RT::Array2(200.0f, 200.0f), 60, handle));
    EXPECT_EQ(RT::Monitor::Screen16_9, monitor.getScreenRatio());

    ASSERT_NO_THROW(monitor.initialize("Virtual Monitor", RT::ViewportInt32(0, 0, 800, 600), RT::Array2(200.0f, 200.0f), 60, handle));
    EXPECT_EQ(RT::Monitor::Screen4_3, monitor.getScreenRatio());

    ASSERT_NO_THROW(monitor.initialize("Virtual Monitor", RT::ViewportInt32(0, 0, 800, 800), RT::Array2(200.0f, 200.0f), 60, handle));
    EXPECT_EQ(RT::Monitor::ScreenUndefined, monitor.getScreenRatio());
}