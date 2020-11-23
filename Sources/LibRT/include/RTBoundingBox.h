/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibRT/include/RTEnum.h>

namespace RT
{
    class BoundingBox
    {
        public:
            BoundingBox():
            _minMaxPoint( {{Point3(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()),
                            Point3(-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity())}}),
            _radius( 0.0f )
            {
                BT_POST_CONDITION( !isValid() );
            }

            BoundingBox( const BoundingBox& copy ) = default;

            BoundingBox(const Point3& min, const Point3& max):
            _radius(0.0f)
            {
                BT_PRE_CONDITION(all( lessThanEqual(min, max)));

                _minMaxPoint[Min] = min;
                _minMaxPoint[Max] = max;

                refreshData();
                BT_POST_CONDITION( isValid() );
            }

            ~BoundingBox() = default;

        //--------------------------------------------------------------------------
        //							    Creation methods
        //--------------------------------------------------------------------------
            void expand( const std::vector<Point3>& allPoints );

            void expand( const size_t size, const RT::Point3* allPoints );

        //--------------------------------------------------------------------------
        //							Getter / Setter methods
        //--------------------------------------------------------------------------
            bool isValid() const
            {
                return all( lessThanEqual( _minMaxPoint[Min], _minMaxPoint[Max] ) );
            }

            Array3 getSize() const
            {
                return _size;
            }

            Point3 getCenter() const
            {
                return _center;
            }

            GeoFloat getRadius() const
            {
                return _radius;
            }

        private:
            enum
            {
                Min = 0,
                Max
            };
            // Major data
            std::array<Point3, 2> _minMaxPoint;

            // Computed data
            Point3	    _center;
            GeoFloat	_radius;
            Array3  	_size;

            void refreshData();
    };

}