/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include "LibRT/include/RTEventManager.h"
#include "LibRT/include/RTCanvas.h"

namespace RT
{

Canvas::Canvas():
_resolutionPixel(0, 0)
{
    BT_PRE_CONDITION(isNull());
}

Canvas::~Canvas()
{
    BT_PRE_CONDITION(isNull()); // DEV Issue: missing release();
}

void Canvas::initialize(const EventManagerWPtr& eventManager, const Array2ui& resolution)
{
    BT_PRE_CONDITION(isNull());

    _eventManager    = eventManager;
    _resolutionPixel = resolution;

    BT_POST_CONDITION(!isNull());
}

void Canvas::release()
{
    BT_PRE_CONDITION(!isNull());

    _eventManager.reset();
    _resolutionPixel = Array2ui(0, 0);

    BT_POST_CONDITION(isNull());
}

}