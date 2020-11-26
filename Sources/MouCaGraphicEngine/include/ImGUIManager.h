#pragma once

#include <LibVulkan/include/VKCommand.h>

#include <MouCaGraphicEngine/include/Engine3DXMLLoader.h>

namespace RT
{
    class BufferLinkedCPU;
    using BufferLinkedCPUSPtr = std::shared_ptr<BufferLinkedCPU>;

    class Image;
    using ImageSPtr = std::shared_ptr<Image>;
    using ImageWPtr = std::weak_ptr<Image>;

    class RenderDialog;
    using RenderDialogWPtr = std::weak_ptr<RenderDialog>;
    
    class VirtualMouse;

    class Window;
}

namespace Vulkan
{
    class ContextDevice;

    class Buffer;
    using BufferSPtr = std::shared_ptr<Buffer>;
    using BufferWPtr = std::weak_ptr<Buffer>;
}

namespace MouCaGraphic
{
    class VulkanManager;

    class ImGUIManager final
    {
        public:
            struct PushConstBlock
            {
                glm::vec2 scale;
                glm::vec2 translate;
            };

            static RT::BufferDescriptor getMeshDescriptor();

            /// Constructor: Build manager and let in isNUll() state;
            ImGUIManager();
            /// Destructor: Must be in isNull() state;
            ~ImGUIManager();

            /// Initialize all data.
            /// \note Need isNull() state
            void initialize(const RT::Array2ui& resolution);

            /// Release initialized manager.
            /// \note Need !isNull() state
            void release(const Vulkan::ContextDevice& context);

            bool prepareBuffer(const Vulkan::ContextDevice& context);

            bool isNull() const { return _font == nullptr; }

            void buildCommands(Vulkan::Commands& command);

            RT::ImageWPtr getImageFont() const { return _font; }

            Vulkan::BufferWPtr getVertexBuffer() const { return _vertexBuffer; }
            Vulkan::BufferWPtr getIndexBuffer() const  { return _indexBuffer;  }

            RT::BufferLinkedCPUSPtr getPush() const { return _pushConstBuffer; }

            void updateMouse(const RT::VirtualMouse& mouse) const;

        private:
            void resize(RT::Window*, const RT::Array2ui&);

            Vulkan::BufferSPtr      _vertexBuffer;
            Vulkan::BufferSPtr      _indexBuffer;

            RT::BufferLinkedCPUSPtr _fontBuffer;        ///< Buffer linked to ImGUI memory !
            RT::ImageSPtr           _font;              ///< [OWNERSHIP] CPU image font.
            float                   _guiScale = 1.0f;

            PushConstBlock          _pushConstBlock;
            RT::BufferLinkedCPUSPtr _pushConstBuffer;
    };
}