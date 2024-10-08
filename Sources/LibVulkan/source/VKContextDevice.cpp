/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibVulkan/include/VKContextDevice.h"
#include "LibVulkan/include/VKCommandBuffer.h"
#include "LibVulkan/include/VKDescriptorSet.h"
#include "LibVulkan/include/VKEnvironment.h"
#include "LibVulkan/include/VKFence.h"
#include "LibVulkan/include/VKFrameBuffer.h"
#include "LibVulkan/include/VKGraphicsPipeline.h"
#include "LibVulkan/include/VKImage.h"
#include "LibVulkan/include/VKPipelineLayout.h"
#include "LibVulkan/include/VKRayTracingPipeline.h"
#include "LibVulkan/include/VKRenderPass.h"
#include "LibVulkan/include/VKSampler.h"
#include "LibVulkan/include/VKSequence.h"
#include "LibVulkan/include/VKSemaphore.h"
#include "LibVulkan/include/VKShaderProgram.h"

namespace Vulkan
{

ContextDevice::~ContextDevice()
{
    MouCa::postCondition(isNull());        //Dev Issue: Need to call release
}

void ContextDevice::initialize(const Vulkan::Environment& environment, const std::vector<const char*>& deviceExtensions, const PhysicalDeviceFeatures& enabled, const Vulkan::Surface* surface)
{
    MouCa::preCondition(isNull());                 //DEV Issue: No re-entrance: call release before initialize again.
    MouCa::preCondition(!environment.isNull());    //DEV Issue:  Environment is not ready.

    // Create best device for rendering and adapted to surface.
    _device.initializeBestGPU(environment, deviceExtensions, surface, enabled);

    // Create Pipeline Cache
    _pipelineCache.initialize(_device);

    MouCa::preCondition(!isNull()); //DEV Issue: Something wrong ?
}

void ContextDevice::release()
{
    MouCa::preCondition(!isNull()); //DEV Issue: initialize() must be call before !

    // WARNING: Remove in invert order to declaration to avoid used memory !

    _sequences.clear();

    for (auto& commandBuffer : _commandBuffers)
    {
        commandBuffer->release(_device);
    }
    _commandBuffers.clear();

    for(auto& commandPool : _commandPools)
    {
        commandPool->release(_device);
    }
    _commandPools.clear();

    for (auto& graphicsPipeline : _graphicsPipelines)
    {
        graphicsPipeline->release(_device);
    }
    _graphicsPipelines.clear();
    
    for (auto& pipelineLayout : _pipelineLayouts)
    {
        pipelineLayout->release(_device);
    }
    _pipelineLayouts.clear();

    for (auto& descriptorSetLayout : _descriptorSetLayouts)
    {
        descriptorSetLayout->release(_device);
    }
    _descriptorSetLayouts.clear();

    for (auto& descriptorSet : _descriptorSets)
    {
        descriptorSet->release(_device);
    }
    _descriptorSets.clear();

    for (auto& descriptorPool : _descriptorPools)
    {
        descriptorPool->release(_device);
    }
    _descriptorPools.clear();    

    for (auto& renderPass : _renderPasses)
    {
        renderPass->release(_device);
    }
    _renderPasses.clear();

    for (auto& frameBuffer : _frameBuffers)
    {
        frameBuffer->release(_device);
    }
    _frameBuffers.clear();

    for(auto& image : _images)
    {
        image->release(_device);
    }
    _images.clear();

    for (auto& sampler : _samplers)
    {
        sampler->release(_device);
    }
    _samplers.clear();

    for (auto& buffer : _buffers)
    {
        buffer->release(_device);
    }
    _buffers.clear(); 

    for(auto& semaphore : _semaphores)
    {
        semaphore->release(_device);
    }
    _semaphores.clear();

    for (auto& fence : _fences)
    {
        fence->release(_device);
    }
    _fences.clear();

    for (auto& shader : _shaderModules)
    {
        shader->release(_device);
    }
    _shaderModules.clear();

    for (auto& rayTracingPipeline : _rayTracingPipelines)
    {
        rayTracingPipeline->release(_device);
    }
    _rayTracingPipelines.clear();

    // Release all item which use device
    _pipelineCache.release(_device);

    // Finally Release device
    _device.release();

    MouCa::preCondition(isNull()); //DEV Issue: Something wrong ?
}

void ContextDevice::removeImage(ImageWPtr data)
{
    MouCa::preCondition(!data.expired()); //DEV Issue: Need valid data
    auto itErase = std::find_if(_images.begin(), _images.end(), [&](const auto& image) { return image.get() == data.lock().get(); });
    MouCa::assertion(itErase != _images.end());
    _images.erase(itErase);
}

void ContextDevice::removeSemaphore(SemaphoreWPtr semaphore)
{
    MouCa::preCondition(!semaphore.expired()); //DEV Issue: Need valid data
    auto itErase = std::find_if(_semaphores.begin(), _semaphores.end(), [&](const auto& s) { return s.get() == semaphore.lock().get(); });
    MouCa::assertion(itErase != _semaphores.end());
    _semaphores.erase(itErase);
}

void ContextDevice::removeFrameBuffer(FrameBufferWPtr framebuffer)
{
    MouCa::preCondition(!framebuffer.expired()); //DEV Issue: Need valid data
    auto itErase = std::find_if(_frameBuffers.begin(), _frameBuffers.end(), [&](const auto& s) { return s.get() == framebuffer.lock().get(); });
    MouCa::assertion(itErase != _frameBuffers.end());
    _frameBuffers.erase(itErase);
}

void ContextDevice::removeRenderPass(RenderPassWPtr renderpass)
{
    MouCa::preCondition(!renderpass.expired()); //DEV Issue: Need valid data
    auto itErase = std::find_if(_renderPasses.begin(), _renderPasses.end(), [&](const auto& s) { return s.get() == renderpass.lock().get(); });
    MouCa::assertion(itErase != _renderPasses.end());
    _renderPasses.erase(itErase);
}

void ContextDevice::removeFence(FenceWPtr fence)
{
    MouCa::preCondition(!fence.expired()); //DEV Issue: Need valid data
    auto itErase = std::find_if(_fences.begin(), _fences.end(), [&](const auto& s) { return s.get() == fence.lock().get(); });
    MouCa::assertion(itErase != _fences.end());
    _fences.erase(itErase);
}

}