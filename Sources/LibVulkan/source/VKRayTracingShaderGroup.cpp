/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include <LibVulkan/include/VKBufferStrided.h>
#include "LibVulkan/include/VKContextDevice.h"
#include "LibVulkan/include/VKGraphicsPipeline.h"
#include "LibVulkan/include/VKRayTracingShaderGroup.h"

namespace Vulkan
{

void RayTracingShaderGroup::initialize(const ContextDevice& context, const GraphicsPipeline& pipeline, std::vector<VkRayTracingShaderGroupCreateInfoKHR>&& shaderGroups, const uint32_t firstGroup, const std::vector<VkDeviceSize>& bindings)
{
    MOUCA_PRE_CONDITION(isNull());
    const auto& device = context.getDevice();
    const auto& properties = device.getPhysicalDeviceRayTracingPipelineProperties();

    _shaderGroups = std::move(shaderGroups);

    const uint32_t groupCount = static_cast<uint32_t>(_shaderGroups.size());
    const VkDeviceSize handleSize        = static_cast<VkDeviceSize>(properties.shaderGroupHandleSize);
    const VkDeviceSize handleSizeAligned = Buffer::alignedSize(properties.shaderGroupHandleSize, properties.shaderGroupHandleAlignment);
    const VkDeviceSize sbtSize           = groupCount * handleSizeAligned;

    _shaderHandleStorage.resize(sbtSize);
    if(vkGetRayTracingShaderGroupHandlesKHR(device.getInstance(), pipeline.getInstance(), firstGroup, groupCount, 
        static_cast<uint32_t>(_shaderHandleStorage.size()), _shaderHandleStorage.data()) != VK_SUCCESS)
    {
        MOUCA_THROW_ERROR(u8"Vulkan", u8"ShaderGroupHandlesError");
    }
    
    auto pointer = _shaderHandleStorage.data();
    for(const auto& size : bindings)
    {
        auto buffer = std::make_unique<BufferStrided>(std::make_unique<MemoryBuffer>(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
        buffer->initialize(device, 0, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                          size, properties.shaderGroupHandleSize, properties.shaderGroupHandleAlignment);

        Memory& memory = buffer->getMemory();
        memory.map(device);

        memcpy(memory.getMappedMemory<uint8_t>(), pointer, handleSize * size);
        pointer += handleSizeAligned * size;

        _bindings.emplace_back(std::move(buffer));
    }
    
    MOUCA_POST_CONDITION(!isNull());
}

}