/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibVulkan/include/VKSequence.h"

#include "LibVulkan/include/VKCommandBuffer.h"
#include "LibVulkan/include/VKDevice.h"
#include "LibVulkan/include/VKFence.h"
#include "LibVulkan/include/VKSemaphore.h"
#include "LibVulkan/include/VKSubmitInfo.h"
#include "LibVulkan/include/VKSwapChain.h"

namespace Vulkan
{

SequenceAcquire::SequenceAcquire(SwapChainWPtr swapChain, SemaphoreWPtr semaphore, FenceWPtr fence, const uint64_t timeout):
_swapChain(swapChain),
_semaphore(semaphore.expired() ? VK_NULL_HANDLE : semaphore.lock()->getInstance()),
_fence(fence.expired() ? VK_NULL_HANDLE : fence.lock()->getInstance()),
_timeout(timeout)
{
    MouCa::preCondition(!_swapChain.expired()); //DEV Issue: Bad swapchain !
}

VkResult SequenceAcquire::execute(const Device& device)
{
    MouCa::preCondition(!_swapChain.expired()); //DEV Issue: Never call initialized !

    auto sw = _swapChain.lock();
    if (!sw->isReady())
        return VK_ERROR_OUT_OF_DATE_KHR;

    uint32_t currentImageIndex;
    const VkResult result = vkAcquireNextImageKHR(device.getInstance(), sw->getInstance(), _timeout, _semaphore, _fence, &currentImageIndex);
    if(result == VK_SUCCESS)
    {
        sw->setCurrentImage(currentImageIndex);
    }
    return result;
}

SequenceWaitFence::SequenceWaitFence(const std::vector<FenceWPtr>& fences, const uint64_t timeout, const VkBool32 waitAll):
_timeout(timeout), _all(waitAll)
{
    MouCa::preCondition(!fences.empty());

    _fences.reserve(fences.size());
    for (auto& fence : fences)
    {
        MouCa::assertion(!fence.lock()->isNull());
        _fences.emplace_back(fence.lock()->getInstance());
    }
}

VkResult SequenceWaitFence::execute(const Device& device)
{
    return vkWaitForFences(device.getInstance(), static_cast<uint32_t>(_fences.size()), _fences.data(), _all, _timeout);
}

SequenceResetFence::SequenceResetFence(const std::vector<FenceWPtr>& fences)
{
    MouCa::preCondition(!fences.empty());

    _fences.reserve(fences.size());
    for (auto& fence : fences)
    {
        MouCa::assertion(!fence.lock()->isNull());
        _fences.emplace_back(fence.lock()->getInstance());
    }
}

VkResult SequenceResetFence::execute(const Device& device)
{
    return vkResetFences(device.getInstance(), static_cast<uint32_t>(_fences.size()), _fences.data());
}

SequenceSubmit::SequenceSubmit(SubmitInfos&& submitInfos, FenceWPtr fence):
_fence(fence.expired() ? VK_NULL_HANDLE : fence.lock()->getInstance())
{
    MouCa::preCondition(!submitInfos.empty()); //DEV Issue: Need valid data !
    
    _submitInfos = std::move(submitInfos);
    _vkSubmitInfos.reserve(submitInfos.size());
    for(const auto& submitInfo : _submitInfos)
    {
        _vkSubmitInfos.emplace_back(submitInfo->buildSubmitInfo());
    }

    MouCa::postCondition(!_vkSubmitInfos.empty()); //DEV Issue: Not ready ?
}

VkResult SequenceSubmit::execute(const Device& device)
{
    MouCa::preCondition(!_vkSubmitInfos.empty()); //DEV Issue: nothing to submit ?

    // Update SubmitInfo (possibly based on changing SwapChain CommandBuffer)
    auto itSubmitInfo = _vkSubmitInfos.begin();
    for (const auto& submitInfo : _submitInfos)
    {
        *itSubmitInfo = submitInfo->buildSubmitInfo();
        ++itSubmitInfo;
    }
    MouCa::assertion(itSubmitInfo == _vkSubmitInfos.end());

    return vkQueueSubmit(device.getQueue(), static_cast<uint32_t>(_vkSubmitInfos.size()), _vkSubmitInfos.data(), _fence);
}

SequencePresentKHR::SequencePresentKHR(const std::vector<SemaphoreWPtr>& semaphores, const std::vector<SwapChainWPtr>& swapChains):
_swapChains(swapChains)
{
    MouCa::preCondition(!swapChains.empty());

    _semaphores.reserve(semaphores.size());
    for (const auto& semaphore : semaphores)
    {
        _semaphores.emplace_back(semaphore.lock()->getInstance());
    }

    _swapChains = swapChains;
    _swapChainIDs.resize(swapChains.size());
    _imageId.resize(swapChains.size()); // Not initialized yet
    
    // Fill data
    update();

    // Allocate result memory
    _result.resize(swapChains.size());

    _presentInfo =
    {
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,          // VkStructureType              sType
        nullptr,                                     // const void                  *pNext
        static_cast<uint32_t>(_semaphores.size()),   // uint32_t                     waitSemaphoreCount
        _semaphores.data(),                          // const VkSemaphore           *pWaitSemaphores
        static_cast<uint32_t>(_swapChainIDs.size()), // uint32_t                     swapchainCount
        _swapChainIDs.data(),                        // const VkSwapchainKHR        *pSwapchains
        _imageId.data(),                             // const uint32_t              *pImageIndices
        _result.data()                               // VkResult                    *pResults
    };
}

void SequencePresentKHR::registerSwapChain()
{
    // register
    auto sharedThis = shared_from_this();   //DEV Issue: Missing to make_shared ?
    
    for (auto& swapChain : _swapChains)
    {
        swapChain.lock()->registerSequence(sharedThis);
    }
}

SequencePresentKHR::~SequencePresentKHR()
{
    // Unregister
    try
    {
        auto sharedThis = shared_from_this();
        if (sharedThis)
        {
            for (auto& swapChain : _swapChains)
            {
                swapChain.lock()->unregisterSequence(sharedThis);
            }
        }
    }
    catch (...)
    {
    } // Disabled automatic unregister
}

VkResult SequencePresentKHR::execute(const Device& device)
{
    MouCa::preCondition(!device.isNull());
    MouCa::preCondition(device.getQueue() != VK_NULL_HANDLE);

    // Update with current ID (memory don't change so don't touch to _presentInfo)
    auto itImageID = _imageId.begin();
    for(const auto& sw : _swapChains)
    {
        const auto swapChain = sw.lock();
        if (!swapChain->isReady())
            return VK_ERROR_OUT_OF_DATE_KHR;

        *itImageID = swapChain->getCurrentImage();
        ++itImageID;
    }
    
    // Ready to queue
    return vkQueuePresentKHR(device.getQueue(), &_presentInfo);
}

void SequencePresentKHR::update()
{
    // Fill data
    auto itSwap = _swapChainIDs.begin();
    for (const auto& swapChain : _swapChains)
    {
        *itSwap = swapChain.lock()->getInstance();
        ++itSwap;
    }
    MouCa::postCondition(_swapChainIDs.end() == itSwap);
}

}