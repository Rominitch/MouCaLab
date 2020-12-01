#include <Dependencies.h>

#include "include/VulkanTest.h"

#include <LibVulkan/include/VKEnvironment.h>
#include <LibVulkan/include/VKDevice.h>
#include <LibVulkan/include/VKPipelineCache.h>

class VulkanPipelineCache : public ::testing::Test
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

Vulkan::Environment VulkanPipelineCache::environment;
Vulkan::Device      VulkanPipelineCache::device;

TEST_F(VulkanPipelineCache, initialize)
{
    Vulkan::PipelineCache cache;

    ASSERT_TRUE(cache.isNull());

    ASSERT_NO_THROW(cache.initialize(device));

    ASSERT_FALSE(cache.isNull());
    ASSERT_TRUE(cache.getInstance() != VK_NULL_HANDLE);

    ASSERT_NO_THROW(cache.release(device));

    ASSERT_TRUE(cache.isNull());
}