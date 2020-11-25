/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include "LibVulkan/include/VKDevice.h"
#include "LibVulkan/include/VKFrameBuffer.h"
#include "LibVulkan/include/VKImage.h"
#include "LibVulkan/include/VKRenderPass.h"
#include "LibVulkan/include/VKSwapChain.h"

namespace Vulkan
{

FrameBuffer::FrameBuffer():
_resolution({ 0,0 }), _frameBuffer(VK_NULL_HANDLE)
{
    MOUCA_PRE_CONDITION(isNull());
}

FrameBuffer::~FrameBuffer()
{
    MOUCA_POST_CONDITION(isNull());
}

void FrameBuffer::initialize(const Device& device, RenderPassWPtr renderPass, const VkExtent2D& resolution, const Attachments& attachments)
{
    MOUCA_PRE_CONDITION(isNull());
    MOUCA_PRE_CONDITION(!device.isNull());
    MOUCA_PRE_CONDITION(!attachments.empty());
    MOUCA_PRE_CONDITION(!renderPass.expired() && !renderPass.lock()->isNull());
    MOUCA_PRE_CONDITION(resolution.width > 0 && resolution.height > 0);

    _renderPass = renderPass;
    _resolution = resolution;

    const VkFramebufferCreateInfo frameBufferInfo =
    {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,      // VkStructureType                sType
        nullptr,                                        // const void                    *pNext
        0,                                              // VkFramebufferCreateFlags       flags
        renderPass.lock()->getInstance(),               // VkRenderPass                   renderPass
        static_cast<uint32_t>(attachments.size()),      // uint32_t                       attachmentCount
        attachments.data(),                             // const VkImageView             *pAttachments
        _resolution.width,                              // uint32_t                       width
        _resolution.height,                             // uint32_t                       height
        1                                               // uint32_t                       layers
    };

    if (vkCreateFramebuffer(device.getInstance(), &frameBufferInfo, nullptr, &_frameBuffer) != VK_SUCCESS)
    {
        BT_THROW_ERROR(u8"Vulkan", u8"FrameBufferCreationError");
    }

    MOUCA_POST_CONDITION(!isNull());
}

void FrameBuffer::release(const Device& device)
{
    MOUCA_PRE_CONDITION(!isNull());
    MOUCA_PRE_CONDITION(!device.isNull());

    // Destroy
    vkDestroyFramebuffer(device.getInstance(), _frameBuffer, nullptr);
    
    // Reset all data
    _frameBuffer = VK_NULL_HANDLE;
    _resolution = { 0,0 };

    MOUCA_POST_CONDITION(isNull());
}

}