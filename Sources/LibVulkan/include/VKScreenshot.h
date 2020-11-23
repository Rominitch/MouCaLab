/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibRT/include/RTBufferDescriptor.h>

#include <LibVulkan/include/VKFence.h>
#include <LibVulkan/include/VKImage.h>

namespace RT
{
    class BufferCPU;
    class Image;
}

namespace Vulkan
{
    class ContextDevice;
    class ContextWindow;
    class DeviceContext;
    class Image;
    class ImageOld;
    class SwapChain;

    /// \brief Allow to transfer VkImage from GPU to CPU
    class GPUImageReader
    {
        MOUCA_NOCOPY_NOMOVE(GPUImageReader);

    public:
        /// Constructor
        GPUImageReader():
        _supportsBlit(false)
        {}

        void initialize(const ContextWindow& context, const RT::ComponentDescriptor& component);

        void release(const ContextWindow& context);

        void extractTo(const VkImage& srcImage, const ContextDevice& context, const RT::Array3ui& positionSrc, const RT::Array3ui& positionDst, const RT::Array3ui& size, RT::BufferCPU& output);
        void extractTo(const VkImage& srcImage, const ContextWindow& context, const RT::Array3ui& positionSrc, const RT::Array3ui& positionDst, const RT::Array3ui& size, RT::Image& diskImage);

    protected:
        void extractTo(const VkImage& srcImage, const ContextDevice& context, const RT::Array3ui& positionSrc, const RT::Array3ui& positionDst, const RT::Array3ui& sizes);

        RT::BufferDescriptor _descriptor;       ///< Description of image buffer.
        Image                _image;            ///< Local buffer to make screenshot.
        bool                 _supportsBlit;     ///< Check if we support Blit extension.
        Fence                _fenceSync;        ///< Synchronize operation to make screenshot.
    };
}