/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include "LibVulkan/include/VKImage.h"
#include "LibVulkan/include/VKMemory.h"

namespace Vulkan
{
    class Device;
    class ImageOld;
    class ImageViewOld;
    class RenderPass;
    using RenderPassWPtr = std::weak_ptr<RenderPass>;

    class SwapChain;

    //----------------------------------------------------------------------------
    /// \brief Manage a framebuffer.
    ///
    class FrameBuffer
    {
        MOUCA_NOCOPY(FrameBuffer);

        public:
            using Attachments = std::vector<VkImageView>;

            FrameBuffer();
            ~FrameBuffer();

            void initialize(const Device& device, const RenderPassWPtr renderPass, const VkExtent2D& resolution, const Attachments& attachments);

            void release(const Device& device);

            bool isNull() const
            {
                return _frameBuffer == VK_NULL_HANDLE;
            }

            const VkFramebuffer getInstance() const
            {
                MouCa::assertion(!isNull());
                return _frameBuffer;
            }

            const VkExtent2D& getResolution() const
            {
                return _resolution;
            }

            const RenderPassWPtr getRenderPass() const { return _renderPass; }

        private:
            RenderPassWPtr  _renderPass;         ///< Keep link to render pass to allow rebuild.

            VkFramebuffer   _frameBuffer;
            VkExtent2D      _resolution;
    };

    using FrameBufferSPtr = std::shared_ptr<FrameBuffer>;
    using FrameBufferWPtr = std::weak_ptr<FrameBuffer>;
    using FrameBuffers    = std::vector<FrameBufferSPtr>;
}