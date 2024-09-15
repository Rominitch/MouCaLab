#include "Dependencies.h"

#include <LibGLFW/include/GLFWPlatform.h>
#include <LibGLFW/include/GLFWWindow.h>

namespace GLFW
{

Platform::Platform():
_initialize(false), _runLoop(false)
{}

Platform::~Platform()
{
    MouCa::assertion(!_initialize); // DEV Issue: missing release call !
    MouCa::assertion(_windows.empty());
    MouCa::assertion(!_runLoop);
}

void Platform::initialize()
{
    MouCa::preCondition(isNull());
    
    // Set error callback
    glfwSetErrorCallback(Platform::errorCallback);

    //Start GLFW
    if(glfwInit() == GLFW_FALSE)
    {
        throw Core::Exception(Core::ErrorData("GLFWError", "InitializeError"));
    }

    //Default configuration (Vulkan config)
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    _initialize = true;

    MouCa::postCondition(!isNull());
}

void Platform::release()
{
    MouCa::preCondition(!isNull());
    MouCa::preCondition(_windows.empty());

    //Stop GLFW
    glfwTerminate();
    _initialize = false;

    MouCa::postCondition(isNull());
}

WindowWPtr Platform::createWindow(const RT::ViewportInt32& viewport, const Core::String& windowsName, const RT::Window::Mode mode)
{
    MouCa::preCondition(!isNull());
    WindowSPtr window(new Window(viewport, windowsName, mode, this));
    _windows.push_back(window);
    
    // Get all signal
    auto weak = WindowWPtr(window);
    _mainMouse.onChangeMode().connectWeakMember(weak, &Window::changeMode);

    return weak;
}

WindowWPtr Platform::createWindow(const RT::Monitor& monitor, const Core::String& windowsName, const RT::Window::Mode mode)
{
    MouCa::preCondition(!isNull());
    WindowSPtr window(new Window(monitor, windowsName, mode, this));
    _windows.push_back(window);

    return WindowWPtr(window);
}

void Platform::releaseWindow(const Windows::iterator& itWindow)
{
    MouCa::preCondition(!isNull());
    MouCa::preCondition(itWindow->use_count() == 1); // Be sure we have latest element (current) !
    MouCa::preCondition(itWindow != _windows.end());

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
    MouCa::preCondition(!isNull());
    MouCa::assertion(window != NULL);
    std::lock_guard<std::mutex> guard(_guardWindows);

    const auto itWindow = std::find_if(_windows.begin(), _windows.end(), [&](const WindowSPtr& SWindow) { return (SWindow.get() == window); });
    releaseWindow(itWindow);
    
    // Unlink pointer
    window = nullptr;
}

void Platform::releaseWindow(const WindowWPtr& weakWindow)
{
    MouCa::preCondition(!isNull());
    MouCa::preCondition(!weakWindow.expired());

    std::lock_guard<std::mutex> guard(_guardWindows);

    const auto itWindow = std::find(_windows.begin(), _windows.end(), weakWindow.lock());
    releaseWindow(itWindow);
}

bool Platform::isWindowsActive()
{
    MouCa::assertion(_initialize);
    std::lock_guard<std::mutex> guard(_guardWindows);
    return !_windows.empty();
}

void Platform::errorCallback(int error, const char* description)
{
    fprintf(stderr, "Error: %d - %s\n", error, description);
}

void Platform::pollEvents()
{
    MouCa::preCondition(!isNull());
    glfwPollEvents();
}

void Platform::runMainLoop()
{
    MouCa::preCondition(!isNull());
    MouCa::assertion(!_windows.empty());    //DEV Issue: launch loop without window !
    
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
    MOUCA_UNUSED(window);
    MOUCA_UNUSED(minimized);
    // Minimized / restore all ?
}

}