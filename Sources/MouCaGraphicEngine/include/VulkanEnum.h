#pragma once

// Forward declaration
namespace RT
{
    enum class ShaderKind : uint8_t;
};

//-----------------------------------------------------------------------------------------
//                                  Helper and string
//-----------------------------------------------------------------------------------------
namespace MouCaGraphic
{
    extern std::map<Core::String, VkAttachmentLoadOp> attachmentLoads;

    extern std::map<Core::String, VkAttachmentStoreOp> attachmentStores;

    extern std::map<Core::String, VkPipelineBindPoint> bindPoints;

    extern std::map<Core::String, VkImageLayout> imageLayouts;

    extern std::map<Core::String, VkPresentModeKHR> presentModes;

    extern std::map<Core::String, VkFormat> formats;

    extern std::map<Core::String, VkSurfaceTransformFlagBitsKHR> surfaceTransforms;

    extern std::map<Core::String, VkBufferCreateFlags> bufferCreates;

    extern std::map<Core::String, VkBufferUsageFlags> bufferUsages;

    extern std::map<Core::String, VkImageUsageFlags> imageUsages;

    extern std::map<Core::String, VkColorSpaceKHR> colorSpaces;

    extern std::map<Core::String, VkImageType> imageTypes;

    extern std::map<Core::String, VkSampleCountFlagBits> samples;

    extern std::map<Core::String, VkImageTiling> tilings;

    extern std::map<Core::String, VkSharingMode> sharingModes;

    extern std::map<Core::String, VkComponentSwizzle> componentSwizzles;

    extern std::map<Core::String, VkImageViewType> viewTypes;

    extern std::map<Core::String, VkMemoryAllocateFlags> memoryAllocates;

    extern std::map<Core::String, VkMemoryPropertyFlags> memoryProperties;

    extern std::map<Core::String, VkCommandBufferLevel> commandBufferLevels;

    extern std::map<Core::String, VkPipelineStageFlags> pipelineStageFlags;

    extern std::map<Core::String, VkSubpassContents> subpassContents;

    extern std::map<Core::String, uint32_t> subPassHelper;

    extern std::map<Core::String, VkAccessFlags> accessFlags;

    extern std::map<Core::String, VkDependencyFlags> dependencyFlags;

    extern std::map<Core::String, VkImageAspectFlags> aspectFlags;

    extern std::map<Core::String, uint32_t> subPassAttachmentHelper;

    extern std::map<Core::String, VkFenceCreateFlags> fenceCreates;

    extern std::map<Core::String, VkPrimitiveTopology> primitiveTopologies;

    extern std::map<Core::String, VkCullModeFlags> cullModes;

    extern std::map<Core::String, VkPolygonMode> polygonModes;

    extern std::map<Core::String, VkFrontFace> frontFaces;

    extern std::map<Core::String, VkShaderStageFlags> shaderStages;

    extern std::map<Core::String, VkLogicOp> logicOperators;

    extern std::map<Core::String, VkBlendOp> blendOperations;

    extern std::map<Core::String, VkBlendFactor> blendFactors;

    extern std::map<Core::String, VkColorComponentFlags> colorComponents;

    extern std::map<Core::String, VkCompareOp> compareOperations;

    extern std::map<Core::String, VkStencilOp> stencilOperations;

    extern std::map<Core::String, VkDynamicState> dynamics;

    extern std::map<Core::String, VkVertexInputRate> vertexInputRates;

    extern std::map<Core::String, VkDescriptorType> descriptorTypes;

    extern std::map<Core::String, VkIndexType> indexTypes;

    extern std::map<Core::String, VkFilter> filters;

    extern std::map<Core::String, VkSamplerMipmapMode> samplerMipmaps;

    extern std::map<Core::String, VkSamplerAddressMode> samplerAdresses;

    extern std::map<Core::String, VkBorderColor> borderColors;

    extern std::map<Core::String, VkCommandPoolCreateFlags> poolCreateFlags;

    extern std::map<Core::String, VkCommandBufferUsageFlags> commandBufferUsages;

    extern std::map<VkShaderStageFlags, RT::ShaderKind> shaderKinds;

    extern std::map<Core::String, VkRayTracingShaderGroupTypeKHR> rayTracingGroupTypes;
}
