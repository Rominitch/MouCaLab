/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include "LibRT/include/RTBoundingBox.h"

namespace RT
{

void BoundingBox::refreshData()
{
    _size = _minMaxPoint[Max] - _minMaxPoint[Min];
    _center = _size * 0.5f + _minMaxPoint[Min];
    _radius = length(_minMaxPoint[Max] - _center);
}

void BoundingBox::expand( const std::vector<Point3>& allPoints )
{
    BT_PRE_CONDITION( !allPoints.empty() ); // DEV Issue: Need a valid list.

    for( const Point3& point : allPoints )
    {
        BT_ASSERT( std::isfinite( point.x ) ); // DEV Issue: Broken point.
        BT_ASSERT( std::isfinite( point.y ) );
        BT_ASSERT( std::isfinite( point.z ) );

        _minMaxPoint[Min].x = std::min( _minMaxPoint[Min].x, point.x );
        _minMaxPoint[Min].y = std::min( _minMaxPoint[Min].y, point.y );
        _minMaxPoint[Min].z = std::min( _minMaxPoint[Min].z, point.z );

        _minMaxPoint[Max].x = std::max( _minMaxPoint[Max].x, point.x );
        _minMaxPoint[Max].y = std::max( _minMaxPoint[Max].y, point.y );
        _minMaxPoint[Max].z = std::max( _minMaxPoint[Max].z, point.z );
    }

    // Finalize
    refreshData();
    BT_POST_CONDITION( all( lessThanEqual( _minMaxPoint[Min], _minMaxPoint[Max] ) ) );
    BT_POST_CONDITION( std::isfinite( _radius ) ); // DEV Issue: Broken BBox.
}

void BoundingBox::expand( const size_t size, const RT::Point3* allPoints )
{
    BT_PRE_CONDITION( size > 0 && allPoints != nullptr ); // DEV Issue: Need a valid list.

    for( const Point3* point = &allPoints[0]; point != &allPoints[size]; ++point )
    {
        BT_ASSERT( std::isfinite( point->x ) ); // DEV Issue: Broken point.
        BT_ASSERT( std::isfinite( point->y ) );
        BT_ASSERT( std::isfinite( point->z ) );

        _minMaxPoint[Min].x = std::min( _minMaxPoint[Min].x, point->x );
        _minMaxPoint[Min].y = std::min( _minMaxPoint[Min].y, point->y );
        _minMaxPoint[Min].z = std::min( _minMaxPoint[Min].z, point->z );

        _minMaxPoint[Max].x = std::max( _minMaxPoint[Max].x, point->x );
        _minMaxPoint[Max].y = std::max( _minMaxPoint[Max].y, point->y );
        _minMaxPoint[Max].z = std::max( _minMaxPoint[Max].z, point->z );
    }

    // Finalize
    refreshData();
    BT_POST_CONDITION( all( lessThanEqual( _minMaxPoint[Min], _minMaxPoint[Max] ) ) );
    BT_POST_CONDITION( std::isfinite( _radius ) ); // DEV Issue: Broken BBox.
}

}
