/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibRT/include/RTViewport.h>

#include <LibVulkan/include/VKShaderProgram.h>

namespace RT
{
    class ComponentDescriptor;
}

namespace Vulkan
{
    class PipelineLayout;
    class RenderPass;

    //----------------------------------------------------------------------------
    /// \brief Pipeline definition of shader for Vulkan.
    class PipelineStageShaders
    {
        public:
            /// Internal save
            struct ShaderInfo
            {
                ShaderInfo(ShaderModuleWPtr module, ShaderSpecialization&& specialization) :
                    _module(module), _specialization(std::move(specialization))
                {}

                ShaderModuleWPtr     _module;
                ShaderSpecialization _specialization;
            };

            /// Constructor
            PipelineStageShaders() = default;

            /// Destructor
            ~PipelineStageShaders()= default;

            void addShaderModule(const ShaderModuleWPtr shader, ShaderSpecialization&& specialization);

            //------------------------------------------------------------------------
            /// \brief  Add generically a shader into pipeline to use. Prefer named method like addShaderVertex.
            /// 
            /// \param[in] id: Stage of shader (fragment, vertex, ...).
            /// \param[in] shader: Link to shader information.
            [[deprecated]] void addShaderProgram(const RT::ShaderKind id, const ShaderProgram& shader);

            //------------------------------------------------------------------------
            /// \brief  Add vertex shader into pipeline to use.
            /// 
            /// \param[in] shader: Link to shader information.
            [[deprecated]] void addShaderVertex(const ShaderProgram& shader);

            //------------------------------------------------------------------------
            /// \brief  Add fragment shader into pipeline to use.
            /// 
            /// \param[in] shader: Link to shader information.
            [[deprecated]] void addShaderFragment(const ShaderProgram& shader);
            //------------------------------------------------------------------------
            /// \brief  Add tessellation control shader into pipeline to use.
            /// 
            /// \param[in] shader: Link to shader information.
            [[deprecated]] void addShaderTessellationControl(const ShaderProgram& shader);

            //------------------------------------------------------------------------
            /// \brief  Add tessellation evaluation shader into pipeline to use.
            /// 
            /// \param[in] shader: Link to shader information.
            [[deprecated]] void addShaderTessellationEvaluation(const ShaderProgram& shader);

            //------------------------------------------------------------------------
            /// \brief  Get stage information for Vulkan. Need to call all "addShaderXXX" before.
            /// 
            /// \returns Prepare PipelineShaderStage for Vulkan.
            const VkPipelineShaderStageCreateInfo* getStage() const;

            //------------------------------------------------------------------------
            /// \brief  Get how many shader was binded into pipeline.
            /// 
            /// \returns Number of shader binded.
            uint32_t getNbShaders() const
            {
                return static_cast<uint32_t>(_shaderStage.size());
            }

            const std::vector<ShaderInfo>& getShadersData() const { return _shaderData; }

            void update();

        private:
            std::vector<ShaderInfo>                      _shaderData;   ///< Keep memory alive.
            std::vector<VkPipelineShaderStageCreateInfo> _shaderStage;  ///< Vulkan data: 

            //------------------------------------------------------------------------
            /// \brief  Generic method to add a shader binding into pipeline.
            /// 
            /// \param[in] shader: Link to shader information.
            /// \param[in] stageFlag: Stage of shader (fragment, vertex, ...) in Vulkan.
            [[deprecated]] void addShaderGeneric(const ShaderProgram& shader, const VkShaderStageFlagBits stageFlag);
    };

    class PipelineStateInputAssembly
    {
        public:
            PipelineStateInputAssembly();

            VkPipelineInputAssemblyStateCreateInfo  _state;
    };

    class PipelineStateVertexInput
    {
        private:
            std::vector<VkVertexInputBindingDescription>    _bindingDescriptions;
            std::vector<VkVertexInputAttributeDescription>  _attributeDescriptions;

        public:
            VkPipelineVertexInputStateCreateInfo    _state;

            PipelineStateVertexInput();

            const std::vector<VkVertexInputBindingDescription>&    getBindingDescriptions() const   { return _bindingDescriptions; }
            const std::vector<VkVertexInputAttributeDescription>&  getAttributeDescriptions() const { return _attributeDescriptions; }

            void setBindingDescriptions(std::vector<VkVertexInputBindingDescription>&& bindings);
            void setAttributeDescriptions(std::vector<VkVertexInputAttributeDescription>&& attributs);
    };

    class PipelineStateViewport
    {
        private:
            std::vector<VkViewport>  _viewport;
            std::vector<VkRect2D>    _scissor;

        public:
            VkPipelineViewportStateCreateInfo _state;

            PipelineStateViewport();
            void addViewport(const VkViewport& viewport, const VkRect2D& scissor);
            void addDynamicViewport();
    };

    class PipelineStateDynamic
    {
        private:
            std::vector<VkDynamicState>      _dynamicStates; ///< Customizable dynamic state. Default: VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
        
        public:
            VkPipelineDynamicStateCreateInfo _state;

            PipelineStateDynamic();

            void addDynamicState(VkDynamicState state);
    };

    class PipelineStateRasterizer
    {
        public:
            VkPipelineRasterizationStateCreateInfo _state;

            PipelineStateRasterizer();
    };

    class PipelineStateMultisample
    {
        MOUCA_NOCOPY(PipelineStateMultisample);
        public:
            VkPipelineMultisampleStateCreateInfo _state;
            PipelineStateMultisample();
    };

    class PipelineStateBlending
    {
        private:
            std::vector<VkPipelineColorBlendAttachmentState> _attachment;

        public:
            VkPipelineColorBlendStateCreateInfo _state;

            PipelineStateBlending();

            static VkColorComponentFlags getColorComponentRGBA()
            {
                return static_cast<VkColorComponentFlags>(
                       VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                       VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);
            }

            void setColorBlendAttachments(const std::vector<VkPipelineColorBlendAttachmentState>& attachments);

            static VkPipelineColorBlendAttachmentState defaultAttachment();

            static VkPipelineColorBlendAttachmentState zeroAttachment();
    };

    class PipelineStateDepthStencil
    {
        public:
            VkPipelineDepthStencilStateCreateInfo _state;
            PipelineStateDepthStencil();
    };

    class PipelineStateTessellation
    {
        public:
            VkPipelineTessellationStateCreateInfo _state;
            PipelineStateTessellation();
    };

    //----------------------------------------------------------------------------
    /// \brief Allow to configure pipeline stage properly without huge user manipulation.
    ///
    /// \see   
    class PipelineStateCreateInfo final
    {
        MOUCA_NOCOPY_NOMOVE(PipelineStateCreateInfo);
        public:
            enum States
            {
                Uninitialized   = 0x0000000,

                VertexInput     = 0x0000001,
                InputAssembly   = 0x0000002,
                Viewport        = 0x0000004,
                Rasterization   = 0x0000008,
                Multisample     = 0x0000010,
                ColorBlend      = 0x0000020,
                Dynamic         = 0x0000040,
                DepthStencil    = 0x0000080,
                Tessellation    = 0x0000100,

                ViewportArea    = 0x000003F,
                Renderable      = 0x00000FF,
                AllStates       = 0x00001FF
            };

            /// Constructor
            PipelineStateCreateInfo();

            /// Destructor
            ~PipelineStateCreateInfo() = default;

            //------------------------------------------------------------------------
            /// \brief  Initialize current stages using default configuration mode.
            /// 
            /// \param[in] mode: all actives part of pipeline stage. For example VertexInput must be activate to get data with getInputVertexStage().
            void initialize(const States mode);

            States getMode() const
            {
                return _mode;
            }

            const PipelineStageShaders&  getStages() const           { return _shader; }
            PipelineStageShaders&        getStages()                 { return _shader; }

            PipelineStateInputAssembly& getInputAssembly()          { return _inputAssembly; }

            PipelineStateVertexInput&   getVertexInput()            { return _inputVertex; }

            PipelineStateTessellation&  getTessellation()
            {
                return _tessellation;
            }

            PipelineStateBlending& getBlending()
            {
                return _blending;
            }

            PipelineStateMultisample& getMultisample()
            {
                return _multiSampling;
            }

            PipelineStateRasterizer& getRasterizer()
            {
                return _rasterizer;
            }

            PipelineStateViewport& getViewport()
            {
                return _viewport;
            }

            PipelineStateDepthStencil& getDepthStencil()
            {
                return _depthStencil;
            }

            PipelineStateDynamic& getDynamic()
            {
                return _dynamic;
            }

            static VkFormat computeDescriptorFormat(const RT::ComponentDescriptor& component);

            VkPipelineVertexInputStateCreateInfo const*const   getInputVertexState() const   { return isMode(VertexInput)   ? &_inputVertex._state                   : nullptr; }
            VkPipelineInputAssemblyStateCreateInfo const*const getInputAssemblyState() const { return isMode(InputAssembly) ? &_inputAssembly._state              : nullptr; }
            VkPipelineViewportStateCreateInfo const*const      getViewportState() const      { return isMode(Viewport)      ? &_viewport._state         : nullptr; }
            VkPipelineRasterizationStateCreateInfo const*const getRasterizerState() const    { return isMode(Rasterization) ? &_rasterizer._state  : nullptr; }
            VkPipelineMultisampleStateCreateInfo const*const   getMultisampleState() const   { return isMode(Multisample)   ? &_multiSampling._state : nullptr; }
            VkPipelineColorBlendStateCreateInfo const*const    getBlendingState() const      { return isMode(ColorBlend)    ? &_blending._state            : nullptr; }
            VkPipelineDynamicStateCreateInfo const*const       getDynamicState() const       { return isMode(Dynamic)       ? &_dynamic._state           : nullptr; }
            VkPipelineDepthStencilStateCreateInfo const*const  getDepthStencilState() const  { return isMode(DepthStencil)  ? &_depthStencil._state             : nullptr; }
            VkPipelineTessellationStateCreateInfo const*const  getTessellationState() const  { return isMode(Tessellation)  ? &_tessellation._state             : nullptr; }

            VkGraphicsPipelineCreateInfo buildInfo(const RenderPass& renderPass, const PipelineLayout& layout) const;

        private:
            //------------------------------------------------------------------------
            /// \brief  Check if current mode is enabled into stages.
            /// 
            /// \param[in] mode: data to check.
            /// \returns True if present into stages, false otherwise.
            bool isMode(const States mode) const
            {
                return (_mode & mode) == mode;
            }

            States                      _mode;              ///< Enabled stages.
            PipelineStageShaders        _shader;
            PipelineStateVertexInput    _inputVertex;
            PipelineStateInputAssembly  _inputAssembly;
            PipelineStateDynamic        _dynamic;
            PipelineStateRasterizer     _rasterizer;
            PipelineStateMultisample    _multiSampling;
            PipelineStateBlending       _blending;
            PipelineStateViewport       _viewport;
            PipelineStateDepthStencil   _depthStencil;
            PipelineStateTessellation   _tessellation;
            
            uint32_t                    _subPass;
            VkPipeline                  _basePipelineHandle;
            int32_t                     _basePipelineIndex;
    };
}
