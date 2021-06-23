/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibVulkan/include/VKBufferStrided.h>

namespace Vulkan
{
    class Device;

    class RayTracingPipeline;
    using RayTracingPipelineWPtr = std::weak_ptr<RayTracingPipeline>;

    class TracingRay final
    {
        MOUCA_NOCOPY_NOMOVE(TracingRay);

        public:
            static const size_t _nbInputs = 4;
            using BufferSizes = std::array<VkDeviceSize, _nbInputs>;

            TracingRay();
            void initialize(const Device& device, const RayTracingPipelineWPtr pipeline, const uint32_t firstGroup,
                            const BufferSizes& bindings);

            void release(const Device& device);

            bool isNull() const { return _shaderHandleStorage.empty(); }

            const BufferStrided& getBufferStrided(uint32_t id) const
            {
                MOUCA_PRE_CONDITION(id < _bindings.size());
                return _bindings[id];
            }

        private:
            std::array<BufferStrided, _nbInputs>  _bindings;
            std::vector<uint8_t>                  _shaderHandleStorage;
    };
}
