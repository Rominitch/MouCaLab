/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibVulkan/include/VKSubmitInfo.h"

#include "LibVulkan/include/VKCommandBuffer.h"
#include "LibVulkan/include/VKSemaphore.h"
#include "LibVulkan/include/VKSwapChain.h"

namespace Vulkan
{

void SubmitInfo::addSynchronization(const std::vector<WaitSemaphore>& waitSemaphores, const std::vector<SemaphoreWPtr>& signalSemaphores)
{
    MOUCA_PRE_CONDITION(_waitSemaphore.empty() && _waitStageFlag.empty()); //DEV Issue: Already call initialize !

    // Build array (keep memory alive)
    _waitSemaphore.reserve(waitSemaphores.size());
    _waitStageFlag.reserve(waitSemaphores.size());
    for (const auto& waitSemaphore : waitSemaphores)
    {
        MOUCA_ASSERT(!waitSemaphore.first.lock()->isNull()); //DEV Issue: Bad semaphore: plesae call intialize() !
        _waitSemaphore.emplace_back(waitSemaphore.first.lock()->getInstance());
        _waitStageFlag.emplace_back(waitSemaphore.second);
    }
    _signalSemaphore.reserve(signalSemaphores.size());
    for (const auto& signalSemaphore : signalSemaphores)
    {
        MOUCA_ASSERT(!signalSemaphore.lock()->isNull()); //DEV Issue: Bad semaphore: plesae call intialize() !
        _signalSemaphore.emplace_back(signalSemaphore.lock()->getInstance());
    }
}

void SubmitInfo::initialize(std::vector<ICommandBufferWPtr>&& commandBuffers)
{
    MOUCA_PRE_CONDITION(_commands.empty());        //DEV Issue: Already call initialize !
    MOUCA_PRE_CONDITION(!commandBuffers.empty());  //DEV Issue : Need valid data

    _commandBuffers = std::move(commandBuffers);

    // Allocate array for VkSubmitInfo
    _commands.resize(_commandBuffers.size());
}

VkSubmitInfo SubmitInfo::buildSubmitInfo()
{
    MOUCA_PRE_CONDITION(!_commandBuffers.empty()); //DEV Issue: Missing call initialize !

    // Compute current state of CommandBuffer
    auto itCommand = _commands.begin();
    for(const auto& cs : _commandBuffers)
    {
        *itCommand = cs.lock()->getActiveCommandBuffer();
        ++itCommand;
    }

    return
    {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,                  // VkStructureType              sType
        nullptr,                                        // const void                  *pNext
        static_cast<uint32_t>(_waitSemaphore.size()),   // uint32_t                     waitSemaphoreCount
        _waitSemaphore.data(),                          // const VkSemaphore           *pWaitSemaphores
        _waitStageFlag.data(),                          // const VkPipelineStageFlags  *pWaitDstStageMask
        static_cast<uint32_t>(_commands.size()),        // uint32_t                     commandBufferCount
        _commands.data(),                               // const VkCommandBuffer       *pCommandBuffers
        static_cast<uint32_t>(_signalSemaphore.size()), // uint32_t                     signalSemaphoreCount
        _signalSemaphore.data()                         // const VkSemaphore           *pSignalSemaphores
    };
}

}