#pragma once

#include <LibRT/include/RTRenderDialog.h>
#include <LibRT/include/RTViewport.h>

namespace RT
{
    class Monitor;
    class VirtualMouse;
}

namespace GLFW
{
    class Platform;

    ///	\brief	Manage GLFW Window.
    ///	\see	RT::RenderDialog
    class Window final : public RT::RenderDialog
    {
        // Google test
        FRIEND_TEST(VulkanDeviceContextTest, cleanWindowLikeUSERBefore);
        FRIEND_TEST(GLFWWindowTest, events);
        FRIEND_TEST(GLFWWindowTest, createWindow);
        FRIEND_TEST(GLFWWindowTest, methods);

        // Duplication forbidden
        MOUCA_NOCOPY(Window);

    private:
        GLFWwindow* _window;        ///< Window handle of GLFW.
        Platform*   _manager;       ///< Parent platform.

        //------------------------------------------------------------------------
        /// \brief  [INTERNAL API] GLFW callback to analyze key events.
        /// 
        /// \param[in] glfwWindow: window which catch event.
        /// \param[in] key: key code.
        /// \param[in] scancode: .
        /// \param[in] action: .
        /// \param[in] mods: .
        static void onKeyEvents(GLFWwindow* glfwWindow, int key, int scancode, int action, int mods);

        //------------------------------------------------------------------------
        /// \brief  [INTERNAL API] GLFW callback to analyze mouse movement events.
        /// 
        /// \param[in] glfwWindow: window which catch event.
        /// \param[in] xpos: position of mouse in X.
        /// \param[in] ypos: position of mouse in Y.
        static void onMouseMove(GLFWwindow* glfwWindow, double xpos, double ypos);

        //------------------------------------------------------------------------
        /// \brief  [INTERNAL API] GLFW callback to analyze mouse button events.
        /// 
        /// \param[in] glfwWindow: window which catch event.
        /// \param[in] button: .
        /// \param[in] action: .
        /// \param[in] mods: .
        static void onMouseButtonEvents(GLFWwindow* glfwWindow, int button, int action, int mods);

        //------------------------------------------------------------------------
        /// \brief  [INTERNAL API] GLFW callback to analyze mouse wheel events.
        /// 
        /// \param[in] glfwWindow: window which catch event.
        /// \param[in] xoffset: wheel direction event.
        /// \param[in] yoffset: wheel direction event.
        static void onMouseWheelEvents(GLFWwindow* glfwWindow, double xoffset, double yoffset);

        //------------------------------------------------------------------------
        /// \brief  [INTERNAL API] GLFW callback to analyze iconifier events.
        /// 
        /// \param[in] glfwWindow: window which send event.
        /// \param[in] iconified: True if iconified, otherwise restore.
        static void onIconify(GLFWwindow* glfwWindow, int iconified);

        //------------------------------------------------------------------------
        /// \brief  [INTERNAL API] GLFW callback to analyze maximization events.
        /// 
        /// \param[in] glfwWindow: window which send event.
        /// \param[in] maximized: True if maximized, otherwise restore.
        static void onMaximized(GLFWwindow* glfwWindow, int maximized);

        //------------------------------------------------------------------------
        /// \brief  [INTERNAL API] GLFW callback to analyze size events.
        /// 
        /// \param[in] glfwWindow: window which catch event.
        /// \param[in] width: new width.
        /// \param[in] height: new height.
        static void onResize(GLFWwindow* glfwWindow, int width, int height);

        //------------------------------------------------------------------------
        /// \brief [INTERNAL API] Retrieve RT::Window data from GLFWwindow pointer.
        /// 
        /// \param[in] glfwWindow: GLFW window ID.
        /// \returns Pointer to associate RT::Window.
        static Window* getWindow(GLFWwindow* glfwWindow)
        {
            MOUCA_ASSERT(glfwWindow != NULL);

            //Get user data
            Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
            MOUCA_ASSERT(window != NULL);
            return window;
        }

        //RT::EMouseButton getMouseState() const;

        //------------------------------------------------------------------------
        /// \brief  Finalize prepared window to connect all callback and finalized object.
        /// 
        void finalizeWindowHandler();

        //------------------------------------------------------------------------
        /// \brief Check if specific window mode is enabled.
        /// 
        /// \param[in] mode: complete window mode.
        /// \param[in] mask: state to check.
        /// \returns GLFW_TRUE if active, otherwise GLFW_FALSE.
        int isMode(const Mode mode, const Mode mask) const
        {
            return ((mode & mask) == mask) ? GLFW_TRUE : GLFW_FALSE;
        }

        //------------------------------------------------------------------------
        /// \brief  Change window visibility.
        /// 
        /// \param[inOut]  visible: want visible otherwise hide.
        void setVisibility(bool visible) override;

        //------------------------------------------------------------------------
        /// \brief  Get platform of window.
        /// 
        /// \returns Editable platform.
        RT::Platform& getPlatform() const override;

    public:
        ///	Constructor of fullscreen window based on monitor.
        ///	\param	monitor: where draw.
        ///	\param	windowName: Dialog name.
        ///	\param	mode: how to create window for OS.
        ///	\param	manager: platform who create window.
        Window(const RT::Monitor& monitor, const std::string& windowName, const Mode mode = RT::Window::FullscreenWindowed, Platform* manager = nullptr);

        ///	Constructor of window. Build window physically after operation.
        ///	\param	viewport: size 2D of window in pixel.
        ///	\param	windowName: Dialog name.
        ///	\param	mode: how to create window for OS.
        ///	\param	manager: platform who create window.
        Window(const RT::ViewportInt32& viewport, const std::string& windowName, const Mode mode = RT::Window::StaticDialogMode, Platform* manager = nullptr);

        /// Destructor
        ~Window() override;

        ///	Check is window need to be closed.
        ///	\return	True if window need to be closed, false otherwise.
        bool needClose() const;

        //------------------------------------------------------------------------
        /// \brief  Change window size (WARNING: change visibility state after operation).
        /// 
        /// \param[in] width: new width (MUST BE > 0).
        /// \param[in] height: new height (MUST BE > 0).
        void setSize(const uint32_t width, const uint32_t height) override;

        //------------------------------------------------------------------------
        /// \brief  Change window title.
        /// 
        /// \param[in] title: new title.
        void setWindowTitle(const Core::String& title) override;

        //------------------------------------------------------------------------
        /// \brief  Get current window size.
        /// 
        RT::Array2i getSize() const override;

        //------------------------------------------------------------------------
        /// \brief  Change state size of window.
        /// 
        /// \param[in] state: new state;
        /// \see StateSize
        void setStateSize(const StateSize state) override;

        //------------------------------------------------------------------------
        /// \brief  Get state size of window.
        /// 
        /// \returns Current state;
        /// \see StateSize
        StateSize getStateSize() const override;

        //------------------------------------------------------------------------
        /// \brief  Get pixel scaling of window based on monitor configuration.
        /// 
        /// \returns Scaling in x/y;
        RT::Point2 getPixelScaling() const override;

        //------------------------------------------------------------------------
        /// \brief  Get GLFW handler. Please don't abuse of this API !
        /// 
        /// \returns GLFW handler.
        GLFWwindow* getGLFWHandler() const
        {
            return _window;
        }

        //------------------------------------------------------------------------
        /// \brief  Change mouse mode (Visible, invisible, or locked).
        /// 
        /// \param[in] mouse: Current mouse data. Need to synchronize window on this data.
        void changeMode(const RT::VirtualMouse& mouse) const;

        ///	Manage GLFW event when window closure is demanded.
        ///	\param	glfwWindow: GLFW pointer on window.
        static void onDemandClose(GLFWwindow* glfwWindow);
    };

    using WindowSPtr = std::shared_ptr<Window>;
    using WindowWPtr = std::weak_ptr<Window>;
}