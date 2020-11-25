#include "Dependancies.h"
#include "LibGLFW/include/GLFWHardware.h"

namespace GLFW
{

void Hardware::readHardware()
{
    // Read number of monitor
    int count;
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    if(count == 0)
    {
        BT_THROW_ERROR(u8"GLFWError", u8"NoMonitorError");
    }
    
    MOUCA_ASSERT(monitors !=nullptr);
    _monitors.resize(static_cast<size_t>(count));

    GLFWmonitor** itMonitor = monitors;
    for(auto& monitor : _monitors)
    {
        // New video mode
        int nbVideoMode;
        const GLFWvidmode* modes = glfwGetVideoModes(*itMonitor, &nbVideoMode);
        const GLFWvidmode& bestMonitor = modes[nbVideoMode-1];

        int widthMM, heightMM;
        glfwGetMonitorPhysicalSize(*itMonitor, &widthMM, &heightMM);
        const RT::Array2 sizeMM(static_cast<float>(widthMM), static_cast<float>(heightMM));

        int xpos, ypos;
        glfwGetMonitorPos(*itMonitor, &xpos, &ypos);

        const RT::ViewportInt32 viewport(xpos, ypos, static_cast<uint32_t>(bestMonitor.width), static_cast<uint32_t>(bestMonitor.height));
        monitor.initialize(Core::String(glfwGetMonitorName(*itMonitor)), viewport, sizeMM, bestMonitor.refreshRate, *itMonitor);

        ++itMonitor;
    }
}

}