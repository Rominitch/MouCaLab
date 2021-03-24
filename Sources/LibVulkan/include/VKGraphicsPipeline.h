/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

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
    class GraphicsPipeline final
    {
        MOUCA_NOCOPY(GraphicsPipeline);

        public:
            GraphicsPipeline();
            ~GraphicsPipeline();

            [[deprecated]] void initialize(const Device& device, const RenderPass& renderPass, const PipelineLayout& layout, const PipelineCache& pipelineCache = PipelineCache());

            void initialize(const Device& device, RenderPassWPtr renderPass, PipelineLayoutWPtr layout, PipelineCacheWPtr pipelineCache);
            void release(const Device& device);

            PipelineStateCreateInfo&        getInfo()       { return _infos; }
            const PipelineStateCreateInfo&  getInfo() const { return _infos; }

            bool isNull() const
            {
                return _pipeline == VK_NULL_HANDLE;
            }

            const VkPipeline& getInstance() const
            {
                return _pipeline;
            }

            RenderPassWPtr       getRenderPass() const      { return _renderPass; }
            PipelineLayoutWPtr   getPipelineLayout() const  { return _layout; }
            PipelineCacheWPtr    getPipelineCache() const   { return _pipelineCache; }

        private:
            VkPipeline              _pipeline;

            RenderPassWPtr          _renderPass;
            PipelineLayoutWPtr      _layout;
            PipelineCacheWPtr       _pipelineCache;
            PipelineStateCreateInfo _infos;
    };
}