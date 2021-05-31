/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibVulkan/include/VKPipeline.h>
#include <LibVulkan/include/VKPipelineCache.h>
#include <LibVulkan/include/VKPipelineStates.h>

namespace Vulkan
{
    class Device;
    class RenderPass;
    using RenderPassWPtr = std::weak_ptr<RenderPass>;

    class PipelineCache;
    using PipelineCacheWPtr = std::weak_ptr<PipelineCache>;

    class PipelineLayout;
    using PipelineLayoutWPtr = std::weak_ptr<PipelineLayout>;

    class PipelineStateCreateInfo;

    //----------------------------------------------------------------------------
    /// \brief 
    ///
    class GraphicsPipeline final : public Pipeline
    {
        MOUCA_NOCOPY(GraphicsPipeline);

        public:
            GraphicsPipeline();
            ~GraphicsPipeline() override;

            [[deprecated]] void initialize(const Device& device, const RenderPass& renderPass, const PipelineLayout& layout, const PipelineCache& pipelineCache = PipelineCache());

            void initialize(const Device& device, RenderPassWPtr renderPass, PipelineLayoutWPtr layout, PipelineCacheWPtr pipelineCache);
            void release(const Device& device);

            PipelineStateCreateInfo&        getInfo()       { return _infos; }
            const PipelineStateCreateInfo&  getInfo() const { return _infos; }

            RenderPassWPtr       getRenderPass() const      { return _renderPass; }
            PipelineLayoutWPtr   getPipelineLayout() const  { return _layout; }
            PipelineCacheWPtr    getPipelineCache() const   { return _pipelineCache; }

        private:
            RenderPassWPtr          _renderPass;
            PipelineLayoutWPtr      _layout;
            PipelineCacheWPtr       _pipelineCache;
            PipelineStateCreateInfo _infos;
    };
}