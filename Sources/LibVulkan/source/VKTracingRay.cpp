/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibVulkan/include/VKTracingRay.h"

#include "LibVulkan/include/VKBufferStrided.h"
#include "LibVulkan/include/VKDevice.h"
#include "LibVulkan/include/VKRayTracingPipeline.h"

namespace Vulkan
{
TracingRay::TracingRay():
_bindings
    { {
        BufferStrided(std::make_unique<MemoryBufferAllocate>(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR)),
        BufferStrided(std::make_unique<MemoryBufferAllocate>(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR)),
        BufferStrided(std::make_unique<MemoryBufferAllocate>(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR)),
        BufferStrided(std::make_unique<MemoryBufferAllocate>(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR))
    } }
{}

void TracingRay::initialize(const Device& device, const RayTracingPipelineWPtr pipelineW, 
                            const uint32_t firstGroup, const std::array<VkDeviceSize, _nbInputs>& bindings)
{
    MouCa::preCondition(!device.isNull());
    MouCa::preCondition(isNull());

    const auto& properties = device.getPhysicalDeviceRayTracingPipelineProperties();
    const auto& pipeline = pipelineW.lock();

    const VkDeviceSize groupCount        = pipeline->getGroupCount();
    const VkDeviceSize handleSize        = static_cast<VkDeviceSize>(properties.shaderGroupHandleSize);
    const VkDeviceSize handleSizeAligned = Buffer::alignedSize(properties.shaderGroupHandleSize, properties.shaderGroupHandleAlignment);
    const VkDeviceSize sbtSize           = groupCount * handleSizeAligned;

    _shaderHandleStorage.resize(sbtSize);
    if(device.vkGetRayTracingShaderGroupHandlesKHR(device.getInstance(), pipeline->getInstance(), firstGroup, static_cast<uint32_t>(groupCount), 
        static_cast<uint32_t>(_shaderHandleStorage.size()), _shaderHandleStorage.data()) != VK_SUCCESS)
    {
        throw Core::Exception(Core::ErrorData("Vulkan", "ShaderGroupHandlesError"));
    }
    
    auto pointer = _shaderHandleStorage.data();
    auto itBinding = _bindings.begin();
    
    for(const auto& size : bindings)
    {
        if(size > 0)
        {
            itBinding->initialize(device, 0, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                               size, properties.shaderGroupHandleSize, properties.shaderGroupHandleAlignment);

            Memory& memory = itBinding->getMemory();
            memory.map(device);

            memcpy(memory.getMappedMemory<uint8_t>(), pointer, handleSize * size);
            pointer += handleSizeAligned * size;
        }
        ++itBinding;
    }
    
    MouCa::postCondition(!isNull());
}

void TracingRay::release(const Device& device)
{
    MouCa::preCondition(!device.isNull());
    MouCa::preCondition(!isNull());

    // Clean buffers
    for(auto& bindings : _bindings)
    {
        if(!bindings.isNull())
            bindings.release(device);
    }
    _shaderHandleStorage.clear();
    
    MouCa::postCondition(isNull());
}

}