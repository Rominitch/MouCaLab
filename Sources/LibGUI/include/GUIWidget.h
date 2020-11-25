#pragma once

#include <LibCore/include/CoreIdentifier.h>

#include <LibRT/include/RTHierarchic.h>

namespace RT
{
    class Mesh;
}

namespace GUI
{

    //----------------------------------------------------------------------------
    /// \brief Main basic widget for GUI. We just implement all features to manage it except rendering which will be done by specific renderer systems.
    /// 
    /// \see   RT::Hierarchic
    class Widget : public RT::Hierarchic, public Core::Identifier
    {
        MOUCA_NOCOPY_NOMOVE(Widget);
        
        public:
            enum class State : uint8_t
            {
                Disable = 0,
                //
                Visible = 0x00000001,
                Enabled = 0x00000002,
                Focus   = 0x00000004,
                //Mouse interaction
                Hover   = 0x00000010,

                // Default state
                VisibleEnable = Visible | Enabled
            };

            Widget():
            _state(State::VisibleEnable)
            {}

            ~Widget() override = default;
            
            bool isState(const State mask) const
            {
                return static_cast<State>(static_cast<uint8_t>(_state) & static_cast<uint8_t>(mask)) == mask;
            }

            void setVisible(const bool visible)
            { 
                _state = static_cast<State>((static_cast<uint8_t>(_state) & ~static_cast<uint8_t>(State::Visible)) | static_cast<uint8_t>(visible ? State::Visible : State::Disable));
            }
            
            void setEnabled(const bool enabled)
            {
                _state = static_cast<State>((static_cast<uint8_t>(_state) & ~static_cast<uint8_t>(State::Enabled)) | static_cast<uint8_t>(enabled ? State::Enabled : State::Disable));
            }

        protected:
            State       _state;     ///< Current state of widget.
    };

    //----------------------------------------------------------------------------
    /// \brief Main basic widget for GUI. We just implement all features to manage it except rendering which will be done by specific renderer systems.
    /// 
    /// \see   RT::Object
    class Widget2D : public Widget
    {
        public:
            enum Anchor
            {
                HLeft   = 0x001,
                HCenter = 0x002,
                HRight  = 0x004,
            
                VBottom = 0x010,
                VCenter = 0x020,
                VTop    = 0x040,

                CornerLeftTop    = Widget2D::HLeft | Widget2D::VTop,
                CornerLeftBottom = Widget2D::HLeft | Widget2D::VBottom,

                CornerRightTop      = Widget2D::HRight | Widget2D::VTop,
                CornerRightBottom   = Widget2D::HRight | Widget2D::VBottom,

                MaskHorizontal = 0x007,
                MaskVertical   = 0x070
            };

            Widget2D();
            ~Widget2D() override = default;

            void initialize( const RT::Point3& positionRatio, const RT::Point2& sizeRatio, const Anchor anchor );

            const RT::Point3& getPosition() const { return _positionRatio; }
            const RT::Point2& getSize() const     { return _sizeRatio; }
            Anchor            getAnchor() const   { return _anchor; }

        private:
            RT::Point3  _positionRatio;
            RT::Point2  _sizeRatio;
            Anchor      _anchor;
    };
}