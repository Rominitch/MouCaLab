#pragma once

#include <LibRT/include/RTVirtualMouse.h>

namespace GLFW
{
    class Window;

    //----------------------------------------------------------------------------
    /// \brief Software mouse position in 2D/3D for GLFW.
    ///
    class VirtualMouse final : public RT::VirtualMouse
    {
        public:
            VirtualMouse() = default;
            ~VirtualMouse() override = default;

            void applyMode(const Window& window) const;

        //-----------------------------------------------------------------------------------------
        //                                   Build cursor methods
        //-----------------------------------------------------------------------------------------
            size_t createNewCursor( const RT::Image& image, const RT::Point2i& center ) override;
            void deleteCursor( const size_t id ) override;
            void applyCursor( const RT::Window& window, const size_t id ) override;

        private:
            std::vector<GLFWcursor*> _cursors;
    };
}