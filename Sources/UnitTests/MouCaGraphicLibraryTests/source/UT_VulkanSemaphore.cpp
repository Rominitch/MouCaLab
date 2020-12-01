#include <Dependencies.h>

#include "include/VulkanTest.h"

#include <LibVulkan/include/VKEnvironment.h>
#include <LibVulkan/include/VKDevice.h>
#include <LibVulkan/include/VKSemaphore.h>

class VulkanSemaphore : public ::testing::Test
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

Vulkan::Environment VulkanSemaphore::environment;
Vulkan::Device      VulkanSemaphore::device;

TEST_F(VulkanSemaphore, initialize)
{
    Vulkan::Semaphore semaphore;

    ASSERT_TRUE(semaphore.isNull());

    ASSERT_NO_THROW(semaphore.initialize(device));

    ASSERT_FALSE(semaphore.isNull());
    ASSERT_TRUE(semaphore.getInstance() != VK_NULL_HANDLE);

    ASSERT_NO_THROW(semaphore.release(device));

    ASSERT_TRUE(semaphore.isNull());
}