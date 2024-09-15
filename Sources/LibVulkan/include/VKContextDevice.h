/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibVulkan/include/VKCommandPool.h>
#include <LibVulkan/include/VKContextWindow.h>
#include <LibVulkan/include/VKDevice.h>
#include <LibVulkan/include/VKPipelineCache.h>

namespace Vulkan
{
    class AccelerationStructure;
    using AccelerationStructureSPtr = std::shared_ptr<AccelerationStructure>;
    using AccelerationStructureWPtr = std::weak_ptr<AccelerationStructure>;
    using AccelerationStructures    = std::vector<AccelerationStructureSPtr>;

    class Buffer;
    using BufferSPtr = std::shared_ptr<Buffer>;
    using BufferWPtr = std::weak_ptr<Buffer>;
    using Buffers    = std::vector<BufferSPtr>;

    class CommandBuffer;
    using CommandBufferSPtr = std::shared_ptr<CommandBuffer>;
    using CommandBuffers    = std::vector<CommandBufferSPtr>;

    class CommandPool;
    using CommandPoolSPtr = std::shared_ptr<CommandPool>;
    using CommandPools    = std::vector<CommandPoolSPtr>;

    class DescriptorPool;
    using DescriptorPoolSPtr = std::shared_ptr<DescriptorPool>;
    using DescriptorPoolWPtr = std::weak_ptr<DescriptorPool>;
    using DescriptorPools    = std::vector<DescriptorPoolSPtr>;

    class DescriptorSet;
    using DescriptorSetSPtr = std::shared_ptr<DescriptorSet>;
    using DescriptorSetWPtr = std::weak_ptr<DescriptorSet>;
    using DescriptorSets    = std::vector<DescriptorSetSPtr>; 

    class DescriptorSetLayout;
    using DescriptorSetLayoutSPtr = std::shared_ptr<DescriptorSetLayout>;
    using DescriptorSetLayoutWPtr = std::weak_ptr<DescriptorSetLayout>;
    using DescriptorSetLayouts    = std::vector<DescriptorSetLayoutSPtr>;

    class Fence;
    using FenceSPtr = std::shared_ptr<Fence>;
    using FenceWPtr = std::weak_ptr<Fence>;
    using Fences    = std::vector<FenceSPtr>;

    class FrameBuffer;
    using FrameBufferSPtr = std::shared_ptr<FrameBuffer>;
    using FrameBufferWPtr = std::weak_ptr<FrameBuffer>;
    using FrameBuffers    = std::vector<FrameBufferSPtr>;

    class Image;
    using ImageSPtr = std::shared_ptr<Image>;
    using ImageWPtr = std::weak_ptr<Image>;
    using Images    = std::vector<ImageSPtr>;

    class GraphicsPipeline;
    using GraphicsPipelineSPtr = std::shared_ptr<GraphicsPipeline>;
    using GraphicsPipelineWPtr = std::weak_ptr<GraphicsPipeline>;
    using GraphicsPipelines    = std::vector<GraphicsPipelineSPtr>;

    struct PhysicalDeviceFeatures;

    class PipelineLayout;
    using PipelineLayoutSPtr = std::shared_ptr<PipelineLayout>;
    using PipelineLayoutWPtr = std::weak_ptr<PipelineLayout>;
    using PipelineLayouts    = std::vector<PipelineLayoutSPtr>;

    class RayTracingPipeline;
    using RayTracingPipelineSPtr = std::shared_ptr<RayTracingPipeline>;
    using RayTracingPipelineWPtr = std::weak_ptr<RayTracingPipeline>;
    using RayTracingPipelines    = std::vector<RayTracingPipelineSPtr>;

    class RenderPass;
    using RenderPassSPtr = std::shared_ptr<RenderPass>;
    using RenderPassWPtr = std::weak_ptr<RenderPass>;
    using RenderPasses   = std::vector<RenderPassSPtr>;

    class Sampler;
    using SamplerSPtr = std::shared_ptr<Sampler>;
    using SamplerWPtr = std::weak_ptr<Sampler>;
    using Samplers    = std::vector<SamplerSPtr>;

    class Sequence;
    using SequenceSPtr = std::shared_ptr<Sequence>;

    using QueueSequence     = std::vector<SequenceSPtr>;
    using QueueSequenceSPtr = std::shared_ptr<QueueSequence>;
    using QueueSequences    = std::vector<QueueSequenceSPtr>;

    class Semaphore;
    using SemaphoreSPtr = std::shared_ptr<Semaphore>;
    using SemaphoreWPtr = std::weak_ptr<Semaphore>;
    using Semaphores    = std::vector<SemaphoreSPtr>;

    class ShaderModule;
    using ShaderModuleSPtr = std::shared_ptr<ShaderModule>;
    using ShaderModuleWPtr = std::weak_ptr<ShaderModule>;
    using ShaderModules    = std::vector<ShaderModuleSPtr>;

    class TracingRay;
    using TracingRaySPtr = std::shared_ptr<TracingRay>;
    using TracingRayWPtr = std::weak_ptr<TracingRay>;
    using TracingRays    = std::vector<TracingRaySPtr>;
    

    //----------------------------------------------------------------------------
    /// \brief Manage a Vulkan device: create all objects which need a device to live.
    class ContextDevice final : public std::enable_shared_from_this<ContextDevice>
    {
        MOUCA_NOCOPY(ContextDevice);

        public:
            /// Constructor
            ContextDevice() = default;

            /// Destructor
            ~ContextDevice();

            void initialize(const Environment& environment, const std::vector<const char*>& extensions, const PhysicalDeviceFeatures& enabled, const Surface* surface = nullptr);
            
            void release();

            //------------------------------------------------------------------------
            /// \brief  Check if current device is not initialized.
            /// 
            /// \returns True if each component is null, otherwise false.
            bool isNull() const
            {
                return _device.isNull() && _pipelineCache.isNull();
            }

            const QueueSequences&   getQueueSequences() const   { return _sequences; }

            //------------------------------------------------------------------------
            /// \brief  Return current device.
            const Device& getDevice() const { return _device; }

            void insertImage(ImageSPtr data)
            {
                MouCa::preCondition(data.get()); //DEV Issue: Need valid data
                _images.emplace_back(data);
            }

            void removeImage(ImageWPtr data);

            void insertBuffer(BufferSPtr data)
            {
                MouCa::preCondition(data.get()); //DEV Issue: Need valid data
                _buffers.emplace_back(data);
            }

            void insertSemaphore(SemaphoreSPtr semaphore)
            {
                MouCa::preCondition(semaphore.get()); //DEV Issue: Need valid data
                _semaphores.emplace_back(semaphore);
            }

            void removeSemaphore(SemaphoreWPtr semaphore);

            void insertFrameBuffer(FrameBufferSPtr framebuffer)
            {
                MouCa::preCondition(framebuffer.get()); //DEV Issue: Need valid data
                _frameBuffers.emplace_back(framebuffer);
            }

            void removeFrameBuffer(FrameBufferWPtr framebuffer);

            void insertRenderPass(RenderPassSPtr renderpass)
            {
                MouCa::preCondition(renderpass.get()); //DEV Issue: Need valid data
                _renderPasses.emplace_back(renderpass);
            }

            void removeRenderPass(RenderPassWPtr renderpass);

            void insertFence(FenceSPtr fence)
            {
                MouCa::preCondition(fence.get()); //DEV Issue: Need valid data
                _fences.emplace_back(fence);
            }
            void removeFence(FenceWPtr fence);

            void insertSequence(QueueSequenceSPtr sequences)
            {
                MouCa::preCondition(sequences.get()); //DEV Issue: Need valid data
                _sequences.emplace_back(sequences);
            }

            void insertGraphicsPipeline(GraphicsPipelineSPtr pipelineGraphic)
            {
                MouCa::preCondition(pipelineGraphic.get()); //DEV Issue: Need valid data
                _graphicsPipelines.emplace_back(pipelineGraphic);
            }

            void insertPipelineLayout(PipelineLayoutSPtr pipelineLayout)
            {
                MouCa::preCondition(pipelineLayout.get()); //DEV Issue: Need valid data
                _pipelineLayouts.emplace_back(pipelineLayout);
            }

            void insertDescriptorSetLayout(DescriptorSetLayoutSPtr descriptorSetLayout)
            {
                MouCa::preCondition(descriptorSetLayout.get()); //DEV Issue: Need valid data
                _descriptorSetLayouts.emplace_back(descriptorSetLayout);
            }

            void insertShaderModule(ShaderModuleSPtr shader)
            {
                MouCa::preCondition(shader.get()); //DEV Issue: Need valid data
                _shaderModules.emplace_back(shader);
            }

            void insertDescriptorSet(DescriptorSetSPtr descriptorSet)
            {
                MouCa::preCondition(descriptorSet.get()); //DEV Issue: Need valid data
                _descriptorSets.emplace_back(descriptorSet);
            }

            void insertDescriptorPool(DescriptorPoolSPtr descriptorPool)
            {
                MouCa::preCondition(descriptorPool.get()); //DEV Issue: Need valid data
                _descriptorPools.emplace_back(descriptorPool);
            }

            void insertSampler(SamplerSPtr sampler)
            {
                MouCa::preCondition(sampler.get()); //DEV Issue: Need valid data
                _samplers.emplace_back(sampler);
            }

            void insertCommandBuffer(CommandBufferSPtr commandBuffer)
            {
                MouCa::preCondition(commandBuffer.get()); //DEV Issue: Need valid data
                _commandBuffers.emplace_back(commandBuffer);
            }

            void insertCommandPool(CommandPoolSPtr commandPool)
            {
                MouCa::preCondition(commandPool.get()); //DEV Issue: Need valid data
                _commandPools.emplace_back(commandPool);
            }

            void insertRayTracingPipeline(RayTracingPipelineSPtr rayTracingPipeline)
            {
                MouCa::preCondition(rayTracingPipeline.get()); //DEV Issue: Need valid data
                _rayTracingPipelines.emplace_back(rayTracingPipeline);
            }

            void insertAccelerationStructure(AccelerationStructureSPtr as)
            {
                MouCa::preCondition(as.get()); //DEV Issue: Need valid data
                _accelerationStructures.emplace_back(as);
            }

            void insertTracingRay(TracingRaySPtr tracingRay)
            {
                MouCa::preCondition(tracingRay.get()); //DEV Issue: Need valid data
                _tracingRays.emplace_back(tracingRay);
            }

            RayTracingPipelines& getRayTracingPipelines() { return _rayTracingPipelines; }

            GraphicsPipelines& getGraphicsPipelines() { return _graphicsPipelines; }

        private:
            Device              _device;                ///< Vulkan Device data.
            PipelineCache       _pipelineCache;         ///< Pipeline cache.

        // WARNING: Declaration in right creation order !

        // Device data
            Semaphores          _semaphores;            ///< [OWNERSHIP]
            Fences              _fences;                ///< [OWNERSHIP]
            Images              _images;                ///< [OWNERSHIP]
            Samplers            _samplers;              ///< [OWNERSHIP]
            Buffers             _buffers;               ///< [OWNERSHIP]
            ShaderModules       _shaderModules;         ///< [OWNERSHIP]

            FrameBuffers        _frameBuffers;          ///< [OWNERSHIP]
            RenderPasses        _renderPasses;          ///< [OWNERSHIP]            

        // Pipeline
            DescriptorPools        _descriptorPools;       ///< [OWNERSHIP]
            DescriptorSetLayouts   _descriptorSetLayouts;  ///< [OWNERSHIP]
            DescriptorSets         _descriptorSets;        ///< [OWNERSHIP]
            PipelineLayouts        _pipelineLayouts;       ///< [OWNERSHIP]
            GraphicsPipelines      _graphicsPipelines;     ///< [OWNERSHIP]
            RayTracingPipelines    _rayTracingPipelines;   ///< [OWNERSHIP]
        
        // RayTracing
            AccelerationStructures _accelerationStructures;///< [OWNERSHIP]
            TracingRays            _tracingRays;           ///< [OWNERSHIP]

        // Sequence
            CommandPools         _commandPools;          ///< [OWNERSHIP]
            CommandBuffers       _commandBuffers;        ///< [OWNERSHIP]
            QueueSequences       _sequences;             ///< [OWNERSHIP]
    };

    using ContextDeviceSPtr = std::shared_ptr<ContextDevice>;
    using ContextDeviceWPtr = std::weak_ptr<ContextDevice>;
    using ContextDevices    = std::vector<ContextDeviceSPtr>;
}