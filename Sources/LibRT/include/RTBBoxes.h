/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibRT/include/RTMassiveInstance.h>

namespace RT
{

    class BBoxes final : public BasicMassiveInstance
    {
        MOUCA_NOCOPY_NOMOVE( BBoxes );

        public:
            BBoxes():
            BasicMassiveInstance(Object::TBBoxes)
            {}

            ~BBoxes() override = default;

            void initialize();

            void release() override;

            bool isNull() const override;

            const RT::Mesh& getMesh() const
            {
                return _bbox;
            }

            void update( const std::vector<Indirect>& indirects, const std::vector<Instance>& instances );

        private:
            RT::Mesh    _bbox;
            std::vector<RT::Point3> _nodes;
            std::vector<Edge>       _edges;
    };

    using BBoxesSPtr = std::shared_ptr<BBoxes>;
}