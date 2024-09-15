/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

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
    MouCa::preCondition( !allPoints.empty() ); // DEV Issue: Need a valid list.

    for( const Point3& point : allPoints )
    {
        MouCa::assertion( std::isfinite( point.x ) ); // DEV Issue: Broken point.
        MouCa::assertion( std::isfinite( point.y ) );
        MouCa::assertion( std::isfinite( point.z ) );

        _minMaxPoint[Min].x = std::min( _minMaxPoint[Min].x, point.x );
        _minMaxPoint[Min].y = std::min( _minMaxPoint[Min].y, point.y );
        _minMaxPoint[Min].z = std::min( _minMaxPoint[Min].z, point.z );

        _minMaxPoint[Max].x = std::max( _minMaxPoint[Max].x, point.x );
        _minMaxPoint[Max].y = std::max( _minMaxPoint[Max].y, point.y );
        _minMaxPoint[Max].z = std::max( _minMaxPoint[Max].z, point.z );
    }

    // Finalize
    refreshData();
    MouCa::postCondition( all( lessThanEqual( _minMaxPoint[Min], _minMaxPoint[Max] ) ) );
    MouCa::postCondition( std::isfinite( _radius ) ); // DEV Issue: Broken BBox.
}

void BoundingBox::expand( const size_t size, const RT::Point3* allPoints )
{
    MouCa::preCondition( size > 0 && allPoints != nullptr ); // DEV Issue: Need a valid list.

    for( const Point3* point = &allPoints[0]; point != &allPoints[size]; ++point )
    {
        MouCa::assertion( std::isfinite( point->x ) ); // DEV Issue: Broken point.
        MouCa::assertion( std::isfinite( point->y ) );
        MouCa::assertion( std::isfinite( point->z ) );

        _minMaxPoint[Min].x = std::min( _minMaxPoint[Min].x, point->x );
        _minMaxPoint[Min].y = std::min( _minMaxPoint[Min].y, point->y );
        _minMaxPoint[Min].z = std::min( _minMaxPoint[Min].z, point->z );

        _minMaxPoint[Max].x = std::max( _minMaxPoint[Max].x, point->x );
        _minMaxPoint[Max].y = std::max( _minMaxPoint[Max].y, point->y );
        _minMaxPoint[Max].z = std::max( _minMaxPoint[Max].z, point->z );
    }

    // Finalize
    refreshData();
    MouCa::postCondition( all( lessThanEqual( _minMaxPoint[Min], _minMaxPoint[Max] ) ) );
    MouCa::postCondition( std::isfinite( _radius ) ); // DEV Issue: Broken BBox.
}

}
