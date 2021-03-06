#include "Dependencies.h"

#include "LibRT/include/RTRenderDialog.h"

#include "LibVulkan/include/VKSurface.h"
#include "LibVulkan/include/VKWindowSurface.h"

namespace Vulkan
{

WindowSurface::WindowSurface(RT::RenderDialogWPtr linkWindow) :
_linkWindow(linkWindow), _handle(linkWindow.lock()->getHandle())
{
    MOUCA_PRE_CONDITION(!linkWindow.expired());
}

}
