/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace RT
{
    class BufferCPUBase;
    using BufferCPUBaseWPtr = std::weak_ptr<BufferCPUBase>;
        
    class Image;
    using ImageWPtr = std::weak_ptr<Image>;

    class BufferDescriptor;

    class RenderDialog;
    using RenderDialogWPtr = std::weak_ptr<RenderDialog>;

    class Window;
}

namespace MouCaCore
{
    class ResourceManager;
}

namespace XML
{
    class Node;
    using NodeUPtr = std::unique_ptr<Node>;

    class Parser;
    using ParserSPtr = std::shared_ptr<Parser>;
}

namespace Vulkan
{
    class Buffer;
    using BufferWPtr = std::weak_ptr<Buffer>;

    class Command;
    using CommandUPtr = std::unique_ptr<Command>;
    using Commands    = std::vector<CommandUPtr>;
    using CommandsUPtr = std::unique_ptr<Commands>;

    class CommandBuffer;
    using CommandBufferWPtr = std::weak_ptr<CommandBuffer>;

    class CommandPool;
    using CommandPoolWPtr = std::weak_ptr<CommandPool>;

    class ContextDevice;
    using ContextDeviceWPtr = std::weak_ptr<ContextDevice>;

    class ContextWindow;
    using ContextWindowWPtr = std::weak_ptr<ContextWindow>;

    class DescriptorPool;
    using DescriptorPoolWPtr = std::weak_ptr<DescriptorPool>;

    class DescriptorSetLayout;
    using DescriptorSetLayoutWPtr = std::weak_ptr<DescriptorSetLayout>;

    class Fence;
    using FenceWPtr = std::weak_ptr<Fence>;

    class FrameBuffer;
    using FrameBufferWPtr = std::weak_ptr<FrameBuffer>;

    class GraphicsPipeline;
    using GraphicsPipelineWPtr = std::weak_ptr<GraphicsPipeline>;

    class Image;
    using ImageWPtr = std::weak_ptr<Image>;

    class ImageView;
    using ImageViewWPtr = std::weak_ptr<ImageView>;

    class MemoryBuffer;
    using MemoryBufferUPtr = std::unique_ptr<MemoryBuffer>;
    
    struct PhysicalDeviceFeatures;

    class PipelineStateCreateInfo;
    class PipelineStageShaders;
    class PipelineLayout;
    using PipelineLayoutWPtr = std::weak_ptr<PipelineLayout>;

    class RayTracingPipeline;
    class RayTracingPipelines;
    using RayTracingPipelinesWPtr = std::weak_ptr<RayTracingPipelines>;

    class RenderPass;
    using RenderPassWPtr = std::weak_ptr<RenderPass>;

    class Sampler;
    using SamplerWPtr = std::weak_ptr<Sampler>;

    class Semaphore;
    using SemaphoreWPtr = std::weak_ptr<Semaphore>;

    class DescriptorSet;
    using DescriptorSetWPtr = std::weak_ptr<DescriptorSet>;

    class Sequence;
    using SequenceSPtr      = std::shared_ptr<Sequence>;
    using QueueSequence     = std::vector<SequenceSPtr>;
    using QueueSequenceWPtr = std::weak_ptr<QueueSequence>;

    class ShaderModule;
    using ShaderModuleWPtr = std::weak_ptr<ShaderModule>;

    class SubmitInfo;
    using SubmitInfoUPtr = std::unique_ptr<SubmitInfo>;
    using SubmitInfos    = std::vector<SubmitInfoUPtr>;
}

namespace MouCaGraphic
{
    class VulkanManager;
    class GraphicEngine;    

    struct SubPassAttachment;

    /// XML loader of Engine 3D.
    class Engine3DXMLLoader final
    {
        MOUCA_NOCOPY_NOMOVE(Engine3DXMLLoader);
        
        public:
            using WindowDict                = std::map<uint32_t, RT::RenderDialogWPtr>;
            using WindowSurfaceDict         = std::map<uint32_t, Vulkan::ContextWindowWPtr>;
            using SemaphoreDict             = std::map<uint32_t, Vulkan::SemaphoreWPtr>;
            using FenceDict                 = std::map<uint32_t, Vulkan::FenceWPtr>;
            using FrameBuffersDict          = std::map<uint32_t, Vulkan::FrameBufferWPtr>;
            using RenderPassesDict          = std::map<uint32_t, Vulkan::RenderPassWPtr>;
            using ImagesDict                = std::map<uint32_t, Vulkan::ImageWPtr>;
            using ImageViewsDict            = std::map<uint32_t, Vulkan::ImageViewWPtr>;
            using QueueSequenceDict         = std::map<uint32_t, Vulkan::QueueSequenceWPtr>;
            using GraphicsPipelineDict      = std::map<uint32_t, Vulkan::GraphicsPipelineWPtr>;
            using PipelineLayoutDict        = std::map<uint32_t, Vulkan::PipelineLayoutWPtr>;
            using DescriptorSetLayoutDict   = std::map<uint32_t, Vulkan::DescriptorSetLayoutWPtr>;
            using ShaderModuleDict          = std::map<uint32_t, Vulkan::ShaderModuleWPtr>;
            using BufferDict                = std::map<uint32_t, Vulkan::BufferWPtr>;
            using DescriptorSetDict         = std::map<uint32_t, Vulkan::DescriptorSetWPtr>;
            using DescriptorPoolDict        = std::map<uint32_t, Vulkan::DescriptorPoolWPtr>;
            using SamplerDict               = std::map<uint32_t, Vulkan::SamplerWPtr>;
            using CommandBufferDict         = std::map<uint32_t, Vulkan::CommandBufferWPtr>;
            using DeviceDict                = std::map<uint32_t, Vulkan::ContextDeviceWPtr>;
            using CommandPoolDict           = std::map<uint32_t, Vulkan::CommandPoolWPtr>;
            using CommandDict               = std::map<uint32_t, Vulkan::Command*>;
            using CommandsDict              = std::map<uint32_t, Vulkan::CommandsUPtr>;

            // CPU external data
            using CPUBufferDict      = std::map<uint32_t, RT::BufferCPUBaseWPtr>;
            using CPUImageDict       = std::map<uint32_t, RT::ImageWPtr>;
            using MeshDescriptorDict = std::map<uint32_t, RT::BufferDescriptor>;

            /// Create a loader attached to engine to fill using xml file.
            /// \param[in] engine: engine to fill.
            Engine3DXMLLoader(VulkanManager& manager) :
            _manager(manager)
            {}
            
            /// Destructor
            ~Engine3DXMLLoader();

            struct ContextLoading
            {
                public:
                    ContextLoading(GraphicEngine& engine, XML::Parser& parser, MouCaCore::ResourceManager& resources);

                    GraphicEngine&              _engine;
                    XML::Parser&                _parser;
                    MouCaCore::ResourceManager& _resources;

                    XML::NodeUPtr       _globalData;    ///< Saved node
                    RT::ApplicationInfo _info;

                    const Core::String& getFileName() const { return _xmlFileName; }
                private:
                    Core::String    _xmlFileName;
            };

            void load(ContextLoading& context);
                        
            /// Temporary data for build and retrieve data from ID
            DeviceDict              _devices;               ///< Pair of xml ID/ContextDevice
            WindowDict              _dialogs;               ///< Pair of xml ID/Dialog
            WindowSurfaceDict       _surfaces;              ///< Pair of xml ID/Surface
            SemaphoreDict           _semaphores;            ///< Pair of xml ID/Semaphore
            FenceDict               _fences;                ///< Pair of xml ID/Fence
            FrameBuffersDict        _frameBuffers;          ///< Pair of xml ID/FrameBuffer
            RenderPassesDict        _renderPasses;          ///< Pair of xml ID/RenderPass
            ImagesDict              _images;                ///< Pair of xml ID/Image
            BufferDict              _buffers;               ///< Pair of xml ID/Buffer
            ImageViewsDict          _view;                  ///< Pair of xml ID/View
            QueueSequenceDict       _queueSequences;        ///< Pair of xml ID/QueueSequence
            GraphicsPipelineDict    _graphicsPipelines;     ///< Pair of xml ID/GraphicsPipeline
            PipelineLayoutDict      _pipelineLayouts;       ///< Pair of xml ID/PipelineLayout
            DescriptorSetLayoutDict _descriptorSetLayouts;  ///< Pair of xml ID/DescriptorSetLayout
            DescriptorSetDict       _descriptorSets;        ///< Pair of xml ID/DescriptorSet
            ShaderModuleDict        _shaderModules;         ///< Pair of xml ID/ShaderModule
            DescriptorPoolDict      _descriptorPools;       ///< Pair of xml ID/DescriptorPool
            SamplerDict             _samplers;              ///< Pair of xml ID/Sampler
            CommandBufferDict       _commandBuffers;        ///< Pair of xml ID/CommandBuffer
            CommandPoolDict         _commandPools;          ///< Pair of xml ID/CommandPool
            CommandDict             _commandLinks;          ///< Pair of xml ID/Command
            CommandsDict            _commandsGroup;         ///< Pair of xml ID/Commands

            // CPU external data
            CPUBufferDict           _cpuBuffers;
            CPUImageDict            _cpuImages;
            MeshDescriptorDict      _cpuMeshDescriptors;

        private:
        //-----------------------------------------------------------------------------------------
        //                                      Dictionary
        //-----------------------------------------------------------------------------------------
            VulkanManager&  _manager;      ///< [Link] Manager to fill.

        //-----------------------------------------------------------------------------------------
        //                                      Helper
        //-----------------------------------------------------------------------------------------
            void loadExtensions(ContextLoading& context, std::vector<Core::String>& extensions);

        //-----------------------------------------------------------------------------------------
        //                                   Surface management
        //-----------------------------------------------------------------------------------------
            void loadWindows(ContextLoading& context);
            void loadVR(ContextLoading& context);
        //-----------------------------------------------------------------------------------------
        //                                     Main Object
        //-----------------------------------------------------------------------------------------
            void loadEnvironment(ContextLoading& context);
            void loadDevices(ContextLoading& context);
            void loadPhysicalDeviceFeatures(ContextLoading& context, Vulkan::PhysicalDeviceFeatures& mandatory);

        //-----------------------------------------------------------------------------------------
        //                                  Devices Objects
        //-----------------------------------------------------------------------------------------
            void loadRenderPasses(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak);
            void loadSurfaces(ContextLoading& context, Vulkan::ContextDeviceWPtr device);
            void loadSemaphores(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak);
            void loadFences(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak);

            void loadRenderPassAttachment(ContextLoading& context, const uint32_t id, std::vector<VkAttachmentDescription>& attachments);
            void loadRenderPassSubPass(ContextLoading& context, const uint32_t id, std::vector<VkSubpassDescription>& subpasses, std::vector<SubPassAttachment>& attachmentReferences);
            void loadRenderPassDependency(ContextLoading& context, const uint32_t id, std::vector<VkSubpassDependency>& dependencies, const std::vector<VkSubpassDescription>& subPass);

            void loadBuffers(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak);
            void loadImagesAndView(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak);
            void loadSamplers(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak);
            void loadFrameBuffers(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak);

            void loadCommandPools(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak);
            void loadCommandBuffers(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak);
            void loadCommandsGroup(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak);
            void loadCommands(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak, const VkExtent2D& resolution, std::vector<Vulkan::CommandUPtr>& commands, const Core::String& nodeName=u8"Command");

            void loadQueueSequences(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak);
            void loadSequences(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak, Vulkan::QueueSequenceWPtr queueSequence);

            void loadSubmitInfo(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak, Vulkan::SubmitInfos& submitInfos);

            void loadDescriptorSetPools(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak);
            void loadDescriptorSets(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak);
            void loadDescriptorSetLayouts(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak);

            void loadPipelineLayouts(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak);
            void loadGraphicsPipelines(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak);
            void loadPipelineStateCreate(ContextLoading& context, Vulkan::PipelineStateCreateInfo& info, const uint32_t graphicsPipelineId, const uint32_t renderPassId);
            void loadPipelineStages(ContextLoading& context, Vulkan::PipelineStageShaders& stageShader);

            void loadShaderModules(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak);

            void loadMemoryBuffer(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak, Vulkan::MemoryBufferUPtr& memoryBuffer);

            void loadRayTracingPipelines(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak);
            void loadRayTracingShaderGroup(ContextLoading& context, Vulkan::ContextDeviceWPtr deviceWeak, Vulkan::RayTracingPipeline& pipeline);
    };
}