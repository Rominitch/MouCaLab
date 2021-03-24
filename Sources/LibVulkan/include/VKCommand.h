/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace Vulkan
{
    // Forward declaration
    class Buffer;
    using BufferWPtr = std::weak_ptr<Buffer>;

    class Device;

    class DescriptorSet;
    class FrameBuffer;
    using FrameBufferWPtr = std::weak_ptr<FrameBuffer>;

    class Mesh;
    class MeshIndirect;
    class GraphicsPipeline;
    class PipelineLayout;

    class RenderPass;
    using RenderPassWPtr = std::weak_ptr<RenderPass>;

    class Image;
    
    //----------------------------------------------------------------------------
    /// \brief Data to execute command.
    /// Contains command buffer where execute command + id of swap buffer to select custom part.
    struct ExecuteCommands
    {
        const VkCommandBuffer& commandBuffer;   ///< Command Buffer.
        const uint32_t       idSwap;          ///< Id of swapchain (when used).
    };

    ///	\brief	Abstract command to make a command list for CommandBuffer.
    class Command
    {
        MOUCA_NOCOPY(Command);

        public:
            virtual ~Command() = default;

            [[deprecated]] virtual void execute(const VkCommandBuffer& commandBuffer) = 0;
            virtual void execute(const ExecuteCommands& executer) = 0;

        protected:
            Command() = default;
    };

    using CommandUPtr = std::unique_ptr<Command>;
    using CommandSPtr = std::shared_ptr<Command>;
    using CommandWPtr = std::weak_ptr<Command>;
    using Commands    = std::vector<CommandUPtr>;

    class CommandBeginRenderPass final : public Command
    {
        public:
            CommandBeginRenderPass(const RenderPass& renderPass, FrameBufferWPtr frameBuffer, const VkRect2D& renderArea, std::vector<VkClearValue>&& clearColor, const VkSubpassContents subpassContent);
            void execute(const VkCommandBuffer& commandBuffer) override;
            void execute(const ExecuteCommands& executer) override;

        private:
            FrameBufferWPtr           _frameBuffer;

            std::vector<VkClearValue> _clearColor;
            VkRenderPassBeginInfo     _renderPassBeginInfo;
            VkSubpassContents         _subpassContent;
    };

    class CommandBeginRenderPassSurface final : public Command
    {
    public:
        CommandBeginRenderPassSurface(const RenderPass& renderPass, std::vector<FrameBufferWPtr>&& frameBuffer, const VkRect2D& renderArea, std::vector<VkClearValue>&& clearColor, const VkSubpassContents subpassContent);
        void execute(const VkCommandBuffer& commandBuffer) override;
        void execute(const ExecuteCommands& executer) override;

    private:
        std::vector<FrameBufferWPtr> _frameBuffer;

        std::vector<VkClearValue> _clearColor;
        VkRenderPassBeginInfo     _renderPassBeginInfo;
        VkSubpassContents         _subpassContent;
    };

    class CommandEndRenderPass final : public Command
    {
        public:
            void execute(const VkCommandBuffer& commandBuffer) override;
            void execute(const ExecuteCommands& executer) override;
    };

    class CommandViewport final : public Command
    {
        private:
            VkViewport _viewport;
        public:
            CommandViewport(const VkViewport& viewport) :
            _viewport(viewport)
            {}

            void execute(const VkCommandBuffer& commandBuffer) override;
            void execute(const ExecuteCommands& executer) override;
    };

    class CommandScissor final : public Command
    {
        private:
            VkRect2D _scissor;
        public:
            CommandScissor(const VkRect2D& scissor) :
            _scissor(scissor)
            {}

            void execute(const VkCommandBuffer& commandBuffer) override;
            void execute(const ExecuteCommands& executer) override;
    };

    class CommandPipeline final : public Command
    {
        private:
            const GraphicsPipeline& _pipeline;
            VkPipelineBindPoint     _bindPoint;

        public:
            CommandPipeline(const GraphicsPipeline& pipeline, const VkPipelineBindPoint bindPoint);

            void execute(const VkCommandBuffer& commandBuffer) override;
            void execute(const ExecuteCommands& executer) override;
    };

    class CommandDraw final : public Command
    {
        private:
            const uint32_t _vertexCount;
            const uint32_t _instanceCount;
            const uint32_t _firstVertex;
            const uint32_t _firstInstance;

        public:
            CommandDraw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance);

            void execute(const VkCommandBuffer& commandBuffer) override;
            void execute(const ExecuteCommands& executer) override;
    };

    class CommandDrawIndexed final : public Command
    {
        private:
            const uint32_t _indexCount;
            const uint32_t _instanceCount;
            const uint32_t _firstIndex;
            const int32_t  _vertexOffset;
            const uint32_t _firstInstance;

        public:
            CommandDrawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const int32_t vertexOffset, const uint32_t firstInstance);

            void execute(const VkCommandBuffer& commandBuffer) override;
            void execute(const ExecuteCommands& executer) override;
    };
    using CommandDrawIndexedUPtr = std::unique_ptr<CommandDrawIndexed>;

    class CommandBindVertexBuffer : public Command
    {
        public:
            CommandBindVertexBuffer(const uint32_t firstBinding, const uint32_t bindingCount, std::vector<BufferWPtr>&& buffer, std::vector<VkDeviceSize>&& offsets);
            [[deprecated]] CommandBindVertexBuffer(const uint32_t firstBinding, const uint32_t bindingCount, std::vector<VkBuffer>&& buffer, std::vector<VkDeviceSize>&& offsets);

            void execute(const VkCommandBuffer& commandBuffer) override;
            void execute(const ExecuteCommands& executer) override;

        private:
            std::vector<Vulkan::BufferWPtr> _buffers;

            uint32_t _firstBinding;
            uint32_t _bindingCount;
            std::vector<VkBuffer>     _buffersId;
            std::vector<VkDeviceSize> _offsets;
    };

    class CommandBindIndexBuffer : public Command
    {
        public:
            CommandBindIndexBuffer(const BufferWPtr buffer, const VkDeviceSize offset, const VkIndexType indexType);
            [[deprecated]] CommandBindIndexBuffer(const VkBuffer buffer, const VkDeviceSize offset, const VkIndexType indexType);

            void execute(const VkCommandBuffer& commandBuffer) override;
            void execute(const ExecuteCommands& executer) override;

        private:
            BufferWPtr _buffer;

            VkBuffer        _bufferId;
            VkDeviceSize    _offset;
            VkIndexType     _indexType;
    };
/*
    class CommandBindMesh : public Command
    {
        private:
            const VkIndexType   _index;
            const VkBuffer      _indices;
            const VkBuffer      _vertices;
            const uint32_t      _bindID;
        protected:
            const VkDeviceSize  _offsets;

        public:
            CommandBindMesh(const Mesh& mesh, const uint32_t bindID, const VkIndexType index = VK_INDEX_TYPE_UINT32);

            void execute(const VkCommandBuffer& commandBuffer) override;
            void execute(const ExecuteCommands& executer) override;
    };

    class CommandDrawMeshIndexed final : public CommandBindMesh
    {
        private:
            std::vector<CommandDrawIndexedUPtr> _indexed;

        public:
            CommandDrawMeshIndexed(const Mesh& mesh, const uint32_t bindID);

            void execute(const VkCommandBuffer& commandBuffer) override;
            void execute(const ExecuteCommands& executer) override;
    };

    class CommandDrawMesh final : public CommandBindMesh
    {
        private:
            uint32_t            _indicesCount;
            uint32_t            _instanceCount;

        public:
            CommandDrawMesh(const Mesh& mesh, const uint32_t bindID, const uint32_t instanceCount);

            void execute(const VkCommandBuffer& commandBuffer) override;
            void execute(const ExecuteCommands& executer) override;
    };

    class CommandDrawLines final : public CommandBindMesh
    {
        private:
            uint32_t            _indicesCount;
            uint32_t            _instanceCount;
            const float         _widthLine;
        public:
            CommandDrawLines(const Mesh& mesh, const uint32_t bindID, const uint32_t instanceCount, const float widthLine);

            void execute(const VkCommandBuffer& commandBuffer) override;
            void execute(const ExecuteCommands& executer) override;
    };
*/
    class CommandBindDescriptorSets final : public Command
    {
        private:
            VkPipelineLayout                    _pipelineLayoutID;
            VkPipelineBindPoint                 _bindPoint;
            const uint32_t                      _firstSet;
            const std::vector<VkDescriptorSet>& _descriptorsID;
            std::vector<uint32_t>               _dynamicOffsets;

        public:
            CommandBindDescriptorSets(const PipelineLayout&               pipelineLayout, 
                                      const VkPipelineBindPoint           bindPoint,
                                      const uint32_t                      firstSet,
                                      const std::vector<VkDescriptorSet>& descriptors,
                                      std::vector<uint32_t>&&        dynamicOffsets);

            void execute(const VkCommandBuffer& commandBuffer) override;
            void execute(const ExecuteCommands& executer) override;
    };

    class CommandCopyImage final : public Command
    {
        private:
            const VkImage     _source;
            const VkImage     _destination;
            const VkImageCopy _copyRegion;

        public:
            CommandCopyImage(const VkImage& source, const VkImage& destination, const VkImageCopy& copyRegion);

            void execute(const VkCommandBuffer& commandBuffer) override;
            void execute(const ExecuteCommands& executer) override;
    };

    class CommandCopy final : public Command
    {
        private:
            const VkBuffer     _source;
            const VkBuffer     _destination;
            const VkBufferCopy _copyRegion;

        public:
            CommandCopy(const Buffer& source, const Buffer& destination, const VkBufferCopy& copyRegion);

            void execute(const VkCommandBuffer& commandBuffer) override;
            void execute(const ExecuteCommands& executer) override;
    };

    class CommandCopyBufferToImage final : public Command
    {
        private:
            const VkBuffer                       _source;
            const VkImage                        _destination;
            const std::vector<VkBufferImageCopy> _copyRegion;

        public:
            CommandCopyBufferToImage(const Buffer& source, const Image& destination, std::vector<VkBufferImageCopy>&& copyRegion);

            void execute(const VkCommandBuffer& commandBuffer) override;
            void execute(const ExecuteCommands& executer) override;
    };

    class CommandPipelineBarrier final : public Command
    {
        public:
            using ImageMemoryBarriers   = std::vector<VkImageMemoryBarrier>;
            using MemoryBarriers        = std::vector<VkMemoryBarrier>;
            using BufferMemoryBarriers  = std::vector<VkBufferMemoryBarrier>;

            CommandPipelineBarrier(const VkPipelineStageFlags srcStage, const VkPipelineStageFlags dstStage, const VkDependencyFlags dependencyFlags,
                                   MemoryBarriers&&         memoryBarriers,
                                   BufferMemoryBarriers&&   bufferMemoryBarriers,
                                   ImageMemoryBarriers&&    imageBarriers);

            void execute(const VkCommandBuffer& commandBuffer) override;
            void execute(const ExecuteCommands& executer) override;

        private:
            const MemoryBarriers        _memoryBarriers;
            const BufferMemoryBarriers  _bufferMemoryBarriers;
            const ImageMemoryBarriers   _imageBarriers;
            
            const VkPipelineStageFlags  _srcStage;
            const VkPipelineStageFlags  _dstStage;
            const VkDependencyFlags     _dependencyFlags;
    };

    class CommandBlit final : public Command
    {
        private:
            const VkImageBlit&  _region;
            const VkImage       _source;
            const VkImage       _destination;

        public:
            CommandBlit(const VkImage& source, const VkImage& destination, const VkImageBlit& region);

            void execute(const VkCommandBuffer& commandBuffer) override;
            void execute(const ExecuteCommands& executer) override;
    };

    class CommandPushConstants final : public Command
    {
        private:
            const VkPipelineLayout      _pipelineLayout;
            const VkShaderStageFlags    _stage;
            const uint32_t              _memorySize;
            const void*                 _buffer;        ///< Pointer to buffer: WARNING must be valid !!!

        public:
            CommandPushConstants(const PipelineLayout& pipelineLayout, const VkShaderStageFlags stage, const uint32_t memorySize, const void* buffer);

            void execute(const VkCommandBuffer& commandBuffer) override;
            void execute(const ExecuteCommands& executer) override;
    };

    /// Special Command to contain other command
    class CommandContainer : public Command
    {
        public:
            CommandContainer() = default;
            ~CommandContainer() override = default;

            void clear()
            {
                _commands.clear();
            }

            void transfer(Commands&& commands)
            {
                MOUCA_PRE_CONDITION(!commands.empty());
                for(auto& command : commands)
                {
                    _commands.emplace_back(std::move(command));
                }
                // Release old container
                commands.clear();
            }

            void transfer(CommandUPtr&& command)
            {
                MOUCA_PRE_CONDITION(command != nullptr);
                _commands.emplace_back(std::move(command));
            }

            Commands& getCommands() { return _commands; }

            void execute(const VkCommandBuffer& commandBuffer) override;
            void execute(const ExecuteCommands& executer) override;

        protected:
            Commands _commands;
    };

    /// Special Command to contain other command
    class CommandSwitch final : public CommandContainer
    {
        public:
            CommandSwitch();
            ~CommandSwitch() override = default;

            void setIdNode(const size_t idNode)
            {
                MOUCA_PRE_CONDITION(idNode < _commands.size());
                _idNode = idNode;
            }

            void execute(const VkCommandBuffer& commandBuffer) override;
            void execute(const ExecuteCommands& executer) override;

        private:
            size_t  _idNode = 0;
    };

    class CommandBuildAccelerationStructures final : public Command
    {
        public:
            CommandBuildAccelerationStructures(const Device& device, std::vector<VkAccelerationStructureBuildGeometryInfoKHR>&& buildGeometries,
                std::vector<const VkAccelerationStructureBuildRangeInfoKHR*>&& accelerationBuildStructureRangeInfos);
            ~CommandBuildAccelerationStructures() override = default;


            void execute(const VkCommandBuffer& commandBuffer) override;
            void execute(const ExecuteCommands& executer) override;

        private:
            const Device& _device;
            const std::vector<VkAccelerationStructureBuildGeometryInfoKHR>     _buildGeometries;
            const std::vector<const VkAccelerationStructureBuildRangeInfoKHR*> _accelerationBuildStructureRangeInfos;
    };
}