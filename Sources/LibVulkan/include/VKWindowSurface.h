/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibVulkan/include/VKSurface.h>

// Forward declaration
namespace RT
{
    class RenderDialog;
    using RenderDialogWPtr = std::weak_ptr<RenderDialog>;
}

namespace Vulkan
{
    //----------------------------------------------------------------------------
    /// \brief Vulkan Surface linked to real window.
    class WindowSurface final
    {
        MOUCA_NOCOPY(WindowSurface);

        public:
            RT::RenderDialogWPtr    _linkWindow;    ///< [LINK] Link to existing window.
            SurfaceUPtr             _surface;       ///< [OWNERSHIP] Vulkan surface.

            /// Constructor
            WindowSurface(RT::RenderDialogWPtr linkWindow);
            ~WindowSurface() = default;

            void* getHandle() const     { return _handle; }

        private:
            void*       _handle;        ///< Just to compare to find window.
    };

    using WindowSurfaceUPtr = std::unique_ptr<WindowSurface>;
    using WindowSurfaces    = std::vector<WindowSurfaceUPtr>;
}
