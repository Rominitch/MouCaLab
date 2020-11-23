/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

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
    BT_PRE_CONDITION(!isNull());            //DEV Issue: missing to call initialize;
    BT_PRE_CONDITION(!_commands.empty());   //DEV Issue: missing to call registerCommands;

    // Reset
    if (vkResetCommandBuffer(executer.commandBuffer, reset) != VK_SUCCESS)
    {
        BT_THROW_ERROR(u8"Vulkan", u8"CommandBufferResetError");
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
        BT_THROW_ERROR(u8"Vulkan", u8"CommandBufferBeginError");
    }

    // Execute all commands
    for (const auto& command : _commands)
    {
        command->execute(executer);
    }

    // End
    if (vkEndCommandBuffer(executer.commandBuffer) != VK_SUCCESS)
    {
        BT_THROW_ERROR(u8"Vulkan", u8"CommandBufferEndError");
    }
}

CommandBuffer::CommandBuffer():
_commandBuffer(VK_NULL_HANDLE)
{
    BT_ASSERT(isNull());
}

CommandBuffer::~CommandBuffer()
{
    BT_ASSERT(isNull());
}

void CommandBuffer::initialize(const Device& device, const CommandPoolSPtr pool, const VkCommandBufferLevel level, const VkCommandBufferUsageFlags usage)
{
    BT_PRE_CONDITION(isNull());         //DEV Issue: Already initialize ?
    BT_PRE_CONDITION(!device.isNull()); //DEV Issue: Bad device ?
    BT_PRE_CONDITION(!pool->isNull());  //DEV Issue: Bad pool ?

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
        BT_THROW_ERROR(u8"Vulkan", u8"CommandBufferCreationError");
    }

    BT_POST_CONDITION(!isNull());    //DEV Issue: Not initialize ?
}

void CommandBuffer::release(const Device& device)
{
    BT_PRE_CONDITION(!isNull());        //DEV Issue: Not initialize ?
    BT_PRE_CONDITION(!_pool.expired()); //DEV Issue: Pool was delete before command.

    // Delete command buffer
    vkFreeCommandBuffers(device.getInstance(), _pool.lock()->getInstance(), 1, &_commandBuffer);

    // Remove memory about commands
    _commands.clear();
    _commandBuffer = VK_NULL_HANDLE;

    _pool.reset();

    BT_POST_CONDITION(isNull());    //DEV Issue: Still alive ?
}

void CommandBuffer::registerCommands(Commands&& commands)
{
    BT_PRE_CONDITION(!commands.empty());    //DEV Issue: insert no command ?
    _commands = std::move(commands);
}

void CommandBuffer::addCommands(Commands&& commands)
{
    BT_PRE_CONDITION(!commands.empty());    //DEV Issue: insert no command ?

    _commands.insert(_commands.end(), std::make_move_iterator(commands.begin()), std::make_move_iterator(commands.end()));
}

void CommandBuffer::addCommand(CommandUPtr&& command)
{
    BT_PRE_CONDITION(command != nullptr);    //DEV Issue: insert no command ?

    _commands.emplace_back(std::move(command));
}

void CommandBuffer::execute(const VkCommandBufferResetFlags reset) const
{
    executeCommand({ _commandBuffer, 0 }, reset);
}

}