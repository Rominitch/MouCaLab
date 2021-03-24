/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace Vulkan
{
    class BufferStrided;
    using BufferStridedUPtr = std::unique_ptr<BufferStrided>;

    class ContextDevice;

    class RayTracingShaderGroup
    {
        MOUCA_NOCOPY_NOMOVE(RayTracingShaderGroup);

        public:
            void initialize(const ContextDevice& context, const GraphicsPipeline& pipeline, std::vector<VkRayTracingShaderGroupCreateInfoKHR>&& shaderGroups, const uint32_t firstGroup,
                const std::vector<VkDeviceSize>& bindings);

            bool isNull() const
            {
                return _shaderGroups.empty();
            }

        private:
            std::vector<VkRayTracingShaderGroupCreateInfoKHR> _shaderGroups;
            std::vector<BufferStridedUPtr>                    _bindings;
            std::vector<uint8_t>                              _shaderHandleStorage;
    };
}
