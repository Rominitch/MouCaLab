/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "include/MouCaLab.h"

#include "LibVR/include/VRPlatform.h"

#include "LibVulkan/include/VKBuffer.h"
#include "LibVulkan/include/VKContextDevice.h"
#include "LibVulkan/include/VKSequence.h"

class VRTest : public MouCaLabTest
{
public:
    bool _vrEnabled=false;
    void SetUp() override
    {
        MouCaLabTest::SetUp();

        ASSERT_NO_THROW(_graphic.getVRPlatform().initialize()) << "VR is not supported";
        _vrEnabled = true;
    }

    void TearDown() override
    {
        if(_vrEnabled)
        {
            ASSERT_NO_THROW(_graphic.getVRPlatform().release());
        }

        MouCaLabTest::TearDown();
    }

    void updateUBO(MouCaGraphic::Engine3DXMLLoader& loader, const Vulkan::ContextDevice& context, float time)
    {
        ASSERT_EQ(1, loader._buffers.size());
        auto& memory = loader._buffers[0].lock()->getMemory();
        memory.map(context.getDevice());

        memcpy(memory.getMappedMemory<char>(), &time, sizeof(time));

        memory.unmap(context.getDevice());
    }
};

TEST_F(VRTest, platform)
{
    auto& vrEngine = _graphic.getVRPlatform();
    
    ASSERT_NO_THROW(vrEngine.pollEvents());
    ASSERT_NO_THROW(vrEngine.pollControllerEvents());
    ASSERT_NO_THROW(vrEngine.updateTracked());

    // Check best size
    EXPECT_LT(0ull, vrEngine.getRenderSize().x);
    EXPECT_LT(0ull, vrEngine.getRenderSize().y);
}

TEST_F(VRTest, run)
{
    auto& vrEngine = _graphic.getVRPlatform();
    MouCaGraphic::VulkanManager manager;

    MouCaGraphic::Engine3DXMLLoader loader(manager);
    ASSERT_NO_FATAL_FAILURE(loadEngine(loader, "VRTriangleScreenSpace.xml"));

    auto context = manager.getDevices().at(0);

    // Execute commands
    updateCommandBuffersSurface(loader);
    updateCommandBuffers(loader);

    const auto renderVRFrame = [&](const double)
    {
        ASSERT_NO_THROW(vrEngine.pollEvents());
        ASSERT_NO_THROW(vrEngine.pollControllerEvents());

        manager.execute(0, 1, true);

        ASSERT_NO_THROW(vrEngine.updateTracked());
    };

    context->getDevice().waitIdle();

    // Execute rendering
    if (MouCaEnvironment::isDemonstrator())
    {
        ASSERT_NO_THROW(vrEngine.pollEvents());
        ASSERT_NO_THROW(vrEngine.pollControllerEvents());
        ASSERT_NO_THROW(vrEngine.updateTracked());

        mainLoop(manager, "Triangle ScreenSpace Demo ", renderVRFrame);
    }
    else
    {
        auto queueSequences = context->getQueueSequences();
        ASSERT_EQ(2, queueSequences.size());

        // Run one frame
        for (const auto& sequence : *queueSequences.at(0))
        {
            ASSERT_EQ(VK_SUCCESS, sequence->execute(context->getDevice()));
        }

        renderVRFrame(0.0);

        takeScreenshot(manager, "VRCompanionTriangle.png");
    }

    // Clean
    ASSERT_NO_THROW(manager.release());

    // Clean allocate dialog
    ASSERT_NO_THROW(clearDialog(manager));
}