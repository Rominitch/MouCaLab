#include "Dependancies.h"

#include "include/VulkanTest.h"

#include "LibCore/include/BTFile.h" 

#include "LibVulkan/include/VKDevice.h"
#include "LibVulkan/include/VKEnvironment.h"
#include "LibVulkan/include/VKShaderProgram.h"

class VulkanShaderProgram : public ::testing::Test
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

Vulkan::Environment VulkanShaderProgram::environment;
Vulkan::Device      VulkanShaderProgram::device;

TEST_F(VulkanShaderProgram, initialize)
{
    Core::File shaderFile(MouCaEnvironment::getInputPath() / L"libraries" / L"vertex.spv");
    ASSERT_NO_THROW(shaderFile.open());

    Vulkan::ShaderProgram program;
    ASSERT_TRUE(program.isNull());

    ASSERT_NO_THROW(program.initialize(device, shaderFile, std::string("main")));

    ASSERT_FALSE(program.isNull());
    ASSERT_TRUE(program.getInstance() != VK_NULL_HANDLE);

    ASSERT_NO_THROW(program.release(device));

    ASSERT_TRUE(program.isNull());
}

TEST_F(VulkanShaderProgram, badInitialize)
{
    Core::File badShaderFile(MouCaEnvironment::getInputPath() / L"libraries" / L"badShader.glsl");
    ASSERT_NO_THROW(badShaderFile.open());

    Vulkan::ShaderProgram program;
    ASSERT_TRUE(program.isNull());

    ASSERT_THROW(program.initialize(device, badShaderFile, std::string("main")), Core::Exception);

    ASSERT_TRUE(program.isNull());
}