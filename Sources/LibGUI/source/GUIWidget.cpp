#include "Dependancies.h"

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
    BT_PRE_CONDITION(sizeRatio.x > 0.0f);
    BT_PRE_CONDITION(sizeRatio.y > 0.0f);
    
    BT_PRE_CONDITION((anchor & MaskHorizontal) == HLeft || (anchor & MaskHorizontal) == HCenter || (anchor & MaskHorizontal) == HRight);
    BT_PRE_CONDITION((anchor & MaskVertical) == VBottom || (anchor & MaskVertical)   == VCenter || (anchor & MaskVertical)   == VTop);
    
    _positionRatio  = positionRatio;
    _sizeRatio      = sizeRatio;
    _anchor         = anchor;

    BT_POST_CONDITION(_sizeRatio.x > 0.0f);
    BT_POST_CONDITION(_sizeRatio.y > 0.0f);
}

}
