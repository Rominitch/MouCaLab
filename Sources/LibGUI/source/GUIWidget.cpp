#include "Dependencies.h"

#include <LibRT/include/RTMesh.h>
#include <LibRT/include/RTBufferCPU.h>

#include <LibGUI/include/GUIWidget.h>

namespace GUI
{

Widget2D::Widget2D():
_sizeRatio(1.0f, 1.0f), _anchor(Widget2D::CornerLeftTop)
{}

void Widget2D::initialize(const RT::Point3& positionRatio, const RT::Point2& sizeRatio, const Anchor anchor)
{
    MouCa::preCondition(sizeRatio.x > 0.0f);
    MouCa::preCondition(sizeRatio.y > 0.0f);
    
    MouCa::preCondition((anchor & MaskHorizontal) == HLeft || (anchor & MaskHorizontal) == HCenter || (anchor & MaskHorizontal) == HRight);
    MouCa::preCondition((anchor & MaskVertical) == VBottom || (anchor & MaskVertical)   == VCenter || (anchor & MaskVertical)   == VTop);
    
    _positionRatio  = positionRatio;
    _sizeRatio      = sizeRatio;
    _anchor         = anchor;

    MouCa::postCondition(_sizeRatio.x > 0.0f);
    MouCa::postCondition(_sizeRatio.y > 0.0f);
}

}
