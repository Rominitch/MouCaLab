/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace Vulkan
{
    class Command;
    using CommandSPtr = std::shared_ptr<Command>;
    using CommandUPtr = std::unique_ptr<Command>;
    using Commands    = std::vector<CommandUPtr>;

    struct ExecuteCommands;

    class CommandPool;
    using CommandPoolSPtr = std::shared_ptr<CommandPool>;
    using CommandPoolWPtr = std::weak_ptr<CommandPool>;
    class Device;

    class ICommandBuffer
    {
        MOUCA_NOCOPY(ICommandBuffer);

        public:
            /// Destructor
            virtual ~ICommandBuffer() = default;

            //------------------------------------------------------------------------
            /// \brief  Check if command buffer is null.
            /// 
            /// \returns True if null, false otherwise.
            virtual bool isNull() const = 0;

            virtual void release(const Device& device) = 0;

            virtual void execute(const VkCommandBufferResetFlags reset) const = 0;

            virtual VkCommandBuffer getActiveCommandBuffer() const = 0;

        protected:
            void executeCommand(const ExecuteCommands& executer, const VkCommandBufferResetFlags reset) const;

            /// Constructor
            ICommandBuffer();

            CommandPoolWPtr             _pool;      ///< [LINK] Pool used to create CommandBuffer.
            VkCommandBufferUsageFlags   _usage;     ///< Usage when begin CommandBuffer.
            Commands                    _commands;  ///< [OWNERSHIP] All commands (Keep memory alive).
    };

    using ICommandBufferWPtr = std::weak_ptr<ICommandBuffer>;

    //----------------------------------------------------------------------------
    /// \brief 
    ///
    /// \code{.cpp}
    ///   CommandBuffer commandBuffer;
    ///   commandBuffer.initialize(...);
    ///   // Give command when ready
    ///   commandBuffer.registerCommand(...);
    ///   // Finalize command buffer to be use by Sequence (call many time to refresh data inside)
    ///   commandBuffer.execute();
    ///   // Release when useless
    ///   commandBuffer.release(...);
    /// \endcode
    class CommandBuffer final : public ICommandBuffer
    {
        MOUCA_NOCOPY(CommandBuffer);

        public:
            /// Constructor
            CommandBuffer();

            /// Destructor
            ~CommandBuffer() override;

            //------------------------------------------------------------------------
            /// \brief  Check if command buffer is null.
            /// 
            /// \returns True if null, false otherwise.
            bool isNull() const override
            {
                return _commandBuffer == VK_NULL_HANDLE;
            }

            void initialize(const Device& device, const CommandPoolSPtr pool, const VkCommandBufferLevel level, const VkCommandBufferUsageFlags usage);
            void release(const Device& device) override;

            void registerCommands(Commands&& commands);
            void addCommands(Commands&& commands);
            void addCommand(CommandUPtr&& command);

            void execute(const VkCommandBufferResetFlags reset = VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT) const override;

            const VkCommandBuffer& getCommandBuffer() const { return _commandBuffer; }

            const Commands& getCommands() const { return _commands; }

            VkCommandBuffer getActiveCommandBuffer() const override { return _commandBuffer; }

        private:
            VkCommandBuffer          _commandBuffer;        
    };

    using CommandBufferSPtr = std::shared_ptr<CommandBuffer>;
    using CommandBufferUPtr = std::unique_ptr<CommandBuffer>;
    using CommandBufferWPtr = std::weak_ptr<CommandBuffer>;
    using CommandBuffers    = std::vector<CommandBufferSPtr>;
}