#pragma once

#include <LibVulkan/include/VKScreenFrame.h>
#include <LibVulkan/include/VKSurfaceFormat.h>
#include <LibVulkan/include/VKSwapChain.h>

namespace MouCa3DEngine
{
    class ContextDevice;
    class Surface;

    //----------------------------------------------------------------------------
    /// \brief Collection of all Vulkan object to renderer on window.
    /// \author Romain MOURARET
    class ContextWindow
    {
        public:
            ContextWindow() = default;
            ~ContextWindow() = default;

            void initialize(ContextDevice& linkContext, Surface& linkSurface);

        private:
            ContextDevice const*        _linkContext;
            Surface const*              _linkSurface;

            Vulkan::ScreenFrame         _screenFrame;
            Vulkan::SwapChain           _swapChain;
            Vulkan::SurfaceFormatSPtr   _format;

            //RenderFrameManager                  _context;
            //std::vector<RendererSequencerUPtr>  _sequencers;
    };

}