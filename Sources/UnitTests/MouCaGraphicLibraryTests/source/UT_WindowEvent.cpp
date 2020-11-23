#include "Dependancies.h"

#include <LibRT/include/RTRenderDialog.h>

#include "include/MouCaBasicScene.h"

class WindowEventTest : public MouCaBasicScene
{
    public:
        RT::Window::Mode getMode() const override
        {
            return RT::Window::InternalMode; // Avoid change size/rendering in debug/release
        }
};

TEST_F(WindowEventTest, checkEvents)
{
    auto window = _env3D._window.lock();
    ASSERT_TRUE(window.get() != nullptr);

    takeScreenshot(BT::Path(u8"Normal.png"), 30, 30.0);

    ASSERT_NO_THROW(window->setStateSize(RT::Window::Iconified));
    EXPECT_EQ(RT::Window::Iconified, window->getStateSize());

    ASSERT_NO_THROW(window->setStateSize(RT::Window::Normal));
    EXPECT_EQ(RT::Window::Normal, window->getStateSize());
    ASSERT_NO_THROW(window->setVisibility(false));

    takeScreenshot(BT::Path(u8"Normal.png"), 30, 30.0);

    ASSERT_NO_THROW(window->setStateSize(RT::Window::Maximized));
    EXPECT_EQ(RT::Window::Maximized, window->getStateSize());
    ASSERT_NO_THROW(window->setVisibility(false));

    takeScreenshot(BT::Path(L"Maximized.png"), 30, 30.0);

    ASSERT_NO_THROW(window->setStateSize(RT::Window::Normal));
    EXPECT_EQ(RT::Window::Normal, window->getStateSize());
    ASSERT_NO_THROW(window->setVisibility(false));

    takeScreenshot(BT::Path(u8"Normal.png"), 30, 30.0);
}

TEST_F(WindowEventTest, resize)
{
    auto window = _env3D._window.lock();
    ASSERT_TRUE(window.get() != nullptr);

    takeScreenshot(BT::Path(u8"Normal.png"), 30, 30.0);

    ASSERT_NO_THROW(window->setSize(200, 200));
    EXPECT_EQ(RT::Window::Normal, window->getStateSize());

    takeScreenshot(BT::Path(u8"Smaller.png"), 30, 30.0);
}