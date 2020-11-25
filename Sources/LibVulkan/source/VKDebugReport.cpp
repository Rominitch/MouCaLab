/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"
#include <LibVulkan/include/VKDebugReport.h>

namespace Vulkan
{

DebugReport::DebugReport():
_callback(VK_NULL_HANDLE)
{
    MOUCA_ASSERT(isNull());
}

DebugReport::~DebugReport()
{
    MOUCA_ASSERT(isNull());
}

void DebugReport::initialize(const VkInstance& vulkanInstance)
{
    MOUCA_ASSERT(isNull());
    MOUCA_ASSERT(vulkanInstance != VK_NULL_HANDLE);
  
    const VkDebugReportCallbackCreateInfoEXT callbackCreateInfo =
    {
        VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,            //VkStructureType                 sType;
        nullptr,                                                            //const void*                     pNext;
        //VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT | 
        VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT,    //VkDebugReportFlagsEXT           flags;
        debugCallback,                                                      //PFN_vkDebugReportCallbackEXT    pfnCallback;
        nullptr                                                             //void*                           pUserData;
    };

    auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vulkanInstance, "vkCreateDebugReportCallbackEXT");
    if(func == nullptr)
    {
        BT_THROW_ERROR("Vulkan", "DebugReportError");
    }

    if(func(vulkanInstance, &callbackCreateInfo, nullptr, &_callback) != VK_SUCCESS)
    {
        BT_THROW_ERROR("Vulkan", "DebugReportError");
    }
}

void DebugReport::release(const VkInstance& vulkanInstance)
{
    MOUCA_ASSERT(!isNull());
    auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vulkanInstance, "vkDestroyDebugReportCallbackEXT");
    if(func == nullptr)
    {
        BT_THROW_ERROR("Vulkan", "DebugReportError");
    }

    func(vulkanInstance, _callback, nullptr);

    _callback = VK_NULL_HANDLE;
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugReport::debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData)
{
    Core::String typeError(u8"Unknown");
    if(flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
        typeError = u8"Information";
    else if(flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
        typeError = u8"Warning";
    else if(flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
        typeError = u8"Performance";
    else if(flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        typeError = u8"Error";
    else if(flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
        typeError = u8"Debug";

    Core::String message(msg);

    Core::String print;
    if(flags & VK_DEBUG_REPORT_ERROR_BIT_EXT || flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
    {
        print = u8"----------------------------------------------------------\r\nVulkan: "
            + typeError + u8":\r\n\t" + message
            + u8"\r\n----------------------------------------------------------\r\n";
    }
    else
    {
        print = u8"Vulkan: " + typeError + u8":\t" + message + u8"\r\n";
    }

    std::cerr << print.c_str() << std::endl;
    OutputDebugString(Core::convertToOS(print).c_str());
    
    if(flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        return VK_FALSE;

    return VK_TRUE;
}

PFN_vkDebugMarkerSetObjectTagEXT    pfnDebugMarkerSetObjectTag  = VK_NULL_HANDLE;
PFN_vkDebugMarkerSetObjectNameEXT   pfnDebugMarkerSetObjectName = VK_NULL_HANDLE;
PFN_vkCmdDebugMarkerBeginEXT        pfnCmdDebugMarkerBegin      = VK_NULL_HANDLE;
PFN_vkCmdDebugMarkerEndEXT          pfnCmdDebugMarkerEnd        = VK_NULL_HANDLE;
PFN_vkCmdDebugMarkerInsertEXT       pfnCmdDebugMarkerInsert     = VK_NULL_HANDLE;

void DebugReport::setup(VkDevice device)
{
    pfnDebugMarkerSetObjectTag  = reinterpret_cast<PFN_vkDebugMarkerSetObjectTagEXT>(vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectTagEXT"));
    pfnDebugMarkerSetObjectName = reinterpret_cast<PFN_vkDebugMarkerSetObjectNameEXT>(vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectNameEXT"));
    pfnCmdDebugMarkerBegin      = reinterpret_cast<PFN_vkCmdDebugMarkerBeginEXT>(vkGetDeviceProcAddr(device, "vkCmdDebugMarkerBeginEXT"));
    pfnCmdDebugMarkerEnd        = reinterpret_cast<PFN_vkCmdDebugMarkerEndEXT>(vkGetDeviceProcAddr(device, "vkCmdDebugMarkerEndEXT"));
    pfnCmdDebugMarkerInsert     = reinterpret_cast<PFN_vkCmdDebugMarkerInsertEXT>(vkGetDeviceProcAddr(device, "vkCmdDebugMarkerInsertEXT"));
}

void insert(VkCommandBuffer cmdbuffer, std::string markerName, glm::vec4 color)
{
    // Check for valid function pointer (may not be present if not running in a debugging application)
    if(pfnCmdDebugMarkerInsert)
    {
        VkDebugMarkerMarkerInfoEXT markerInfo = {};
        markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
        memcpy(markerInfo.color, &color[0], sizeof(float) * 4);
        markerInfo.pMarkerName = markerName.c_str();
        pfnCmdDebugMarkerInsert(cmdbuffer, &markerInfo);
    }
}

/*
void setObjectName(VkDevice device, uint64_t object, VkDebugReportObjectTypeEXT objectType, const char *name)
{
    // Check for valid function pointer (may not be present if not running in a debugging application)
    if(pfnDebugMarkerSetObjectName)
    {
        VkDebugMarkerObjectNameInfoEXT nameInfo = {};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = objectType;
        nameInfo.object = object;
        nameInfo.pObjectName = name;
        pfnDebugMarkerSetObjectName(device, &nameInfo);
    }
}

void setObjectTag(VkDevice device, uint64_t object, VkDebugReportObjectTypeEXT objectType, uint64_t name, size_t tagSize, const void* tag)
{
    // Check for valid function pointer (may not be present if not running in a debugging application)
    if(pfnDebugMarkerSetObjectTag)
    {
        VkDebugMarkerObjectTagInfoEXT tagInfo = {};
        tagInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT;
        tagInfo.objectType = objectType;
        tagInfo.object = object;
        tagInfo.tagName = name;
        tagInfo.tagSize = tagSize;
        tagInfo.pTag = tag;
        pfnDebugMarkerSetObjectTag(device, &tagInfo);
    }
}

void beginRegion(VkCommandBuffer cmdbuffer, const char* pMarkerName, glm::vec4 color)
{
    // Check for valid function pointer (may not be present if not running in a debugging application)
    if(pfnCmdDebugMarkerBegin)
    {
        VkDebugMarkerMarkerInfoEXT markerInfo = {};
        markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
        memcpy(markerInfo.color, &color[0], sizeof(float) * 4);
        markerInfo.pMarkerName = pMarkerName;
        pfnCmdDebugMarkerBegin(cmdbuffer, &markerInfo);
    }
}

void endRegion(VkCommandBuffer cmdBuffer)
{
    // Check for valid function (may not be present if not runnin in a debugging application)
    if(pfnCmdDebugMarkerEnd)
    {
        pfnCmdDebugMarkerEnd(cmdBuffer);
    }
}

void setCommandBufferName(VkDevice device, VkCommandBuffer cmdBuffer, const char * name)
{
    setObjectName(device, (uint64_t)cmdBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, name);
}

void setQueueName(VkDevice device, VkQueue queue, const char * name)
{
    setObjectName(device, (uint64_t)queue, VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT, name);
}

void setImageName(VkDevice device, VkImage image, const char * name)
{
    setObjectName(device, (uint64_t)image, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, name);
}

void setSamplerName(VkDevice device, VkSampler sampler, const char * name)
{
    setObjectName(device, (uint64_t)sampler, VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT, name);
}

void setBufferName(VkDevice device, VkBuffer buffer, const char * name)
{
    setObjectName(device, (uint64_t)buffer, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, name);
}

void setDeviceMemoryName(VkDevice device, VkDeviceMemory memory, const char * name)
{
    setObjectName(device, (uint64_t)memory, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT, name);
}

void setShaderModuleName(VkDevice device, VkShaderModule shaderModule, const char * name)
{
    setObjectName(device, (uint64_t)shaderModule, VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT, name);
}

void setPipelineName(VkDevice device, VkPipeline pipeline, const char * name)
{
    setObjectName(device, (uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, name);
}

void setPipelineLayoutName(VkDevice device, VkPipelineLayout pipelineLayout, const char * name)
{
    setObjectName(device, (uint64_t)pipelineLayout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, name);
}

void setRenderPassName(VkDevice device, VkRenderPass renderPass, const char * name)
{
    setObjectName(device, (uint64_t)renderPass, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT, name);
}

void setFramebufferName(VkDevice device, VkFramebuffer framebuffer, const char * name)
{
    setObjectName(device, (uint64_t)framebuffer, VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, name);
}

void setDescriptorSetLayoutName(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const char * name)
{
    setObjectName(device, (uint64_t)descriptorSetLayout, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, name);
}

void setDescriptorSetName(VkDevice device, VkDescriptorSet descriptorSet, const char * name)
{
    setObjectName(device, (uint64_t)descriptorSet, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT, name);
}

void setSemaphoreName(VkDevice device, VkSemaphore semaphore, const char * name)
{
    setObjectName(device, (uint64_t)semaphore, VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT, name);
}

void setFenceName(VkDevice device, VkFence fence, const char * name)
{
    setObjectName(device, (uint64_t)fence, VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT, name);
}

void setEventName(VkDevice device, VkEvent _event, const char * name)
{
    setObjectName(device, (uint64_t)_event, VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT, name);
}
*/
}