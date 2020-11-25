#include "Dependancies.h"

#include <LibRT/include/RTImage.h>

#include <LibGLFW/include/GLFWVirtualMouse.h>
#include <LibGLFW/include/GLFWWindow.h>

namespace GLFW
{

void VirtualMouse::applyMode(const Window& window) const
{
    // Apply it for GLFW
    const std::array<int, static_cast<size_t>(Mode::NbMode)> _wrapMode =
    {
        GLFW_CURSOR_NORMAL,
        GLFW_CURSOR_HIDDEN,
        GLFW_CURSOR_DISABLED
    };
    glfwSetInputMode(window.getGLFWHandler(), GLFW_CURSOR, _wrapMode[static_cast<size_t>(_mode)]);
}

size_t VirtualMouse::createNewCursor( const RT::Image& image, const RT::Point2i& center )
{
    MOUCA_PRE_CONDITION(!image.isNull());

    const uint32_t layer = 0;
    const uint32_t level = 0;
    auto* data = image.getRAWData(layer, level);

    const GLFWimage glfwImage =
    {
        static_cast<int32_t>(image.getExtents(level).x),
        static_cast<int32_t>(image.getExtents(level).y),
        reinterpret_cast<uint8_t*>(const_cast<void*>(data))
    };

    GLFWcursor* cursor = glfwCreateCursor(&glfwImage, center.x, center.y);
    BT_ASSERT( cursor != NULL );

    const size_t id = _cursors.size();
    _cursors.emplace_back( cursor );
    MOUCA_POST_CONDITION(id < _cursors.size());
    return id;
}

void VirtualMouse::deleteCursor( const size_t id )
{
    MOUCA_PRE_CONDITION(id < _cursors.size());
    auto it = _cursors.begin();
    std::advance(it, id);

    glfwDestroyCursor(*it);
    _cursors.erase(it);
}

void VirtualMouse::applyCursor( const RT::Window& window, const size_t id )
{
    MOUCA_PRE_CONDITION( id < _cursors.size() );
    auto it = _cursors.begin();
    std::advance( it, id );

    const auto glfwWindow = dynamic_cast<const GLFW::Window*>(&window);
    glfwSetCursor(glfwWindow->getGLFWHandler(), *it);
}

}