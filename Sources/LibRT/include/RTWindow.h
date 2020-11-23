/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibCore/include/BTSignal.h>

namespace RT
{
    class Platform;

    class Window
    {
        MOUCA_NOCOPY(Window);

        public:
            using WindowHandle = void*;
            using WindowInstance = void*;
            enum Mode
            {
                Unknown       = 0,
                // Part
                Windowed      = 0x00000001,
                Visible       = 0x00000002,
                IsMaximizable = 0x00000004,

                // Feature
                Resizable   = 0x00000010,
                Border      = 0x00000020,
                OnTop       = 0x00000040,

                // User mode
                SimpleMode       = Windowed | Visible,                      ///< Simple static dialog without frame.
                DialogMode       = Windowed | Visible | Resizable | Border, ///< Classic dialog.
                StaticDialogMode = Windowed | Visible | Border,             ///< Classic dialog with static size.
                InternalMode     = Windowed,                                ///< Hidden dialog for automatic test.

                FullscreenWindowed = Windowed | Visible | OnTop,
                FullscreenReal     = Visible
            };

            enum StateSize
            {
                Normal,
                Iconified,
                Maximized,

                NbStateSize
            };

            /// Constructor
            Window():
            _handler(nullptr)
            {}

            /// Destructor
            ~Window() = default;

            WindowHandle getHandle() const
            {
                return _handler;
            }

            WindowInstance getInstance() const;

            //------------------------------------------------------------------------
            /// \brief  Change window visibility.
            /// 
            /// \param[inOut]  visible: want visible otherwise hide.
            virtual void setVisibility(bool visible) = 0;
            //------------------------------------------------------------------------
            /// \brief  Change window size.
            /// 
            /// \param[in] width: new width (MUST BE > 0).
            /// \param[in] height: new height (MUST BE > 0).
            virtual void setSize(const uint32_t width, const uint32_t height) = 0;

            //------------------------------------------------------------------------
            /// \brief  Get current window size.
            /// 
            virtual RT::Array2i getSize() const = 0;

            //------------------------------------------------------------------------
            /// \brief  Change state size of window.
            /// 
            /// \param[in] state: new state;
            /// \see StateSize
            virtual void setStateSize(const StateSize state) = 0;

            //------------------------------------------------------------------------
            /// \brief  Get state size of window.
            /// 
            /// \returns Current state;
            /// \see StateSize
            virtual StateSize getStateSize() const = 0;

            //------------------------------------------------------------------------
            /// \brief  Get pixel scaling of window based on monitor configuration.
            /// 
            /// \returns Scaling in x/y;
            virtual RT::Point2 getPixelScaling() const = 0;

            //------------------------------------------------------------------------
            /// \brief  Change window title.
            /// 
            /// \param[in] title: new title.
            virtual void setWindowTitle(const Core::String& title) = 0;

            //------------------------------------------------------------------------
            /// \brief  Get platform of window.
            /// 
            /// \returns Editable platform;
            virtual RT::Platform& getPlatform() const = 0;

            Core::Signal<Window*, StateSize>&            signalStateSize()    { return _signalStateSize; }
            Core::Signal<Window*, const RT::Array2ui&>&  signalResize()       { return _signalResized; }
            Core::Signal<Window*>&                       signalClose()        { return _signalClose; }

        protected:
            WindowHandle    _handler;   ///< OS handler

            Core::Signal<Window*, StateSize>            _signalStateSize;
            Core::Signal<Window*, const RT::Array2ui&>  _signalResized;
            Core::Signal<Window*>                       _signalClose;
    };

    using WindowSPtr = std::shared_ptr<Window>;
    using WindowWPtr = std::weak_ptr<Window>;
}