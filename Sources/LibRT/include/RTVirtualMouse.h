/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibCore/include/BTSignal.h>

namespace RT
{
    class Image;
    class Window;

    //----------------------------------------------------------------------------
    /// \brief Software mouse position in 2D/3D.
    ///
    class VirtualMouse
    {
        MOUCA_NOCOPY_NOMOVE(VirtualMouse);
        public:
            enum class Mode : uint8_t
            {
                OSMouse,        ///< = OS mouse (position + draw).
                Invisible,      ///< = OS mouse position but draw can be changed.
                Locked,         ///< = Software mouse (position + draw).

                NbMode          ///< Control mode overflow.
            };

            enum class Buttons : uint16_t
            {
                None    = 0x0000,
                Left    = 0x0001,
                Right   = 0x0002,
                Middle  = 0x0004,
                Mouse4  = 0x0008,
                Mouse5  = 0x0010,
                Mouse6  = 0x0020,
                Mouse7  = 0x0040,
                Mouse8  = 0x0080
            };

            static constexpr bool isPressed(const Buttons b0, const Buttons b1)
            {
                return (static_cast<uint16_t>(b0) & static_cast<uint16_t>(b1));
            }

            VirtualMouse():
            _mode(Mode::OSMouse), _pressedButtons(Buttons::None)
            {}

            virtual ~VirtualMouse() = default;

            Mode getMode() const
            {
                return _mode;
            }

            void setMode( const Mode mode );

            void setPosition(const RT::Point2i& screenPosPx, const RT::Point2& normScreenPos, const RT::Point3& worldPos)
            {
                _screenPosPx   = screenPosPx;
                _normPosition  = normScreenPos;
                _worldPosition = worldPos;
            }

            void setPressedButtons(const Buttons buttons)
            {
                _pressedButtons = buttons;
            }

            const RT::Point2i& getScreenPxPosition() const
            {
                return _screenPosPx;
            }

            const RT::Point2& getNormScreenPosition() const
            {
                return _normPosition;
            }

            const RT::Point3& getWorldPosition() const
            {
                return _worldPosition;
            }

            Buttons getPressedButtons() const
            {
                return _pressedButtons;
            }

            bool isPressed(const RT::VirtualMouse::Buttons masked) const
            {
                return isPressed(_pressedButtons, masked);
            }

        //-----------------------------------------------------------------------------------------
        //                                   Build cursor methods
        //-----------------------------------------------------------------------------------------
            virtual size_t createNewCursor( const Image& image, const Point2i& center ) = 0;
            virtual void deleteCursor( const size_t id ) = 0;
            virtual void applyCursor( const RT::Window& window, const size_t id ) = 0;

            Core::Signal<const RT::VirtualMouse&>& onChangeMode() { return _onChangeMode; }

        protected:
            Mode        _mode;              ///< Current mouse mode.
            RT::Point2i _screenPosPx;       ///< Screen pixel position.
            RT::Point2  _normPosition;      ///< Window position.
            RT::Point3  _worldPosition;     ///< World position.
            Buttons     _pressedButtons;    ///< Current state of pressed mouse button.

            Core::Signal<const RT::VirtualMouse&> _onChangeMode;
    };
}