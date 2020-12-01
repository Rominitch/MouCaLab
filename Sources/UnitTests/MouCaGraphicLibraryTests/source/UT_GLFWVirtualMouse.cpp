#include "Dependencies.h"

#include "include/EventManagerEmpty.h"

#include <LibRT/include/RTImage.h>

#include <LibGLFW/include/GLFWPlatform.h>
#include <LibGLFW/include/GLFWVirtualMouse.h>
#include <LibGLFW/include/GLFWWindow.h>

/*
#include <MouCaCore/interface/ICoreSystem.h>
#include <MouCaCore/interface/ILoaderManager.h>
#include <MouCaCore/interface/IResourceManager.h>
*/

namespace GLFW
{

class GLFWVirtualMouseTest : public ::testing::Test
{
    public:
        //MouCaCore::ICoreSystemUPtr _core;
        GLFW::Platform _platform;

        void SetUp() override
        {
            ASSERT_NO_THROW(_platform.initialize());

//             ASSERT_NO_THROW(_core = MouCaCore::ICoreSystem::launchCore());
//             ASSERT_NO_THROW(_core->getLoaderManager().initialize());
        }

        void TearDown() override
        {
//             ASSERT_NO_THROW(_core->getLoaderManager().release());
//             ASSERT_NO_THROW(_core->getResourceManager().releaseResources());
//             ASSERT_NO_THROW(_core.reset());

            ASSERT_NO_THROW( _platform.release() );
        }
};

// cppcheck-suppress syntaxError
TEST_F( GLFWVirtualMouseTest, setMode )
{
    // Create window
    WindowWPtr window;
    const RT::ViewportInt32 viewport( 0, 0, 50, 10 );
    ASSERT_NO_THROW( window = _platform.createWindow( viewport, "Test - GLFWWindowTest.createWindow", RT::Window::InternalMode ) );
    EXPECT_EQ(1, _platform.getNumberWindows());

    // Check default
    EXPECT_EQ(RT::VirtualMouse::Mode::OSMouse, _platform.getMouse().getMode());

    // Set new value
    _platform.getEditMouse().setMode(RT::VirtualMouse::Mode::Invisible);
    EXPECT_EQ(RT::VirtualMouse::Mode::Invisible, _platform.getMouse().getMode());

    _platform.getEditMouse().setMode(RT::VirtualMouse::Mode::Locked);
    EXPECT_EQ(RT::VirtualMouse::Mode::Locked, _platform.getMouse().getMode());

    _platform.getEditMouse().setMode(RT::VirtualMouse::Mode::OSMouse);
    EXPECT_EQ(RT::VirtualMouse::Mode::OSMouse, _platform.getMouse().getMode());

    _platform.releaseWindow( window );
}

TEST_F(GLFWVirtualMouseTest, setPosition)
{
    const RT::Point2i screen(10, 20);
    const RT::Point2  screenN(1.0f, 0.5f);
    const RT::Point3  world(30.0f, 40.0f, 50.2f);
    EXPECT_NO_THROW(_platform.getEditMouse().setPosition(screen, screenN, world));

    EXPECT_EQ(screen,  _platform.getMouse().getScreenPxPosition());
    EXPECT_EQ(screenN, _platform.getMouse().getNormScreenPosition());
    EXPECT_EQ(world,   _platform.getMouse().getWorldPosition());
}

TEST_F(GLFWVirtualMouseTest, setPressedButtons)
{
    //Check default
    EXPECT_EQ(RT::VirtualMouse::Buttons::None, _platform.getMouse().getPressedButtons());

    RT::VirtualMouse::Buttons buttons = static_cast<RT::VirtualMouse::Buttons>(static_cast<uint16_t>(RT::VirtualMouse::Buttons::Left) | static_cast<uint16_t>(RT::VirtualMouse::Buttons::Right));
    EXPECT_NO_THROW(_platform.getEditMouse().setPressedButtons(buttons));
    EXPECT_EQ(buttons, _platform.getMouse().getPressedButtons());
}
/*
/// Test: 
///   - Create pointer image and apply it on current window.
///   - Impossible to verify rendering (native screenshot don't take mouse)
TEST_F( GLFWVirtualMouseTest, cursor )
{
    size_t id = 0;
    // Create cursor
    {
        auto& resources = _core->getResourceManager();
        auto& loader    = _core->getLoaderManager();
    
        auto mouseCursor = resources.openImage(MouCaEnvironment::getInputPath() / L"textures" / L"NO_RIGHT_mouse.png");
        MouCaCore::LoadingItems items;
        items.emplace_back(MouCaCore::LoadingItem( mouseCursor,  MouCaCore::LoadingItem::Direct ));
        
        // Demand loading
        loader.loadResources( items );

        id = _platform.getEditMouse().createNewCursor(*mouseCursor->getImage().lock(), RT::Point2i(0, 0));
        resources.releaseResource( mouseCursor );
    }

    // Create window
    WindowWPtr window;
    const RT::ViewportInt32 viewport( 0, 0, 300, 300 );
    ASSERT_NO_THROW( window = _platform.createWindow( viewport, "Test - GLFWVirtualMouseTest.Mouse", RT::Window::InternalMode ) );
    EXPECT_EQ( 1, _platform.getNumberWindows() );

    auto eventManager = std::make_shared<EventManagerEmpty>();
    {
        auto windowLock = window.lock();
        windowLock->initialize( eventManager, RT::Array2ui( viewport.getWidth(), viewport.getHeight() ) );

        //Apply cursor
        ASSERT_NO_THROW(_platform.getEditMouse().applyCursor(*windowLock.get(), id));
    }

    ASSERT_NO_THROW(_platform.getEditMouse().deleteCursor(id));

    if( !window.expired() )
        _platform.releaseWindow( window );
}
*/

// DisableCodeCoverage

class EventManagerTEST : public RT::EventManager
{
    public:
    EventManagerTEST(GLFW::Platform* platform) :
        _platform(platform)
    {}

    void onClose(RT::Canvas*) override
    {}

    void onSize(RT::Canvas*, const RT::Array2ui&) override
    {}

    void onMouseMove(RT::Canvas*, const RT::VirtualMouse& mouse) override
    {
        print(mouse);
    }

    void onMousePress(RT::Canvas*, const RT::VirtualMouse& mouse, const RT::VirtualMouse::Buttons) override
    {
        print(mouse);
    }

    void onMouseRelease(RT::Canvas*, const RT::VirtualMouse& mouse, const RT::VirtualMouse::Buttons) override
    {
        print(mouse);
    }

    void onMouseWheel(RT::Canvas*, const RT::VirtualMouse&, const int) override
    {}

    void onKeyPress(RT::Canvas*, int key, int, int, int) override
    {
        switch(key)
        {
        case 321:
        case '1':
            _platform->getEditMouse().setMode(RT::VirtualMouse::Mode::OSMouse);
            break;

        case 322:
        case '2':
            _platform->getEditMouse().setMode(RT::VirtualMouse::Mode::Invisible);
            break;

        case 323:
        case '3':
            _platform->getEditMouse().setMode(RT::VirtualMouse::Mode::Locked);
            break;

        default:
        {}
        }
    }

    void print(const RT::VirtualMouse& mouse)
    {
        std::cout << "Mouse: "<< std::fixed << std::setprecision(4) << std::setw(6) << std::setfill('0') << mouse.getNormScreenPosition().x << ";" << mouse.getNormScreenPosition().y
            << "   ["
            << (mouse.isPressed(RT::VirtualMouse::Buttons::Left)   ? "L" :  " ")
            << (mouse.isPressed(RT::VirtualMouse::Buttons::Right)  ? "R" :  " ")
            << (mouse.isPressed(RT::VirtualMouse::Buttons::Middle) ? "M" :  " ")
            << (mouse.isPressed(RT::VirtualMouse::Buttons::Mouse4) ? "4" :  " ")
            << (mouse.isPressed(RT::VirtualMouse::Buttons::Mouse5) ? "5" :  " ")
            << (mouse.isPressed(RT::VirtualMouse::Buttons::Mouse6) ? "6" :  " ")
            << (mouse.isPressed(RT::VirtualMouse::Buttons::Mouse7) ? "7" :  " ")
            << "]" << std::endl;
    }
    private:
    GLFW::Platform* _platform;
};

/*
/// Test: 
///   - Play with mouse position/button and mode (1,2,3)
///   - Show custom cursor too
TEST_F( GLFWVirtualMouseTest, DEMO_test_mouse_mode )
{
    size_t id = 0;
    // Create cursor
    {
        auto& resources = _core->getResourceManager();
        auto& loader    = _core->getLoaderManager();
    
        auto mouseCursor = resources.openImage(MouCaEnvironment::getInputPath() / L"textures"/ L"NO_RIGHT_mouse.png");
        MouCaCore::LoadingItems items;
        items.emplace_back(MouCaCore::LoadingItem( mouseCursor,  MouCaCore::LoadingItem::Direct ));
        
        // Demand loading
        loader.loadResources( items );

        id = _platform.getEditMouse().createNewCursor(*mouseCursor->getImage().lock(), RT::Point2i(0, 0));
        resources.releaseResource( mouseCursor );
    }

    // Create window
    WindowWPtr window;
    const RT::ViewportInt32 viewport( 0, 0, 300, 300 );
    ASSERT_NO_THROW( window = _platform.createWindow( viewport, "Test - GLFWVirtualMouseTest.Mouse", RT::Window::StaticDialogMode ) );
    EXPECT_EQ( 1, _platform.getNumberWindows() );

    auto eventManager = std::make_shared<EventManagerTEST>(&_platform);
    {
        auto windowLock = window.lock();
        windowLock->initialize( eventManager, RT::Array2ui( viewport.getWidth(), viewport.getHeight() ) );

        //Apply cursor
        _platform.getEditMouse().applyCursor(*windowLock.get(), id);
    }

    _platform.runMainLoop();

    _platform.getEditMouse().deleteCursor(id);

    if( !window.expired() )
        _platform.releaseWindow( window );
}
*/
// EnableCodeCoverage

}