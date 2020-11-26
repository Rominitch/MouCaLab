#include "Dependancies.h"

#include "LibVulkan/include/VKSurface.h"

#include "MouCa3DVulkanEngine/include/ContextDevice.h"
#include "MouCa3DVulkanEngine/include/Surface.h"

#include "MouCa3DVulkanEngine/include/ContextWindow.h"

namespace MouCa3DEngine
{

void ContextWindow::initialize(ContextDevice& linkContext, Surface& linkSurface)
{
    BT_PRE_CONDITION(!linkContext.isNull());
    BT_PRE_CONDITION(!linkSurface._surface->isNull());

    _linkContext = &linkContext;
    _linkSurface = &linkSurface;
}

}