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
    MOUCA_ASSERT(isNull());
}

PipelineCache::~PipelineCache()
{
    MOUCA_ASSERT(isNull());
}

void PipelineCache::initialize(const Device& device)
{
    MOUCA_ASSERT(isNull());
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
        MOUCA_THROW_ERROR(u8"Vulkan", u8"PipelineCacheCreationError");
    }
}

void PipelineCache::release(const Device& device)
{
    MOUCA_ASSERT(!isNull());

    vkDestroyPipelineCache(device.getInstance(), _pipelineCache, nullptr);
    _pipelineCache = VK_NULL_HANDLE;
}

}