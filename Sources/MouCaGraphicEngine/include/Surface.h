#pragma once

// Forward declaration
namespace RT
{
    class RenderDialog;
    using RenderDialogWPtr = std::weak_ptr<RenderDialog>;
}

namespace Vulkan
{
    class Surface;
    using SurfaceUPtr = std::unique_ptr<Surface>;
}

namespace MouCa3DEngine
{
    //----------------------------------------------------------------------------
    /// \brief Vulkan Surface linked to real window.
    /// \author Romain MOURARET
    class Surface
    {
        public:
            RT::RenderDialogWPtr    _linkWindow;    ///< [LINK] Link to existing window.
            Vulkan::SurfaceUPtr     _surface;       ///< [OWNERSHIP] Vulkan surface.

            /// Constructor
            Surface(RT::RenderDialogWPtr linkWindow) :
            _linkWindow(linkWindow)
            {
                BT_PRE_CONDITION(!linkWindow.expired());
            }
    };
}
