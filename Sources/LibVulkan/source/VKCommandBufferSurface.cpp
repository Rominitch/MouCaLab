/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include "LibVulkan/include/VKCommandBufferSurface.h"

#include "LibVulkan/include/VKCommand.h"
#include "LibVulkan/include/VKContextDevice.h"
#include "LibVulkan/include/VKContextWindow.h"
#include "LibVulkan/include/VKSwapChain.h"

namespace Vulkan
{

CommandBufferSurface::CommandBufferSurface()
{
    MOUCA_PRE_CONDITION(isNull());
}

CommandBufferSurface::~CommandBufferSurface()
{
    MOUCA_POST_CONDITION(isNull());
}

void CommandBufferSurface::initialize(const ContextWindowWPtr weakWindow, CommandPoolSPtr pool, const VkCommandBufferLevel level, const VkCommandBufferUsageFlags usage)
{
    MOUCA_PRE_CONDITION(isNull());                 // DEV Issue: Already build CommandBuffer ?
    MOUCA_PRE_CONDITION(!weakWindow.expired());    // DEV Issue: Need a valid surface !

    // Copy window data
    _window = weakWindow;
    auto window = weakWindow.lock();
    MOUCA_PRE_CONDITION(!window->getSwapChain().getImages().empty());

    // Copy data
    _usage = usage;
    _pool = pool;

    const size_t size = window->getSwapChain().getImages().size();
    _commandBuffers.resize(size);
    // Create command Buffer
    const VkCommandBufferAllocateInfo cmdBufferAllocateInfo
    {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, // VkStructureType              sType
        nullptr,                                        // const void*                  pNext
        pool->getInstance(),                            // VkCommandPool                commandPool
        level,                                          // VkCommandBufferLevel         level
        static_cast<uint32_t>(size)                     // uint32_t                     bufferCount
    };

    // Allocate
    if (vkAllocateCommandBuffers(window->getContextDevice().getDevice().getInstance(), &cmdBufferAllocateInfo, _commandBuffers.data()) != VK_SUCCESS)
    {
        BT_THROW_ERROR(u8"Vulkan", u8"CommandBufferCreationError");
    }

    MOUCA_POST_CONDITION(!isNull());    //DEV Issue: Not initialize ?
}

void CommandBufferSurface::release(const Device& device)
{
    MOUCA_PRE_CONDITION(!isNull());        // DEV Issue: Never initialize ?
    MOUCA_PRE_CONDITION(!_pool.expired()); //DEV Issue: Pool was delete before command.

    // Release CommandBuffers
    vkFreeCommandBuffers(device.getInstance(), _pool.lock()->getInstance(), static_cast<uint32_t>(_commandBuffers.size()), _commandBuffers.data());
    _commandBuffers.clear();
    
    // Remove memory about commands
    _commands.clear();
    _pool.reset();

    MOUCA_POST_CONDITION(isNull());       // DEV Issue: Something wrong ?
}

bool CommandBufferSurface::isNull() const
{
    return _commandBuffers.empty();
}

void CommandBufferSurface::registerCommands(Commands&& commands)
{
    // Register
    _commands = std::move(commands);
}

void CommandBufferSurface::execute(const VkCommandBufferResetFlags reset) const
{
    uint32_t idFrameBuffer = 0;
    for(const auto& commandBuffer : _commandBuffers)
    {
        executeCommand({ commandBuffer, idFrameBuffer }, reset);
        ++idFrameBuffer;
    }
}

VkCommandBuffer CommandBufferSurface::getActiveCommandBuffer() const
{
    MOUCA_PRE_CONDITION(!isNull());            // DEV Issue: Never initialize ?
    MOUCA_PRE_CONDITION(!_window.expired());   // DEV Issue: No window ?

    const auto idBuffer = _window.lock()->getSwapChain().getCurrentImage();
    BT_ASSERT(idBuffer < _commandBuffers.size());
    return _commandBuffers[idBuffer];
}

}