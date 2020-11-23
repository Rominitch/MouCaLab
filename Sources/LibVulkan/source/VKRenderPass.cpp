/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include "LibVulkan/include/VKDevice.h"
#include "LibVulkan/include/VKRenderPass.h"
#include "LibVulkan/include/VKSurface.h"

namespace Vulkan
{

RenderPass::RenderPass():
_renderPass(VK_NULL_HANDLE)
{
    BT_PRE_CONDITION(isNull());
}

RenderPass::~RenderPass()
{
    BT_PRE_CONDITION(isNull());
}

void RenderPass::initialize(const Device& device, std::vector<VkAttachmentDescription>&& attachments, std::vector<VkSubpassDescription>&& subpasses, std::vector<VkSubpassDependency>&& dependencies)
{
    BT_PRE_CONDITION(isNull());
    BT_PRE_CONDITION(!device.isNull());
   
    _attachments    = std::move(attachments);
    _subpasses      = std::move(subpasses);
    _dependencies   = std::move(dependencies);

    VkRenderPassCreateInfo renderPassCreateInfo =
    {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,             // VkStructureType                sType
        nullptr,                                               // const void                    *pNext
        0,                                                     // VkRenderPassCreateFlags        flags
        static_cast<uint32_t>(_attachments.size()),            // uint32_t                       attachmentCount
        _attachments.empty()  ? nullptr : _attachments.data(), // const VkAttachmentDescription *pAttachments
        static_cast<uint32_t>(_subpasses.size()),              // uint32_t                       subpassCount
        _subpasses.empty()    ? nullptr : _subpasses.data(),   // const VkSubpassDescription    *pSubpasses
        static_cast<uint32_t>(_dependencies.size()),           // uint32_t                       dependencyCount
        _dependencies.empty() ? nullptr : _dependencies.data() // const VkSubpassDependency     *pDependencies
    };

    if(vkCreateRenderPass(device.getInstance(), &renderPassCreateInfo, nullptr, &_renderPass) != VK_SUCCESS)
    {
        BT_THROW_ERROR(u8"Vulkan", u8"RenderPassCreationError");
    }
}

void RenderPass::release(const Device& device)
{
    BT_PRE_CONDITION(!isNull());
    BT_PRE_CONDITION(!device.isNull());

    vkDestroyRenderPass(device.getInstance(), _renderPass, nullptr);
    _renderPass = VK_NULL_HANDLE;
}

}