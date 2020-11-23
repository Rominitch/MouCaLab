/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace Vulkan
{
    class Device;

    class PipelineLayout final
    {
        MOUCA_NOCOPY(PipelineLayout);

        public:
            /// Constructor
            PipelineLayout();
            /// Destructor
            ~PipelineLayout();

            const VkPipelineLayout& getInstance() const
            {
                return _layout;
            }

            void addPushConstant(const VkPushConstantRange& constant);

            void initialize(const Device& device, const std::vector<VkDescriptorSetLayout>& descriptorLayouts = std::vector<VkDescriptorSetLayout>());
            void release(const Device& device);
            
            bool isNull() const
            {
                return _layout == VK_NULL_HANDLE;
            }

        private:
            VkPipelineLayout _layout;

            std::vector<VkPushConstantRange> _constants;
    };
}
