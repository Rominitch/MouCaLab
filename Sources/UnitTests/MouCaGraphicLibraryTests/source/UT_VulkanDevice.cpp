#include <Dependancies.h>

#include "include/VulkanTest.h"

#include <LibVulkan/include/VKDevice.h>
#include <LibVulkan/include/VKEnvironment.h>
#include <LibVulkan/include/VKSemaphore.h>

//Need namespace for FRIEND_TEST
namespace Vulkan
{
    class VulkanDevice : public ::testing::Test
    {
        protected:
            static Vulkan::Environment environment;

            static void SetUpTestSuite()
            {
                ASSERT_NO_THROW(environment.initialize(g_info));
            }

            static void TearDownTestSuite()
            {
                ASSERT_NO_THROW(environment.release());
            }

            virtual void SetUp() final
            {}

            virtual void TearDown() final
            {}
    };

    Environment VulkanDevice::environment;


#ifndef NDEBUG
/*
TEST_F(VulkanDevice, instance)
{
    // Standard creation + delete
    ASSERT_NO_THROW(Vulkan::Device emptyDevice);

    // API Check
    {
        Vulkan::Device device;
        VkPhysicalDevice physicalDeviceID = VK_NULL_HANDLE;
        VkDevice deviceID = VK_NULL_HANDLE;
        ASSERT_DEATH(device.initialize(physicalDeviceID, deviceID, 0), "");
        ASSERT_DEATH(device.release(), "");
    }

    {
        Vulkan::Device device;
        ASSERT_NO_THROW(device.initialize(environment.getGraphicsPhysicalDevices().at(0), 0));
        ASSERT_DEATH(device.initialize(environment.getGraphicsPhysicalDevices().at(0), 0), "");

        ASSERT_NO_THROW(device.release());
        ASSERT_DEATH(device.release(), "");
    }
}
*/
#endif

// cppcheck-suppress syntaxError
TEST_F(VulkanDevice, initialize)
{
    // Build device
    Vulkan::Device device;
    ASSERT_NO_THROW(device.initialize(environment.getGraphicsPhysicalDevices().at(0), 0));
    ASSERT_NO_THROW(device.release());
}

TEST_F(VulkanDevice, initializeExtensions)
{
    std::vector<const char*> deviceExtensions =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    Vulkan::Device device;
    ASSERT_NO_THROW(device.initialize(environment.getGraphicsPhysicalDevices().at(0), 0, deviceExtensions));
    ASSERT_NO_THROW(device.release());
}

TEST_F(VulkanDevice, initializeInvalidExtensions)
{
    std::vector<const char*> deviceInvalidExtensions =
    {
        "MouCaExtension"
    };

    Vulkan::Device device;
    ASSERT_THROW(device.initialize(environment.getGraphicsPhysicalDevices().at(0), 0, deviceInvalidExtensions), Core::Exception);
    ASSERT_TRUE(device.isNull());
}

TEST_F(VulkanDevice, initializeBestGPU)
{
    Vulkan::Device device;
    ASSERT_NO_THROW(device.initializeBestGPU(environment));
    ASSERT_NO_THROW(device.release());
}

TEST_F(VulkanDevice, initializeBestGPUExtensions)
{
    std::vector<const char*> deviceExtensions =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    Vulkan::Device device;
    ASSERT_NO_THROW(device.initializeBestGPU(environment, deviceExtensions));
    ASSERT_NO_THROW(device.release());
}

TEST_F(VulkanDevice, initializeBestGPUInvalidExtensions)
{
    std::vector<const char*> deviceInvalidExtensions =
    {
        "MouCaExtension"
    };

    Vulkan::Device device;
    ASSERT_THROW(device.initializeBestGPU(environment, deviceInvalidExtensions), Core::Exception);
    ASSERT_TRUE(device.isNull());
}

TEST_F(VulkanDevice, getMemoryType)
{
    Vulkan::Device device;
    ASSERT_NO_THROW(device.initializeBestGPU(environment));

    VkImageCreateInfo imageInfo =
    {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,                // VkStructureType          
        nullptr,                                            // const void*              
        0,                                                  // VkImageCreateFlags       
        VK_IMAGE_TYPE_2D,                                   // VkImageType              
        VK_FORMAT_R16G16B16A16_SFLOAT,                      // VkFormat                 
        {32, 32, 1},                                        // VkExtent3D               
        1,                                                  // uint32_t                 
        1,                                                  // uint32_t                 
        VK_SAMPLE_COUNT_8_BIT,                              // VkSampleCountFlagBits    
        VK_IMAGE_TILING_OPTIMAL,                            // VkImageTiling            
        static_cast<VkImageUsageFlags>(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),  // VkImageUsageFlags        
        VK_SHARING_MODE_EXCLUSIVE,                          // VkSharingMode            
        0,                                                  // uint32_t                 
        nullptr,                                            // const uint32_t*          
        VK_IMAGE_LAYOUT_UNDEFINED                           // VkImageLayout            
    };

    VkImage image = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS, vkCreateImage(device.getInstance(), &imageInfo, nullptr, &image));
    
    VkMemoryRequirements memReqs;
    ASSERT_NO_THROW(vkGetImageMemoryRequirements(device.getInstance(), image, &memReqs));

    //uint32_t id = device.getMemoryType(memReqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    //EXPECT_EQ(7ul, id);

    ASSERT_NO_THROW(vkDestroyImage(device.getInstance(), image, nullptr));

    ASSERT_NO_THROW(device.release());
}

TEST_F(VulkanDevice, getQueueFamiliyIndex)
{
    Vulkan::Device device;
    ASSERT_NO_THROW(device.initializeBestGPU(environment));

    EXPECT_LE(0ul, device.getQueueFamiliyIndex(VK_QUEUE_COMPUTE_BIT));
    EXPECT_LE(0ul, device.getQueueFamiliyIndex(VK_QUEUE_TRANSFER_BIT));
    EXPECT_LE(0ul, device.getQueueFamiliyIndex(VK_QUEUE_GRAPHICS_BIT));

    const VkQueueFlagBits invalidFlag = static_cast<VkQueueFlagBits>(0x10000000);
    EXPECT_ANY_THROW(device.getQueueFamiliyIndex(invalidFlag));

    ASSERT_NO_THROW(device.release());
}

//--------------------------------------------------------------------------
// This test will:
//      - Create dummy device (not initialized)
//      - Check all parameters possible for checkExtensions
//--------------------------------------------------------------------------
TEST_F(VulkanDevice, checkExtensions)
{
    const std::vector<const char*> badExtensions =
    {
        "Extension invalid"
    };

    std::vector<const char*> deviceExtensions =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    Vulkan::Device device;

    ASSERT_FALSE(environment.getGraphicsPhysicalDevices().empty());
    const VkPhysicalDevice physical = environment.getGraphicsPhysicalDevices().at(0);

    // Check bad extensions
    EXPECT_ANY_THROW(device.checkExtensions(physical, badExtensions));
    // Check good extensions
    EXPECT_NO_THROW(device.checkExtensions(physical, deviceExtensions));
}

}

TEST(DeviceCandidate, compareLess)
{
    Vulkan::DeviceCandidate low;
    low.properties.limits.maxImageDimension2D = 4096;
    low.properties.limits.maxImageDimension3D = 1024;
    Vulkan::DeviceCandidate high;
    high.properties.limits.maxImageDimension2D = 8192;
    high.properties.limits.maxImageDimension3D = 2048;
    Vulkan::DeviceCandidate eq;
    eq.properties.limits.maxImageDimension2D = 8192;
    eq.properties.limits.maxImageDimension3D = 2048;

    Vulkan::DeviceCandidate::Less less;

    EXPECT_TRUE(less(low, high));
    EXPECT_FALSE(less(high, low));

    EXPECT_FALSE(less(high, eq));
    EXPECT_FALSE(less(eq, high));
}

