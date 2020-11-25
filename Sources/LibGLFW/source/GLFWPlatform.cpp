#include "Dependancies.h"

#include <LibGLFW/include/GLFWPlatform.h>
#include <LibGLFW/include/GLFWWindow.h>

namespace GLFW
{

Platform::Platform():
_initialize(false), _runLoop(false)
{}

Platform::~Platform()
{
    BT_ASSERT(!_initialize); // DEV Issue: missing release call !
    BT_ASSERT(_windows.empty());
    BT_ASSERT(!_runLoop);
}

void Platform::initialize()
{
    MOUCA_PRE_CONDITION(isNull());
    
    // Set error callback
    glfwSetErrorCallback(Platform::errorCallback);

    //Start GLFW
    if(glfwInit() == GLFW_FALSE)
    {
        BT_THROW_ERROR(u8"GLFWError", u8"InitializeError");
    }

    //Default configuration (Vulkan config)
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    _initialize = true;

    MOUCA_POST_CONDITION(!isNull());
}

void Platform::release()
{
    MOUCA_PRE_CONDITION(!isNull());
    MOUCA_PRE_CONDITION(_windows.empty());

    //Stop GLFW
    glfwTerminate();
    _initialize = false;

    MOUCA_POST_CONDITION(isNull());
}

WindowWPtr Platform::createWindow(const RT::ViewportInt32& viewport, const Core::String& windowsName, const RT::Window::Mode mode)
{
    MOUCA_PRE_CONDITION(!isNull());
    WindowSPtr window(new Window(viewport, windowsName, mode, this));
    _windows.push_back(window);
    
    // Get all signal
    auto weak = WindowWPtr(window);
    _mainMouse.onChangeMode().connectWeakMember(weak, &Window::changeMode);

    return weak;
}

WindowWPtr Platform::createWindow(const RT::Monitor& monitor, const Core::String& windowsName, const RT::Window::Mode mode)
{
    MOUCA_PRE_CONDITION(!isNull());
    WindowSPtr window(new Window(monitor, windowsName, mode, this));
    _windows.push_back(window);

    return WindowWPtr(window);
}

void Platform::releaseWindow(const Windows::iterator& itWindow)
{
    MOUCA_PRE_CONDITION(!isNull());
    MOUCA_PRE_CONDITION(itWindow->use_count() == 1); // Be sure we have latest element (current) !
    MOUCA_PRE_CONDITION(itWindow != _windows.end());

    // Remove connection
    _mainMouse.onChangeMode().disconnect(itWindow->get());

    // Clean window data before delete
    if(!(*itWindow)->isNull())
    {   
        (*itWindow)->release();
    }
    _windows.erase(itWindow);
}

void Platform::releaseWindow(Window*& window)
{
    MOUCA_PRE_CONDITION(!isNull());
    BT_ASSERT(window != NULL);
    std::lock_guard<std::mutex> guard(_guardWindows);

    const auto itWindow = std::find_if(_windows.begin(), _windows.end(), [&](const WindowSPtr& SWindow) { return (SWindow.get() == window); });
    releaseWindow(itWindow);
    
    // Unlink pointer
    window = nullptr;
}

void Platform::releaseWindow(const WindowWPtr& weakWindow)
{
    MOUCA_PRE_CONDITION(!isNull());
    MOUCA_PRE_CONDITION(!weakWindow.expired());

    std::lock_guard<std::mutex> guard(_guardWindows);

    const auto itWindow = std::find(_windows.begin(), _windows.end(), weakWindow.lock());
    releaseWindow(itWindow);
}

bool Platform::isWindowsActive()
{
    BT_ASSERT(_initialize);
    std::lock_guard<std::mutex> guard(_guardWindows);
    return !_windows.empty();
}

void Platform::errorCallback(int error, const char* description)
{
    fprintf(stderr, "Error: %d - %s\n", error, description);
}

void Platform::pollEvents()
{
    MOUCA_PRE_CONDITION(!isNull());
    glfwPollEvents();
}

void Platform::runMainLoop()
{
    MOUCA_PRE_CONDITION(!isNull());
    BT_ASSERT(!_windows.empty());    //DEV Issue: launch loop without window !
    
    _runLoop = true;

    //Loop until window is closed
    while(_runLoop && isWindowsActive())
    {
        // Pool event
        glfwPollEvents();
    }

    _runLoop = false;
}

void Platform::minimizedWindow(Window*& window, const bool minimized)
{
    // Minimized / restore all ?
}

}