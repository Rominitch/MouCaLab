/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibRT/include/RTEventManager.h"
#include "LibRT/include/RTCanvas.h"

namespace RT
{

Canvas::Canvas():
_resolutionPixel(0, 0)
{
    MouCa::preCondition(isNull());
}

Canvas::~Canvas()
{
    MouCa::preCondition(isNull()); // DEV Issue: missing release();
}

void Canvas::initialize(const EventManagerWPtr& eventManager, const Array2ui& resolution)
{
    MouCa::preCondition(isNull());

    _eventManager    = eventManager;
    _resolutionPixel = resolution;

    MouCa::postCondition(!isNull());
}

void Canvas::release()
{
    MouCa::preCondition(!isNull());

    _eventManager.reset();
    _resolutionPixel = Array2ui(0, 0);

    MouCa::postCondition(isNull());
}

}