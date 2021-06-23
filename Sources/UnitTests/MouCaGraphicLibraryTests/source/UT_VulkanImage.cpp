#include "Dependencies.h"

#include "include/VulkanTest.h"

#include <LibVulkan/include/VKDevice.h>
#include <LibVulkan/include/VKCommandBuffer.h>
#include <LibVulkan/include/VKCommandPool.h>
#include <LibVulkan/include/VKEnvironment.h>
#include <LibVulkan/include/VKImage.h>

class VulkanImage : public ::testing::Test
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
};

Vulkan::Environment VulkanImage::environment;
Vulkan::Device      VulkanImage::device;

TEST_F(VulkanImage, createImage)
{
    Vulkan::Image texture;
    ASSERT_TRUE(texture.isNull());

    const VkImageUsageFlags usage = static_cast<VkImageUsageFlags>(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    const Vulkan::Image::Size size({ 5, 1, 1 });
    ASSERT_NO_THROW(texture.initialize(device, size, 
                                       VK_IMAGE_TYPE_1D, VK_FORMAT_R32_SFLOAT, VK_SAMPLE_COUNT_1_BIT,
                                       VK_IMAGE_TILING_OPTIMAL, usage, VK_SHARING_MODE_EXCLUSIVE,
                                       VK_IMAGE_LAYOUT_UNDEFINED,
                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
    ASSERT_FALSE(texture.isNull());

    ASSERT_EQ(1ul, texture.getMaxMipLevels());
    ASSERT_TRUE(texture.getImage() != VK_NULL_HANDLE);
    ASSERT_EQ(VK_FORMAT_R32_SFLOAT, texture.getFormat());

    ASSERT_NO_THROW(texture.release(device));

    ASSERT_TRUE(texture.isNull());
}

TEST_F(VulkanImage, mapping)
{
    Vulkan::Image texture;
    ASSERT_TRUE(texture.isNull());
    /*
    const Core::Path outputPath = MouCaEnvironment::getInputPath() / "textures" / "Demo.ktx";

    Media
    // Read texture data from file
    gli::texture2d texture2D(gli::load(outputPath.u8string()));
    ASSERT_FALSE(texture2D.empty());

    // Build image
    const VkImageUsageFlags usage = static_cast<VkImageUsageFlags>(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    const auto& s = texture2D.extent(0);
    const Vulkan::Image::Size size({ static_cast<uint32_t>(s.x), static_cast<uint32_t>(s.y), 1 }, static_cast<uint32_t>(texture2D.levels()));
    ASSERT_NO_THROW(texture.initialize(device, size,
                                       VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UINT, VK_SAMPLE_COUNT_1_BIT,
                                       VK_IMAGE_TILING_OPTIMAL, usage, VK_SHARING_MODE_EXCLUSIVE,
                                       VK_IMAGE_LAYOUT_UNDEFINED,
                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
    ASSERT_FALSE(texture.isNull());

    //Vulkan::CommandPool commandPool;
    //ASSERT_NO_THROW( commandPool.initialize(device, device.getQueueFamilyGraphicId()) );
    //
    //Vulkan::CommandBufferOld command;
    //ASSERT_NO_THROW( command.initialize(device, commandPool, 1) );
    //
    //ASSERT_NO_THROW(texture.loadTexture(device, command, texture2D));
    
//    ASSERT_LE(memorySize, texture.getMemory().getAllocatedSize());

    // Read data inside buffer
//     ASSERT_NO_THROW(texture.getMemory().map(device));
// 
//     Core::SPictureUC32 const* imageData = reinterpret_cast<Core::SPictureUC32 const*const>(texture2D[0].data());
//     const Core::SPictureUC32* valueBuffer    = texture.getMemory().getMappedMemory<Core::SPictureUC32>();
//     for(size_t index=0; index < memorySize; ++index)
//     {
//         EXPECT_EQ(imageData->m_color, valueBuffer->m_color);
//         ++valueBuffer;
//         ++imageData;
//     }
// 
//     ASSERT_NO_THROW(texture.getMemory().unmap(device));

    ASSERT_NO_THROW(texture.release(device));

    //ASSERT_NO_THROW(command.release(device, commandPool));

    //ASSERT_NO_THROW(commandPool.release(device));
    */
    ASSERT_TRUE(texture.isNull());
}