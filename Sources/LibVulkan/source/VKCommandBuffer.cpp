/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibVulkan/include/VKCommand.h"
#include "LibVulkan/include/VKCommandBuffer.h"
#include "LibVulkan/include/VKCommandPool.h"
#include "LibVulkan/include/VKDescriptorSet.h"
#include "LibVulkan/include/VKDevice.h"
#include "LibVulkan/include/VKFrameBuffer.h"
#include "LibVulkan/include/VKMesh.h"
#include "LibVulkan/include/VKGraphicsPipeline.h"
#include "LibVulkan/include/VKRenderPass.h"
#include "LibVulkan/include/VKSurfaceFormat.h"
#include "LibVulkan/include/VKSwapChain.h"

namespace Vulkan
{

ICommandBuffer::ICommandBuffer():
_usage(0)
{}

void ICommandBuffer::executeCommand(const ExecuteCommands& executer, const VkCommandBufferResetFlags reset) const
{
    MouCa::preCondition(!isNull());            //DEV Issue: missing to call initialize;
    MouCa::preCondition(!_commands.empty());   //DEV Issue: missing to call registerCommands;

    // Reset
    if (vkResetCommandBuffer(executer.commandBuffer, reset) != VK_SUCCESS)
    {
        throw Core::Exception(Core::ErrorData("Vulkan", "CommandBufferResetError"));
    }

    const VkCommandBufferBeginInfo cmdBufferBeginInfo
    {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,  // VkStructureType                        sType
        nullptr,                                      // const void                            *pNext
        _usage,                                       // VkCommandBufferUsageFlags              flags
        nullptr                                       // const VkCommandBufferInheritanceInfo  *pInheritanceInfo
    };

    // Begin 
    if (vkBeginCommandBuffer(executer.commandBuffer, &cmdBufferBeginInfo) != VK_SUCCESS)
    {
        throw Core::Exception(Core::ErrorData("Vulkan", "CommandBufferBeginError"));
    }

    // Execute all commands
    for (const auto& command : _commands)
    {
        command->execute(executer);
    }

    // End
    if (vkEndCommandBuffer(executer.commandBuffer) != VK_SUCCESS)
    {
        throw Core::Exception(Core::ErrorData("Vulkan", "CommandBufferEndError"));
    }
}

CommandBuffer::CommandBuffer():
_commandBuffer(VK_NULL_HANDLE)
{
    MouCa::assertion(isNull());
}

CommandBuffer::~CommandBuffer()
{
    MouCa::assertion(isNull());
}

void CommandBuffer::initialize(const Device& device, const CommandPoolSPtr pool, const VkCommandBufferLevel level, const VkCommandBufferUsageFlags usage)
{
    MouCa::preCondition(isNull());         //DEV Issue: Already initialize ?
    MouCa::preCondition(!device.isNull()); //DEV Issue: Bad device ?
    MouCa::preCondition(!pool->isNull());  //DEV Issue: Bad pool ?

    // Copy data
    _usage = usage;
    _pool = pool;

    // Create command Buffer
    const VkCommandBufferAllocateInfo cmdBufferAllocateInfo
    {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, // VkStructureType              sType
        nullptr,                                        // const void*                  pNext
        pool->getInstance(),                            // VkCommandPool                commandPool
        level,                                          // VkCommandBufferLevel         level
        1                                               // uint32_t                     bufferCount
    };

    // Allocate
    if (vkAllocateCommandBuffers(device.getInstance(), &cmdBufferAllocateInfo, &_commandBuffer) != VK_SUCCESS)
    {
        throw Core::Exception(Core::ErrorData("Vulkan", "CommandBufferCreationError"));
    }

    MouCa::postCondition(!isNull());    //DEV Issue: Not initialize ?
}

void CommandBuffer::release(const Device& device)
{
    MouCa::preCondition(!isNull());        //DEV Issue: Not initialize ?
    MouCa::preCondition(!_pool.expired()); //DEV Issue: Pool was delete before command.

    // Delete command buffer
    vkFreeCommandBuffers(device.getInstance(), _pool.lock()->getInstance(), 1, &_commandBuffer);

    // Remove memory about commands
    _commands.clear();
    _commandBuffer = VK_NULL_HANDLE;

    _pool.reset();

    MouCa::postCondition(isNull());    //DEV Issue: Still alive ?
}

void CommandBuffer::registerCommands(Commands&& commands)
{
    MouCa::preCondition(!commands.empty());    //DEV Issue: insert no command ?
    _commands = std::move(commands);
}

void CommandBuffer::addCommands(Commands&& commands)
{
    MouCa::preCondition(!commands.empty());    //DEV Issue: insert no command ?

    _commands.insert(_commands.end(), std::make_move_iterator(commands.begin()), std::make_move_iterator(commands.end()));
}

void CommandBuffer::addCommand(CommandUPtr&& command)
{
    MouCa::preCondition(command != nullptr);    //DEV Issue: insert no command ?

    _commands.emplace_back(std::move(command));
}

void CommandBuffer::execute(const VkCommandBufferResetFlags reset) const
{
    executeCommand({ _commandBuffer, 0 }, reset);
}

}