#include "Dependencies.h"

#include <LibRT/include/RTCamera.h>
#include <LibRT/include/RTEventManager.h>

#include <LibVulkan/include/VKEnvironment.h>

#include "MouCaGraphicEngine/include/GraphicConfiguration.h"
#include "MouCaGraphicEngine/include/GraphicEngine.h"

namespace MouCaGraphic
{

void GraphicEngine::initialize()
{
    const RT::ApplicationInfo info;

    _platform.initialize();

    _hardware.readHardware();
}

void GraphicEngine::release()
{
    _3DManager.release();

    if (!_vrPlatform.isNull())
    {
        _vrPlatform.release();
    }

    _platform.release();
}

void GraphicEngine::loopEvent()
{
    _platform.runMainLoop();
}

void GraphicEngine::getDefaultConfiguration(GraphicConfiguration& configuration) const
{
    MouCa::preCondition(!_platform.isNull());
    MouCa::preCondition(!_hardware.getMonitors().empty());
#ifndef NDEBUG
    configuration.clear();

    // For debug: little window
    GraphicConfiguration::ConfigurationWindow config
    {
        {0,0, 1500, 1000},
        {1500, 1000},
        RT::Window::StaticDialogMode
    };
#else
    // Add Primary screen windows
    GraphicConfiguration::ConfigurationWindow config
    {
        _hardware.getMonitors().begin()->getMonitorArea(),
        _hardware.getMonitors().begin()->getMonitorArea().getSize(),
        RT::Window::FullscreenWindowed
    };
#endif
    configuration.addWindow(config);
}

const std::vector<RT::Monitor>& GraphicEngine::getMonitors() const
{
    return _hardware.getMonitors();
}

}