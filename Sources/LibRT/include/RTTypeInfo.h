/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace RT
{
    //----------------------------------------------------------------------------
    /// \brief Generic system to give name/type to object and save it inside same container.
    class TypeInfo
    {
        public:
            enum Type
            {
                TObject,
                TGeometry,
                TCamera,
                TLight,
                TMassive,
                TOrigin,
                TBBoxes,
                TAnimatedGeometry,

                TAgents,
                TAgent,
                TMapWorldTerrainFlat,
                TMapRealWorld,

                TPathWorld,
                TPathSolver,

                TDungeon,

                TGUIWidget,
                TGUIButton,

                NbTypes
            };

            virtual ~TypeInfo() = default;

            Type getType() const
            {
                return _type;
            }

            const Core::String& getLabel() const
            {
                return _label;
            }

            void setLabel(const Core::String& strLabel)
            {
                _label = strLabel;
            }

        protected:
            TypeInfo(const Type type = TObject, const Core::String& strLabel = u8"Default Object"):
            _type(type), _label(strLabel)
            {}

            const Type      _type;
            Core::String		_label;             ///< Debugging name.
    };
}