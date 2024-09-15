#include "Dependencies.h"

#include "LibRT/include/RTEventManager.h"
#include "LibRT/include/RTMonitor.h"

#include "LibGLFW/include/GLFWPlatform.h"
#include "LibGLFW/include/GLFWVirtualMouse.h"
#include "LibGLFW/include/GLFWWindow.h"

namespace GLFW
{

Window::Window(const RT::Monitor& monitor, const std::string& windowName, const Mode mode, Platform* manager):
_window(nullptr), _manager(manager)
{
    MouCa::preCondition(monitor.getHandle() != nullptr);
    //MouCa::preCondition(isMode(mode, Visible) == GLFW_TRUE);

    auto* handleMonitor = reinterpret_cast<GLFWmonitor*>(monitor.getHandle());
    const GLFWvidmode* videoMode = glfwGetVideoMode(handleMonitor);

    if(isMode(mode, Windowed) == GLFW_TRUE)
    {
        glfwWindowHint(GLFW_FLOATING, isMode(mode, OnTop));

        glfwWindowHint(GLFW_RED_BITS,       videoMode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS,     videoMode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS,      videoMode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE,   videoMode->refreshRate);        

        _window = glfwCreateWindow(videoMode->width, videoMode->height, windowName.c_str(), handleMonitor, nullptr);
    }
    else
    {
        _window = glfwCreateWindow(videoMode->width, videoMode->height, windowName.c_str(), handleMonitor, nullptr);
    }    
    MouCa::assertion(_window != nullptr);

    finalizeWindowHandler();
}

Window::Window(const RT::ViewportInt32& viewport, const std::string& windowName, const Mode mode, Platform* manager):
_window(nullptr), _manager(manager)
{
    glfwWindowHint(GLFW_RESIZABLE, isMode(mode, Resizable));
    glfwWindowHint(GLFW_DECORATED, isMode(mode, Border));
    glfwWindowHint(GLFW_VISIBLE,   isMode(mode, Visible));
    glfwWindowHint(GLFW_FLOATING,  isMode(mode, OnTop));
    
    //glfwWindowHint(GLFW_FLOATING,   ((mode & Visible)   == mode) ? GLFW_TRUE : GLFW_FALSE);
    //glfwWindowHint(GLFW_MAXIMIZED,  ((mode & Visible)   == mode) ? GLFW_TRUE : GLFW_FALSE);

    //Build window
    _window = glfwCreateWindow(viewport.getWidth(), viewport.getHeight(), windowName.c_str(), nullptr, nullptr);
    MouCa::assertion(_window != nullptr);
    
    glfwSetWindowPos(_window, viewport.getX(), viewport.getY());

    finalizeWindowHandler();
}

Window::~Window()
{
    //Parse each observer and demand to make notify code
    signalClose().emit(this);

    glfwDestroyWindow(_window);
    _window = nullptr;
}

void Window::setVisibility(bool visible)
{
    if( visible )
    {
        glfwShowWindow(_window);
    }
    else
    {
        glfwHideWindow(_window);
    }
}

void Window::finalizeWindowHandler()
{
    MouCa::preCondition(_window != nullptr);

    //Read and save handle
    _handler = glfwGetWin32Window(_window);

    //Set current wrapper to GLFW object
    glfwSetWindowUserPointer(_window, this);

    //Register close event
    glfwSetWindowCloseCallback(_window, &onDemandClose);

    // Window size/iconifier/maximized
    glfwSetWindowIconifyCallback( _window, &onIconify);
    glfwSetWindowMaximizeCallback(_window, &onMaximized);
    glfwSetWindowSizeCallback(    _window, &onResize);

    //Register input event
    glfwSetKeyCallback(_window, &onKeyEvents);

    glfwSetCursorPosCallback(  _window, &onMouseMove);
    glfwSetMouseButtonCallback(_window, &onMouseButtonEvents);
    glfwSetScrollCallback(     _window, &onMouseWheelEvents);
}

bool Window::needClose() const
{
    MouCa::assertion(_window != nullptr);
    return glfwWindowShouldClose(_window) != GLFW_FALSE;
}

void Window::onDemandClose(GLFWwindow* glfwWindow)
{
    //Get user data (with security)
    Window* window = getWindow(glfwWindow);
    
    //Demand to manager to remove window = close.
    MouCa::assertion(window->_manager != nullptr);
    window->_manager->releaseWindow(window);
    // Check window is properly remove
    MouCa::assertion(window == nullptr);
}

void Window::onIconify(GLFWwindow* glfwWindow, int iconified)
{
    //Get user data (with security)
    Window* window = getWindow(glfwWindow);
    // Iconified
    window->_manager->minimizedWindow(window, iconified!=0);

    window->_signalStateSize.emit(window, iconified ? Iconified : Normal);
}

void Window::onMaximized(GLFWwindow* glfwWindow, int maximized)
{
    //Get user data (with security)
    Window* window = getWindow(glfwWindow);
    // Iconified
    window->_manager->minimizedWindow(window, maximized != 0);

    window->_signalStateSize.emit(window, maximized ? Maximized : Normal);
}

void Window::onResize(GLFWwindow* glfwWindow, int width, int height)
{
    MouCa::preCondition(glfwWindow != nullptr);
    MouCa::preCondition(width >= 0);
    MouCa::preCondition(height >= 0);
    //Get user data (with security)
    Window* window = getWindow(glfwWindow);
    
    window->_signalResized.emit(window, RT::Array2ui(static_cast<uint32_t>(width), static_cast<uint32_t>(height)));
}

void Window::setSize(const uint32_t width, const uint32_t height)
{
    MouCa::preCondition(_window != nullptr);
    MouCa::preCondition(width != 0);
    MouCa::preCondition(height != 0);
    
    glfwSetWindowSize(_window, width, height);
}

RT::Array2i Window::getSize() const
{
    MouCa::preCondition(_window != nullptr);
    RT::Array2i size;

    glfwGetWindowSize(_window, &size.x, &size.y);

    return size;
}

Window::StateSize Window::getStateSize() const
{
    MouCa::preCondition(_window != nullptr);

    return glfwGetWindowAttrib(_window, GLFW_ICONIFIED) 
         ? Iconified
         : ( glfwGetWindowAttrib(_window, GLFW_MAXIMIZED) )
           ? Maximized
           : Normal;
}

RT::Point2 Window::getPixelScaling() const
{
    MouCa::preCondition(_window != nullptr);

    RT::Point2 scale;
    glfwGetWindowContentScale(_window, &scale.x, &scale.y);

    return scale;
}

void Window::setStateSize(const StateSize state)
{
    MouCa::preCondition(_window != nullptr);
    MouCa::preCondition(state < NbStateSize);

    if( state == Normal )
    {
        glfwRestoreWindow(_window);
    }
    else if( state == Iconified )
    {
        glfwIconifyWindow(_window);
    }
    else if( state == Maximized )
    {
        glfwMaximizeWindow(_window);
    }
}

void Window::onKeyEvents(GLFWwindow* glfwWindow, int key, int scancode, int action, int mods)
{
    //Get user data (with security)
    Window* window = getWindow( glfwWindow );

    if( !window->_eventManager.expired() )
    {
        window->_eventManager.lock()->onKeyPress( window, key, scancode, action, mods );
    }
}

void Window::onMouseMove(GLFWwindow* glfwWindow, double xpos, double ypos)
{
    //Get user data (with security)
    Window* window = getWindow(glfwWindow);

    if( !window->_eventManager.expired() )
    {
        MouCa::preCondition(window->getResolution().x > 0 && window->getResolution().y > 0);
        // Invert Y
        double yposI = static_cast<double>(window->getResolution().y) - 1.0 - ypos;
        // Half resolution
        const RT::Point2 halfResolution = RT::Point2(static_cast<float>(window->getResolution().x), static_cast<float>(window->getResolution().y)) * 0.5f;
        const RT::Point2 normPos        = RT::Point2(static_cast<float>(xpos), static_cast<float>(yposI)) / halfResolution - RT::Point2(1.0f, 1.0f);

        // Change position of mouse
        window->_manager->getEditMouse().setPosition(RT::Point2i(xpos, ypos), normPos, RT::Point3());
        
        // Send to event manager
        window->_eventManager.lock()->onMouseMove(window, window->_manager->getMouse());
    }
}

void Window::onMouseButtonEvents(GLFWwindow* glfwWindow, int button, int action, int /*mods*/)
{
    //Get user data (with security)
    Window* window = getWindow(glfwWindow);

    if( !window->_eventManager.expired() )
    {
        const auto currentButton = static_cast<RT::VirtualMouse::Buttons>( 1 << button );
        const auto mouseButtons = window->_manager->getMouse().getPressedButtons();
        if(action == GLFW_PRESS)
        {
            // Register new pressed button
            window->_manager->getEditMouse().setPressedButtons( static_cast<RT::VirtualMouse::Buttons>(static_cast<uint16_t>(mouseButtons) | static_cast<uint16_t>(currentButton)) );

            window->_eventManager.lock()->onMousePress(window, window->_manager->getMouse(), currentButton);
        }
        else
        {
            // Unregister new pressed button
            window->_manager->getEditMouse().setPressedButtons( static_cast<RT::VirtualMouse::Buttons>(static_cast<uint16_t>(mouseButtons) & ~static_cast<uint16_t>(currentButton)) );

            window->_eventManager.lock()->onMouseRelease(window, window->_manager->getMouse(), currentButton);
        }
    }
}

void Window::onMouseWheelEvents(GLFWwindow* glfwWindow, double /*xoffset*/, double yoffset)
{
    //Get user data (with security)
    Window* window = getWindow(glfwWindow);

    if( !window->_eventManager.expired() )
    {
        window->_eventManager.lock()->onMouseWheel(window, window->_manager->getMouse(), static_cast<int>(yoffset));
    }
}

// RT::EMouseButton Window::getMouseState() const
// {
//     return static_cast<RT::EMouseButton>(
//            (glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_LEFT)   == GLFW_PRESS ? RT::LeftButton   : RT::NoButton)
//          | (glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_RIGHT)  == GLFW_PRESS ? RT::RightButton  : RT::NoButton)
//          | (glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS ? RT::MiddleButton : RT::NoButton));
// }

void Window::changeMode( const RT::VirtualMouse& mouse ) const
{
    MouCa::preCondition(dynamic_cast<const GLFW::VirtualMouse*>(&mouse) != nullptr);

    static_cast<const GLFW::VirtualMouse*>(&mouse)->applyMode( *this );
}

RT::Platform& Window::getPlatform() const
{ 
    return *_manager;
}

void Window::setWindowTitle(const Core::String& title)
{
    MouCa::preCondition(_window != nullptr);

    glfwSetWindowTitle(_window, title.c_str());
}

}