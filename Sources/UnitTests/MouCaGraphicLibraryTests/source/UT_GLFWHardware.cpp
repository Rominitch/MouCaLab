#include "Dependencies.h"

#include <LibRT/include/RTMonitor.h>
#include <LibGLFW/include/GLFWHardware.h>
#include <LibGLFW/include/GLFWPlatform.h>

TEST(GLFWHardware, readHardware)
{
    GLFW::Platform platform;
    platform.initialize();

    GLFW::Hardware hardware;

    ASSERT_NO_THROW(hardware.readHardware());

    const std::vector<RT::Monitor>& monitors = hardware.getMonitors();

    EXPECT_LE(1, monitors.size());
    for(const auto& monitor : monitors)
    {
        EXPECT_FALSE(monitor.getName().empty());

        // Screen must be upper than 200 mm
        EXPECT_LE(200.0f, monitor.getPhysicSize().x);
        EXPECT_LE(200.0f, monitor.getPhysicSize().y);

        // Screen must be upper than 1920x1200
        EXPECT_LE(1920ul, monitor.getMonitorArea().getWidth());
        EXPECT_LE(1080ul, monitor.getMonitorArea().getHeight());

        EXPECT_TRUE(monitor.getHandle() != nullptr);

        EXPECT_NE(RT::Monitor::ScreenUndefined, monitor.getScreenRatio());
    }

    platform.release();
}