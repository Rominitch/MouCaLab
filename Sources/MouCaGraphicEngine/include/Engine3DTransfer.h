#pragma once

#include <LibVulkan/include/VKBuffer.h>

// Forward declaration
namespace RT
{
    class BufferCPUBase;
    class Image;
}
namespace Vulkan
{
    class Buffer;
    using BufferUPtr = std::unique_ptr<Buffer>;
    class CommandBuffer;
    using CommandBufferWPtr = std::weak_ptr<CommandBuffer>;
    class ContextDevice;
    class Image;
}

namespace MouCaGraphic
{
    class Engine3DTransfer
    {
        public:
            Engine3DTransfer(const Vulkan::ContextDevice& context);
            ~Engine3DTransfer();

        //-----------------------------------------------------------------------------------------
        //                                     Immediate copy
        //-----------------------------------------------------------------------------------------
            /// Only available for Vulkan Host visible and coherent
            void immediatCopyCPUToGPU(const RT::BufferCPUBase& from, Vulkan::Buffer& to);

        //-----------------------------------------------------------------------------------------
        //                                     Deferred copy
        //-----------------------------------------------------------------------------------------
            /// Prepare all buffers (need more memory)
            void indirectCopyCPUToGPU(Vulkan::CommandBufferWPtr commandBuffer, const RT::BufferCPUBase& from, Vulkan::Buffer& to);

            void indirectCopyCPUToGPU(Vulkan::CommandBufferWPtr commandBuffer, const RT::Image& from, Vulkan::Image& to);

            /// Execute all transfers
            void transfer(Vulkan::CommandBufferWPtr commandBuffer) const;

        private:

            void image2DArrayCPUToGPU(Vulkan::CommandBuffer& commandBuffer, const RT::Image& from, Vulkan::Image& to);
            void image2DCPUToGPU(Vulkan::CommandBuffer& commandBuffer, const RT::Image& from, Vulkan::Image& to);

            const Vulkan::ContextDevice& _context;

            std::vector<Vulkan::BufferUPtr> _indirectCopyBuffers;
    };
}
