
#include <Dependancies.h>

#include "include/VulkanTest.h"

#include <LibVulkan/include/VKEnvironment.h>
#include <LibVulkan/include/VKBuffer.h>
#include <LibVulkan/include/VKDevice.h>

class VulkanBuffer : public ::testing::Test
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

Vulkan::Environment VulkanBuffer::environment;
Vulkan::Device      VulkanBuffer::device;

TEST_F(VulkanBuffer, createBuffer)
{
    Vulkan::Buffer buffer;
    ASSERT_TRUE(buffer.isNull());

    const VkDeviceSize size = 30;
    ASSERT_NO_THROW(buffer.initialize(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, size));
    ASSERT_FALSE(buffer.isNull());

    const auto& descriptor = buffer.getDescriptor();
    EXPECT_EQ(buffer.getBuffer(), descriptor.buffer);
    EXPECT_EQ(0,                  descriptor.offset);
    EXPECT_EQ(VK_WHOLE_SIZE,      descriptor.range);

    ASSERT_NO_THROW(buffer.release(device));

    ASSERT_TRUE(buffer.isNull());
}

TEST_F(VulkanBuffer, mapping)
{
    Vulkan::Buffer buffer;
    ASSERT_TRUE(buffer.isNull());

    const std::array<float, 5> data = {0.0f, 1.0f, 2.0f, 1e-15f, 1e15f};

    const VkDeviceSize size = sizeof(data);
    ASSERT_NO_THROW(buffer.initialize(device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, size, data.data()));
    ASSERT_FALSE(buffer.isNull());

    ASSERT_LE(size, buffer.getMemory().getAllocatedSize());

    // Read data inside buffer
    ASSERT_NO_THROW(buffer.getMemory().map(device));

    const float* valueBuffer = buffer.getMemory().getMappedMemory<float>();
    for(float value : data)
    {
        EXPECT_EQ(value, *valueBuffer);
        ++valueBuffer;
    }

    ASSERT_NO_THROW(buffer.getMemory().unmap(device));

    ASSERT_NO_THROW(buffer.release(device));

    ASSERT_TRUE(buffer.isNull());
}