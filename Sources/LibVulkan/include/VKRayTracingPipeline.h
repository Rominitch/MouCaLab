/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibVulkan/include/VKPipeline.h>
#include <LibVulkan/include/VKPipelineStates.h>

namespace Vulkan
{
    class Device;
    
    class BufferStrided;
    using BufferStridedUPtr = std::unique_ptr<BufferStrided>;

    class PipelineLayout;
    using PipelineLayoutWPtr = std::weak_ptr<PipelineLayout>;

    class RayTracingPipeline final : public Pipeline
    {
        MOUCA_NOCOPY_NOMOVE(RayTracingPipeline);

        public:
            RayTracingPipeline() = default;            
            ~RayTracingPipeline() override = default;   

            void addGroup(VkRayTracingShaderGroupCreateInfoKHR&& group)
            {
                _shaderGroups.emplace_back(group);
            }

            void initialize(const Device& device, PipelineLayoutWPtr layout, uint32_t maxPipelineRayRecursionDepth);
            void release(const Device& device);

            PipelineStageShaders& getShadersStage() { return _shaderStages; }

            VkRayTracingPipelineCreateInfoKHR getInfo() const;

            VkDeviceSize getGroupCount() const { return _shaderGroups.size(); }

        private:
            PipelineLayoutWPtr   _layout;
            PipelineStageShaders _shaderStages;
            std::vector<VkRayTracingShaderGroupCreateInfoKHR> _shaderGroups;
            uint32_t             _maxPipelineRayRecursionDepth = 0;
    };

    /*
    // Currently API can allocate many VkPipeline in one instruction BUT it's better APi to build RayTracingPipeline or delete them one by one !
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
            std::vector<RayTracingPipeline> _infos;
    };
    */
}