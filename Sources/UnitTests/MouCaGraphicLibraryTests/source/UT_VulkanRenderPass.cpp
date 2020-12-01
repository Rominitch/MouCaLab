/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "include/VulkanTest.h"

#include <LibVulkan/include/VKContextDevice.h>
#include <LibVulkan/include/VKRenderPass.h>

class VulkanRenderPass : public VulkanDeviceTest
{
    protected:
        VulkanRenderPass():
        VulkanDeviceTest("VulkanRenderPass")
        {}
};

TEST_F(VulkanRenderPass, initialize)
{
    const auto& device = _contextDevice.getDevice();
    Vulkan::RenderPass renderPass;

    ASSERT_TRUE(renderPass.isNull());

    RenderPassParameters param;
    ASSERT_NO_THROW(renderPass.initialize(device, std::move(param.attachments), std::move(param.subpasses), std::move(param.dependencies)));

    ASSERT_FALSE(renderPass.isNull());

    ASSERT_NO_THROW(renderPass.release(device));
    ASSERT_TRUE(renderPass.isNull());
}
