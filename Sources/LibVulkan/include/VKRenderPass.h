/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace Vulkan
{
    class Device;

    class RenderPass final
    {
        MOUCA_NOCOPY(RenderPass);

        public:
            RenderPass();
            ~RenderPass();

            void initialize(const Device& device, std::vector<VkAttachmentDescription>&& attachments, std::vector<VkSubpassDescription>&& subpasses, std::vector<VkSubpassDependency>&& dependencies);

            void release(const Device& device);

            const VkRenderPass& getInstance() const
            {
                return _renderPass;
            }

            bool isNull() const
            {
                return _renderPass == VK_NULL_HANDLE;
            }

            const std::vector<VkSubpassDescription>& getSubPasses() const { return _subpasses; }

        private:
            VkRenderPass _renderPass;   ///< Vulkan instance.

        // Auto update data
            std::vector<VkAttachmentDescription> _attachments;
            std::vector<VkSubpassDescription>    _subpasses;
            std::vector<VkSubpassDependency>     _dependencies;
    };

    using RenderPassSPtr = std::shared_ptr<RenderPass>;
    using RenderPassWPtr = std::weak_ptr<RenderPass>;
}