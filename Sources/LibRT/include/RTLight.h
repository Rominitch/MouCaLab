/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibRT/include/RTObject.h>

namespace RT
{
    class Light : public Object
    {
        public:
            enum class Form
            {
                Point,
                Directional,
                OmniDirectional
            };

            /// Constructor
            Light(const Core::String& label = "Default Light"):
            Object(TLight, label), _form(Form::Point), _color(1.0, 1.0, 1.0), _radius(100.0)
            {}

            Light(const Form form, const Point3& color, const float radius, const Core::String& label = "Default Light"):
            Object(TLight, label), _form(form), _color(color), _radius(radius)
            {}

            /// Destructor
            ~Light() override = default;

            Form getForm() const
            {
                return _form;
            }

            Point3 getColor() const
            {
                return _color;
            }

            float getRadius() const
            {
                return _radius;
            }

        private:
            Form    _form;      ///< Form of source.
            Point3  _color;     ///< RBG color of source (by default = white color).
            float   _radius;    ///< Allow to skip quickly light without many power (in scene unit).
    };
    using LightSPtr = std::shared_ptr<Light>;
    using LightWPtr = std::weak_ptr<Light>;
}