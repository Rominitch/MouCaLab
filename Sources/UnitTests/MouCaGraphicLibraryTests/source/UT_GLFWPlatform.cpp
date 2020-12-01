#include "Dependencies.h"

#include <LibGLFW/include/GLFWPlatform.h>

using namespace std::chrono_literals;

namespace GLFW
{
    
TEST(GLFWPlatform, createWindow)
{
    Platform _platform;
    ASSERT_NO_THROW(_platform.initialize());

    EXPECT_FALSE(_platform.isWindowsActive());
    EXPECT_EQ(0, _platform.getNumberWindows());

    WindowWPtr window;
    const RT::ViewportInt32 viewport(0, 0, 50, 10);
    ASSERT_NO_THROW(window = _platform.createWindow(viewport, "Test - GLFWPlatformTest.createWindow", RT::Window::Windowed));

    EXPECT_TRUE(_platform.isWindowsActive());
    EXPECT_EQ(1, _platform.getNumberWindows());

    {
        WindowWPtr select;
        ASSERT_NO_THROW(select = _platform.getWindow(0));
        EXPECT_EQ(select.lock(), window.lock());
    }

    ASSERT_NO_THROW(_platform.releaseWindow(window));
    EXPECT_EQ(0, _platform.getNumberWindows());

    ASSERT_NO_THROW(_platform.release());
}

// cppcheck-suppress syntaxError
TEST(GLFWPlatform, runtime)
{
    Platform _platform;
    ASSERT_NO_THROW(_platform.initialize());

    WindowWPtr window;
    const RT::ViewportInt32 viewport(0, 0, 50, 10);
    ASSERT_NO_THROW(window = _platform.createWindow(viewport, "Test - GLFWPlatformTest.runtime", RT::Window::Windowed));

    // Demand to remove window
    const auto closeWindow = [&]()
    {
        // BE sure we wait runMainLoop() execution.
        while(!_platform.isRunLoopActive())
        {
            std::this_thread::sleep_for(5ms);
        }
        ASSERT_NO_THROW(_platform.demandExitMainLoop());
    };
    std::thread closerWindow(closeWindow);

    // Launch thread + Lock until finished
    ASSERT_NO_THROW(_platform.runMainLoop());

    // Wait special close of runMainLoop()
    closerWindow.join();

    ASSERT_NO_THROW(_platform.releaseWindow(window));
    EXPECT_EQ(0, _platform.getNumberWindows());
    
    ASSERT_NO_THROW(_platform.release());
}

}