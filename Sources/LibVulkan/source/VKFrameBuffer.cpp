/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

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
    MouCa::preCondition(isNull());
}

FrameBuffer::~FrameBuffer()
{
    MouCa::postCondition(isNull());
}

void FrameBuffer::initialize(const Device& device, RenderPassWPtr renderPass, const VkExtent2D& resolution, const Attachments& attachments)
{
    MouCa::preCondition(isNull());
    MouCa::preCondition(!device.isNull());
    MouCa::preCondition(!attachments.empty());
    MouCa::preCondition(!renderPass.expired() && !renderPass.lock()->isNull());
    MouCa::preCondition(resolution.width > 0 && resolution.height > 0);

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
        throw Core::Exception(Core::ErrorData("Vulkan", "FrameBufferCreationError"));
    }

    MouCa::postCondition(!isNull());
}

void FrameBuffer::release(const Device& device)
{
    MouCa::preCondition(!isNull());
    MouCa::preCondition(!device.isNull());

    // Destroy
    vkDestroyFramebuffer(device.getInstance(), _frameBuffer, nullptr);
    
    // Reset all data
    _frameBuffer = VK_NULL_HANDLE;
    _resolution = { 0,0 };

    MouCa::postCondition(isNull());
}

}