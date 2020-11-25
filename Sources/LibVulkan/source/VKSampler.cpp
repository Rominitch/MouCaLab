/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include <LibVulkan/include/VKDevice.h>
#include <LibVulkan/include/VKSampler.h>

namespace Vulkan
{

Sampler::Sampler():
_sampler(VK_NULL_HANDLE)
{
    MOUCA_ASSERT(isNull());
}

Sampler::~Sampler()
{
    MOUCA_ASSERT(isNull());
}

void Sampler::initialize(const Device& device, VkFilter magFilter, VkFilter minFilter, VkSamplerMipmapMode mipmapMode,
                         VkSamplerAddressMode addressModeU,VkSamplerAddressMode addressModeV, VkSamplerAddressMode addressModeW,
                         float mipLodBias, VkBool32 anisotropyEnable, float maxAnisotropy,
                         VkBool32 compareEnable, VkCompareOp compareOp,
                         float  minLod, float maxLod,
                         VkBorderColor borderColor, VkBool32 unnormalizedCoordinates)
{
    MOUCA_PRE_CONDITION(isNull());
    MOUCA_PRE_CONDITION(!device.isNull());

    // Create sampler
    const VkSamplerCreateInfo samplerInfo
    {
        VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,      // VkStructureType         sType;
        nullptr,                                    // const void*             pNext;
        0,                                          // VkSamplerCreateFlags    flags;
        magFilter,                                  // VkFilter                magFilter;
        minFilter,                                  // VkFilter                minFilter;
        mipmapMode,                                 // VkSamplerMipmapMode     mipmapMode;
        addressModeU,                               // VkSamplerAddressMode    addressModeU;
        addressModeV,                               // VkSamplerAddressMode    addressModeV;
        addressModeW,                               // VkSamplerAddressMode    addressModeW;
        mipLodBias,                                 // float                   mipLodBias;
        anisotropyEnable,                           // VkBool32                anisotropyEnable;
        maxAnisotropy,                              // float                   maxAnisotropy;
        compareEnable,                              // VkBool32                compareEnable;
        compareOp,                                  // VkCompareOp             compareOp;
        minLod,                                     // float                   minLod;
        maxLod,                                     // float                   maxLod;
        borderColor,                                // VkBorderColor           borderColor;
        unnormalizedCoordinates                     // VkBool32                unnormalizedCoordinates;
    };

    if (vkCreateSampler(device.getInstance(), &samplerInfo, nullptr, &_sampler) != VK_SUCCESS)
    {
        BT_THROW_ERROR(u8"Vulkan", u8"SamplerCreationError");
    }
}

void Sampler::initialize(const Device& device, const float& maxLOD, const VkSamplerAddressMode addressModeUVW, float anisotropy, const VkFilter filter)
{
    MOUCA_ASSERT(isNull());
    MOUCA_ASSERT(!device.isNull());

    // Create sampler
    VkSamplerCreateInfo samplerInfo =
    {
        VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,       // VkStructureType         sType;
        nullptr,                                     // const void*             pNext;
        0,                                           // VkSamplerCreateFlags    flags;
        filter,                                      // VkFilter                magFilter;
        filter,                                      // VkFilter                minFilter;
        VK_SAMPLER_MIPMAP_MODE_LINEAR,               // VkSamplerMipmapMode     mipmapMode;
        addressModeUVW,                              // VkSamplerAddressMode    addressModeU;
        addressModeUVW,                              // VkSamplerAddressMode    addressModeV;
        addressModeUVW,                              // VkSamplerAddressMode    addressModeW;
        0.0f,                                        // float                   mipLodBias;
        static_cast<VkBool32>(anisotropy > 0.0f ? VK_TRUE : VK_FALSE), // VkBool32                anisotropyEnable;
        anisotropy,                                  // float                   maxAnisotropy;
        static_cast<VkBool32>(VK_FALSE),             // VkBool32                compareEnable;
        VK_COMPARE_OP_NEVER,                         // VkCompareOp             compareOp;
        0.0f,                                        // float                   minLod;
        maxLOD,                                      // float                   maxLod;
        VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,          // VkBorderColor           borderColor;
        static_cast<VkBool32>(VK_FALSE)              // VkBool32                unnormalizedCoordinates;
    };

    if(vkCreateSampler(device.getInstance(), &samplerInfo, nullptr, &_sampler) != VK_SUCCESS)
    {
        BT_THROW_ERROR(u8"Vulkan", u8"SamplerCreationError");
    }
}

void Sampler::release(const Device& device)
{
    MOUCA_ASSERT(!isNull());
    MOUCA_ASSERT(!device.isNull());

    vkDestroySampler(device.getInstance(), _sampler, nullptr);
    _sampler = VK_NULL_HANDLE;
}

}