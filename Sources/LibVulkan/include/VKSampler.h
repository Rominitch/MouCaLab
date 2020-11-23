/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace Vulkan
{
    class Device;
    
    class Sampler final
    {
        private:
            MOUCA_NOCOPY(Sampler);

            VkSampler _sampler;
        
        public:
            Sampler();
            ~Sampler();

            void initialize(const Device& device, VkFilter magFilter, VkFilter minFilter, VkSamplerMipmapMode mipmapMode,
                            VkSamplerAddressMode addressModeU, VkSamplerAddressMode addressModeV, VkSamplerAddressMode addressModeW,
                            float mipLodBias, VkBool32 anisotropyEnable, float maxAnisotropy, VkBool32 compareEnable, VkCompareOp compareOp, float  minLod, float maxLod,
                            VkBorderColor borderColor, VkBool32 unnormalizedCoordinates);

            void initialize(const Device& device, const float& maxLOD, const VkSamplerAddressMode addressModeUVW = VK_SAMPLER_ADDRESS_MODE_REPEAT, float anisotropy = 0.0f, const VkFilter filter = VK_FILTER_LINEAR);
            void release(const Device& device);

            bool isNull() const
            {
                return _sampler == VK_NULL_HANDLE;
            }

            const VkSampler& getInstance() const
            {
                return _sampler;
            }
    };

    using SamplerSPtr = std::shared_ptr<Sampler>;
    using SamplerWPtr = std::weak_ptr<Sampler>;
}
