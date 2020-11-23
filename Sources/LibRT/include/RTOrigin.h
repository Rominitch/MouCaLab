/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibRT/include/RTObject.h>

namespace RT
{
    //----------------------------------------------------------------------------
    /// \brief Simple gadget to show origin of object
    ///
    class Origin : public Object
    {
        MOUCA_NOCOPY_NOMOVE( Origin );

        public:
            Origin():
            Object(Object::TOrigin),
            _colorX(1.0f, 0.0f, 0.0, 1.0f),
            _colorY(0.0f, 1.0f, 0.0, 1.0f),
            _colorZ(0.0f, 0.0f, 1.0, 1.0f)
            {}

            ~Origin() = default;

            const RT::Point4& getColorX() const { return _colorX; }
            const RT::Point4& getColorY() const { return _colorY; }
            const RT::Point4& getColorZ() const { return _colorZ; }

        private:
            const RT::Point4 _colorX;
            const RT::Point4 _colorY;
            const RT::Point4 _colorZ;
    };

    using OriginSPtr = std::shared_ptr<Origin>;
}