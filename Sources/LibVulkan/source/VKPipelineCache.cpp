/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibVulkan/include/VKDevice.h"
#include "LibVulkan/include/VKPipelineCache.h"

namespace Vulkan
{

PipelineCache::PipelineCache():
_pipelineCache(VK_NULL_HANDLE)
{
    MouCa::assertion(isNull());
}

PipelineCache::~PipelineCache()
{
    MouCa::assertion(isNull());
}

void PipelineCache::initialize(const Device& device)
{
    MouCa::assertion(isNull());
    const VkPipelineCacheCreateInfo pipelineCacheCreateInfo =
    {
        VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,   // VkStructureType               sType;
        nullptr,                                        // const void*                   pNext;
        0,                                              // VkPipelineCacheCreateFlags    flags;
        0,                                              // size_t                        initialDataSize;
        nullptr                                         // const void*                   pInitialData;
    };
    
    if(vkCreatePipelineCache(device.getInstance(), &pipelineCacheCreateInfo, nullptr, &_pipelineCache))
    {
        throw Core::Exception(Core::ErrorData("Vulkan", "PipelineCacheCreationError"));
    }
}

void PipelineCache::release(const Device& device)
{
    MouCa::assertion(!isNull());

    vkDestroyPipelineCache(device.getInstance(), _pipelineCache, nullptr);
    _pipelineCache = VK_NULL_HANDLE;
}

}