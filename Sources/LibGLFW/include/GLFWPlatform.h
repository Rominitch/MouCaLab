/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibRT/include/RTPlatform.h>
#include <LibRT/include/RTViewport.h>
#include <LibRT/include/RTWindow.h>

#include <LibGLFW/include/GLFWVirtualMouse.h>

namespace RT
{
    class Monitor;
}

namespace GLFW
{
    class Window;
    using WindowSPtr = std::shared_ptr<Window>;
    using WindowWPtr = std::weak_ptr<Window>;
    using Windows    = std::vector<WindowSPtr>;

    //----------------------------------------------------------------------------
    /// \brief GLFW platform: allow to use GLFW and create windows.
    /// \see RT::Platform
    class Platform final : public RT::Platform
    {
        MOUCA_NOCOPY_NOMOVE(Platform);

        // Google test
        FRIEND_TEST( GLFWPlatform, createWindow );
        FRIEND_TEST( GLFWPlatform, runtime );
        
        public:
            //------------------------------------------------------------------------
            /// \brief Constructor: need to call initialize() after to use it.
            /// 
            Platform();

            //------------------------------------------------------------------------
            /// \brief Destructor: need to call release() before to use it.
            /// 
            ~Platform();

            //------------------------------------------------------------------------
            /// \brief  Check if object is null.
            /// 
            /// \returns True is null, false otherwise.
            bool isNull() const
            {
                return !_initialize && _windows.empty();
            }

            //------------------------------------------------------------------------
            /// \brief  Initialize GLFW platform (FIRST to call).
            /// 
            void initialize();

            //------------------------------------------------------------------------
            /// \brief  Clean GLFW platform. Mandatory to call before exit application if initialize() was called.
            /// 
            void release();

            //------------------------------------------------------------------------
            /// \brief  Build a new window with specific size/position and name.
            /// 
            /// \param[in] viewport: Size and position of new window
            /// \param[in] windowsName: Dialog name.
            /// \param[in] mode: how open new window (movable, sizable, ...).
            /// \returns Weak pointer to window
            WindowWPtr createWindow(const RT::ViewportInt32& viewport, const Core::String& windowsName, const RT::Window::Mode mode);

            //------------------------------------------------------------------------
            /// \brief  Build a new fullscreen window with name.
            /// 
            /// \param[in] monitor: where window must be create. Read position and size to make fullscreen window.
            /// \param[in] windowsName: Dialog name.
            /// \param[in] mode: how open new window (movable, sizable, ...).
            /// \returns Weak pointer to window
            WindowWPtr createWindow(const RT::Monitor& monitor, const Core::String& windowsName, const RT::Window::Mode mode);

            //------------------------------------------------------------------------
            /// \brief  Clean an instance of window and destroy visible window.
            /// 
            /// \param[in] window: instance to clean. MUST be registered. 
            void releaseWindow(const WindowWPtr& window);

            //------------------------------------------------------------------------
            /// \brief  Clean an instance of window and destroy visible window, edit pointer to return nullptr.
            /// 
            /// \param[in,out] window: instance to clean and return nullptr after done. MUST be registered.
            void releaseWindow(Window*& window);

            //------------------------------------------------------------------------
            /// \brief  Minimize window or restore.
            /// 
            /// \param[in,out] window: instance. MUST be registered.
            void minimizedWindow(Window*& window, const bool minimized);

            //------------------------------------------------------------------------
            /// \brief Get existing window using registration ID (position in array).
            /// 
            /// \param[in] windowID: position of windows in array. MUST be valid.
            /// \returns Get shared pointer to existing window.
            WindowSPtr getWindow(const size_t windowID) const
            {
                BT_ASSERT(windowID < _windows.size());
                return _windows[windowID];
            }

            //------------------------------------------------------------------------
            /// \brief Get number of windows created in manager.
            /// 
            /// \returns Number of windows created or 0 if empty.
            size_t getNumberWindows() const
            {
                return _windows.size();
            }

            //------------------------------------------------------------------------
            /// \brief  Get readable virtual mouse.
            /// 
            /// \returns Virtual mouse.
            const RT::VirtualMouse& getMouse() const override
            {
                return _mainMouse;
            }

            //------------------------------------------------------------------------
            /// \brief  Get editable virtual mouse.
            /// 
            /// \returns Editable virtual mouse.
            RT::VirtualMouse& getEditMouse() override
            {
                return _mainMouse;
            }

            //------------------------------------------------------------------------
            /// \brief  Run event loop in current thread.
            /// Run event loop in current thread.
            /// Only way to quit it, is to close all windows or call demandExitMainLoop().
            void runMainLoop();

            //------------------------------------------------------------------------
            /// \brief  Check if windows is present into platform.
            /// 
            /// \returns True is windows inside, false otherwise.
            bool isWindowsActive() override;

            //------------------------------------------------------------------------
            /// \brief  Run events for this cycle.
            /// 
            void pollEvents() override;

        private:
            std::mutex      _guardWindows;  ///< Guardian of _windows list (for multi-threading).
            bool            _initialize;    ///< Check if initialize is call.
            bool            _runLoop;       ///< Check if runLoop is enable.
            Windows         _windows;       ///< List of all windows created.
            VirtualMouse    _mainMouse;     ///< Main mouse pointer in system.

            

            //------------------------------------------------------------------------
            /// \brief  Check if we are inside runLoop method.
            /// 
            /// \returns True if we are inside runLoop method, false otherwise.
            bool isRunLoopActive() const
            {
                return _runLoop;
            }

            //------------------------------------------------------------------------
            /// \brief  Special system to exit main loop not based on window number.
            /// 
            void demandExitMainLoop()
            {
                _runLoop = false;
            }

            //------------------------------------------------------------------------
            /// \brief  Internal GLFW callback to return error.
            /// 
            /// \param[in] error: error code
            /// \param[in] description: readable message.
            static void errorCallback(int error, const char* description);

            //------------------------------------------------------------------------
            /// \brief  Generic way to release window.
            /// 
            /// \param[in] itWindow: where is window to use. Warning: Don't use iterator after operation.
            void releaseWindow(const Windows::iterator& itWindow);
    };
}