#include <Dependencies.h>

#include "include/VulkanTest.h"

#include <LibVulkan/include/VKEnvironment.h>
#include <LibVulkan/include/VKDevice.h>
#include <LibVulkan/include/VKFence.h>

class VulkanFence : public ::testing::Test
{
protected:
    static Vulkan::Environment environment;
    static Vulkan::Device      device;

    static void SetUpTestSuite()
    {
        ASSERT_NO_THROW(environment.initialize(g_info));
        ASSERT_NO_THROW(device.initializeBestGPU(environment));
    }

    static void TearDownTestSuite()
    {
        ASSERT_NO_THROW(device.release());
        ASSERT_NO_THROW(environment.release());
    }

    virtual void SetUp() final
    {}

    virtual void TearDown() final
    {}
};

Vulkan::Environment VulkanFence::environment;
Vulkan::Device      VulkanFence::device;

TEST_F(VulkanFence, initialize)
{
    Vulkan::Fence fence;

    ASSERT_TRUE(fence.isNull());

    ASSERT_NO_THROW(fence.initialize(device, 10, 0));

    ASSERT_FALSE(fence.isNull());
    ASSERT_TRUE(fence.getInstance() != VK_NULL_HANDLE);

    ASSERT_NO_THROW(fence.release(device));

    ASSERT_TRUE(fence.isNull());
}

TEST_F(VulkanFence, nothingSync)
{
    Vulkan::Fence fence;
    ASSERT_NO_THROW(fence.initialize(device, 10, 0));

    //EXPECT_EQ(VK_TIMEOUT, fence.wait(device, 10) );
    EXPECT_NE(VK_SUCCESS, fence.wait(device));      //Since LunarG operation is not possible ?

    ASSERT_NO_THROW(fence.release(device));
}