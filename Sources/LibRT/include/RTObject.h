/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibRT/include/RTBoundingBox.h>
#include <LibRT/include/RTHierarchic.h>
#include <LibRT/include/RTOrientation.h>
#include <LibRT/include/RTTypeInfo.h>

namespace RT
{
    ///	\brief	Abstract object.
    ///         This class is the main object representing for 2D/3D renderer.
    ///         We have matrix.
    ///	\see	Orientation
    class Object : public TypeInfo, public Hierarchic
    {
        public:
            /// State of node into scene graph.
            /// Bits 1: static/dynamic geometry - allow to propagate properly message
            /// Bits 1:
            enum State
            {
                Disabled            = 0,

                Static              = 0x00000001,
                Dynamic             = 0x00000002,

                VisibilityLow       = 0x00000100,
                VisibilityMedium    = 0x00000200,
                VisibilityHigh      = 0x00000400,
                VisibilityAlways    = VisibilityLow | VisibilityMedium | VisibilityHigh,
                VisibilityMask      = 0x00000F00,

                StaticVisible       = VisibilityAlways | Static
            };

            Object(const Type type=TObject, const Core::String& strLabel = u8"Default Object"):
            TypeInfo(type, strLabel), _state(StaticVisible)
            {}

            ~Object() override = default;

            virtual void release()
            {}

            Orientation& getOrientation()
            {
                return _local;
            }

            const Orientation& getOrientation() const
            {
                return _local;
            }

            void setState(const State state, const State mask)
            {
                _state = (State)((_state & ~mask) | state);
            }

            bool isState(const State checkState) const
            {
                return checkState & _state;
            }

        protected:
            BoundingBox	    _boundingBox;       ///< Box in 3D Space.
            Orientation     _local;             ///< Matrix system of object.
            Orientation     _global;            ///< Matrix system of object based on parent (avoid recompute parsing each time).
            State           _state;             ///< State of node into scene graph.

        private:
            MOUCA_NOCOPY_NOMOVE(Object);
    };

    using ObjectSPtr = std::shared_ptr<Object>;
    using ObjectWPtr = std::weak_ptr<Object>;
}

