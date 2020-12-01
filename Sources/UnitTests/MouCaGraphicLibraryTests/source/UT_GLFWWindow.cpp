#include "Dependencies.h"

#include "include/EventManagerEmpty.h"

#include <LibGLFW/include/GLFWHardware.h>
#include <LibGLFW/include/GLFWPlatform.h>
#include <LibGLFW/include/GLFWWindow.h>

namespace GLFW
{

class GLFWWindowTest : public ::testing::Test
{
    public:
        static GLFW::Platform _platform;

        static void SetUpTestSuite()
        {
            ASSERT_NO_THROW(_platform.initialize());
        }

        static void TearDownTestSuite()
        {
            ASSERT_NO_THROW(_platform.release());
        }
};
GLFW::Platform GLFWWindowTest::_platform;

// cppcheck-suppress syntaxError
TEST_F(GLFWWindowTest, events)
{
    EXPECT_EQ(0, _platform.getNumberWindows());

    // Create window
    WindowWPtr window;
    const RT::ViewportInt32 viewport(0, 0, 50, 10);
    ASSERT_NO_THROW(window = _platform.createWindow(viewport, "Test - GLFWWindowTest.createWindow", RT::Window::InternalMode));
    EXPECT_EQ(1, _platform.getNumberWindows());

    // Call state method
    {
        WindowSPtr windowLock = window.lock();

        EXPECT_FALSE(windowLock->needClose());

        //EXPECT_EQ(RT::NoButton, windowLock->getMouseState());
    }

    // Call static event without EventManager
    {
        WindowSPtr windowLock = window.lock();

        ASSERT_NO_THROW(windowLock->onIconify(windowLock->_window, 0));
        ASSERT_NO_THROW(windowLock->onKeyEvents(windowLock->_window, 0, 0, 0, 0));
        ASSERT_NO_THROW(windowLock->onMouseButtonEvents(windowLock->_window, 0, 0, 0));
        ASSERT_NO_THROW(windowLock->onMouseMove(windowLock->_window, 0.0, 0.0));
        ASSERT_NO_THROW(windowLock->onMouseWheelEvents(windowLock->_window, 0.0, 0.0));
    }

    // Call static event with EventManager : TODO -> must be call programmatically but currently we check real state of window.
    {
        auto eventManager = std::make_shared<EventManagerEmpty>();
        
        WindowSPtr windowLock = window.lock();

        windowLock->initialize(eventManager, RT::Array2ui(viewport.getWidth(), viewport.getHeight()));
        
        ASSERT_NO_THROW(windowLock->onIconify(windowLock->_window, 0));
        ASSERT_NO_THROW(windowLock->onKeyEvents(windowLock->_window, 0, 0, 0, 0));
        ASSERT_NO_THROW(windowLock->onMouseButtonEvents(windowLock->_window, 0, 0, 0));
        ASSERT_NO_THROW(windowLock->onMouseMove(windowLock->_window, 10.0, 20.0));
        ASSERT_NO_THROW(windowLock->onMouseWheelEvents(windowLock->_window, 0.1, 0.2));

        ASSERT_NO_THROW(windowLock->release());
    }

    // Release
    ASSERT_NO_THROW(_platform.releaseWindow(window));
}

TEST_F(GLFWWindowTest, methods)
{
    EXPECT_EQ(0, _platform.getNumberWindows());

    // Create window
    WindowWPtr window;
    const RT::ViewportInt32 viewport(50, 50, 400, 400);
    ASSERT_NO_THROW(window = _platform.createWindow(viewport, "Test - GLFWWindowTest.createWindow", RT::Window::InternalMode));

    {
        WindowSPtr windowLock = window.lock();
        
        EXPECT_EQ(GLFW_TRUE, windowLock->isMode(RT::Window::Resizable, RT::Window::Resizable));
        EXPECT_EQ(GLFW_FALSE, windowLock->isMode(RT::Window::Resizable, RT::Window::Windowed));
    }

    // Release
    ASSERT_NO_THROW(_platform.releaseWindow(window));
}

TEST_F(GLFWWindowTest, createWindow)
{
    EXPECT_EQ(0, _platform.getNumberWindows());

    GLFW::Hardware hardware;
    ASSERT_NO_THROW(hardware.readHardware());
    ASSERT_FALSE(hardware.getMonitors().empty());

    // Create window
    WindowWPtr window;
    const RT::ViewportInt32 viewport(50, 50, 400, 400);
    {
        ASSERT_NO_THROW(window = _platform.createWindow(viewport, "Test - GLFWWindowTest.createWindow", RT::Window::InternalMode));
        
        ASSERT_NE(nullptr, window.lock()->getHandle());

        ASSERT_NO_THROW(_platform.releaseWindow(window));
    }

    {
        ASSERT_NO_THROW(window = _platform.createWindow(viewport, "Test - GLFWWindowTest.createWindow", RT::Window::StaticDialogMode));
        ASSERT_NO_THROW(_platform.releaseWindow(window));
    }

    {
        ASSERT_NO_THROW(window = _platform.createWindow(*hardware.getMonitors().crbegin(), "Test - GLFWWindowTest.createWindow", RT::Window::FullscreenWindowed));
        ASSERT_NO_THROW(_platform.releaseWindow(window));
    }
    {
        ASSERT_NO_THROW(window = _platform.createWindow(*hardware.getMonitors().crbegin(), "Test - GLFWWindowTest.createWindow", RT::Window::FullscreenReal));
        ASSERT_NO_THROW(_platform.releaseWindow(window));
    }
}

TEST_F(GLFWWindowTest, sorting)
{
    EXPECT_EQ(0, _platform.getNumberWindows());

    const RT::ViewportInt32 viewport(50, 50, 400, 400);
    auto dialogA = _platform.createWindow(viewport, "Test - A", RT::Window::InternalMode);
    auto dialogB = _platform.createWindow(viewport, "Test - B", RT::Window::InternalMode);
    auto dialogC = _platform.createWindow(viewport, "Test - C", RT::Window::InternalMode);

    std::map<RT::RenderDialogWPtr, int> mapping;
    mapping[dialogA] = 0;
    mapping[dialogB] = 1;
    mapping[dialogC] = 2;

    EXPECT_EQ(1, mapping.find(dialogB)->second);

    ASSERT_NO_THROW(_platform.releaseWindow(dialogC));
    ASSERT_NO_THROW(_platform.releaseWindow(dialogB));
    ASSERT_NO_THROW(_platform.releaseWindow(dialogA));
}

class TestObserver final
{
public:
    bool _close;

    TestObserver() :
        _close(false)
    {}

    ~TestObserver() = default;

    void notifyClose(RT::Window*)
    {
        _close = true;
    }
};

TEST_F(GLFWWindowTest, observer)
{
    const RT::ViewportInt32 viewport(50, 50, 400, 400);

    auto observer = std::make_shared<TestObserver>();
    // TEST 1: connect and disconnect without emit
    {
        auto window = _platform.createWindow(viewport, "Test - A", RT::Window::InternalMode);
        {
            auto lockWindow = window.lock();
            ASSERT_NO_THROW(lockWindow->signalClose().connectMember(observer.get(), &TestObserver::notifyClose));

            ASSERT_NO_THROW(lockWindow->signalClose().disconnectAll());
        }
        ASSERT_NO_THROW(_platform.releaseWindow(window));
    }
    EXPECT_FALSE(observer->_close);

    // TEST 2: connect and emit
    {
        auto window = _platform.createWindow(viewport, "Test - B", RT::Window::InternalMode);
        {
            auto lockWindow = window.lock();
            ASSERT_NO_THROW(lockWindow->signalClose().connectMember(observer.get(), &TestObserver::notifyClose));

            ASSERT_NO_THROW(lockWindow->signalClose().emit(lockWindow.get()));
        }
        ASSERT_NO_THROW(_platform.releaseWindow(window));
    }
    EXPECT_TRUE(observer->_close);
}

}