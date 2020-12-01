#include <Dependencies.h>

#include "include/VulkanTest.h"

#include <LibVulkan/include/VKEnvironment.h>

TEST(VulkanEnvironment, initialize)
{
	Vulkan::Environment environment;

	EXPECT_TRUE(environment.isNull());
	ASSERT_NO_THROW(environment.initialize(g_info));

	// Suppose DEV machine can run Vulkan !!!
	EXPECT_GE(1, environment.getGraphicsPhysicalDevices().size());
	EXPECT_FALSE(environment.isNull());

	ASSERT_NO_THROW(environment.release());
	
	EXPECT_TRUE(environment.isNull());
	EXPECT_EQ(0, environment.getGraphicsPhysicalDevices().size());
}

TEST(VulkanEnvironment, initializeExtensions)
{
	Vulkan::Environment environment;

	const std::vector<const char*> extensions =
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_WIN32_KHR
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_XCB_KHR)
		VK_KHR_XCB_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
		VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#endif
	};

	ASSERT_NO_THROW(environment.initialize(g_info, extensions));

	ASSERT_NO_THROW(environment.release());
}

TEST(VulkanEnvironment, initializeInvalidExtensions)
{
	Vulkan::Environment environment;

	const std::vector<const char*> invalidExtensions =
	{
		"MouCaExtension"
	};

	ASSERT_THROW(environment.initialize(g_info, invalidExtensions), Core::Exception);
	ASSERT_TRUE(environment.isNull());
}