/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibVulkan/include/VKPipelineStates.h>

namespace Vulkan
{
    class Device;
    
    class PipelineLayout;
    using PipelineLayoutWPtr = std::weak_ptr<PipelineLayout>;

    class RayTracingShaderGroup;

    class RayTracingPipeline
    {
        //MOUCA_NOCOPY(RayTracingPipelineInfo);
        public:

            void addGroup(VkRayTracingShaderGroupCreateInfoKHR&& group)
            {
                _shaderGroups.emplace_back(group);
            }

            void initialize(PipelineLayoutWPtr layout, uint32_t maxPipelineRayRecursionDepth);

            PipelineStageShaders& getShadersStage() { return _shaderStages; }

            VkRayTracingPipelineCreateInfoKHR getInfo() const;

        private:
            PipelineLayoutWPtr   _layout;
            uint32_t             _maxPipelineRayRecursionDepth = 0;
            PipelineStageShaders _shaderStages;
            std::vector<VkRayTracingShaderGroupCreateInfoKHR> _shaderGroups;
    };

    class RayTracingPipelines
    {
        MOUCA_NOCOPY_NOMOVE(RayTracingPipelines);
        public:
            RayTracingPipelines() = default;

            void addPipeline(RayTracingPipeline&& info);

            void initialize(const Device& context);

            void release(const Device& context);

            bool isNull() const { return _pipelines.empty();  }

        private:
            std::vector<VkPipeline>         _pipelines;
            std::vector<RayTracingPipeline> _infos;
    };

}