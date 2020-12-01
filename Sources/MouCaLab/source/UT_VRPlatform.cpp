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
    static bool _vrEnabled;
    static void SetUpTestSuite()
    {
        try
        {
            MouCaVR::Platform vrEngine;
            vrEngine.initialize();
            vrEngine.release();
            _vrEnabled = true;
        }
        catch(...)
        {
            _vrEnabled = false;
        }
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

bool VRTest::_vrEnabled = false;

TEST_F(VRTest, platform)
{
    if (!_vrEnabled)
        FAIL() << "VR is not supported";

    MouCaVR::Platform vrEngine;
    ASSERT_NO_THROW(vrEngine.initialize());

    ASSERT_NO_THROW(vrEngine.pollEvents());
    ASSERT_NO_THROW(vrEngine.pollControllerEvents());
    ASSERT_NO_THROW(vrEngine.updateTracked());

    // Check best size
    EXPECT_LT(0ull, vrEngine.getRenderSize().x);
    EXPECT_LT(0ull, vrEngine.getRenderSize().y);

    ASSERT_NO_THROW(vrEngine.release());
}

TEST_F(VRTest, run)
{
    if (!_vrEnabled)
        FAIL() << "VR is not supported";

    MouCaGraphic::VulkanManager manager;

    MouCaGraphic::Engine3DXMLLoader loader(manager);
    ASSERT_NO_FATAL_FAILURE(loadEngine(loader, u8"VRTriangleScreenSpace.xml"));

    auto context = manager.getDevices().at(0);

    // Execute rendering
#ifdef VULKAN_DEMO
    context->getDevice().waitIdle();

    auto& vrEngine = _environment.get3DEngine().getVRPlatform();

    //std::thread thread([&]()
    //    {
    //        //float fTime = 0.0;
    //        while(_environment.get3DEngine().getRTPlatform().isWindowsActive())
    //        {
    //            //updateUBO(loader, *context, fTime);
    //            //fTime += 0.00001f;
    //
                ASSERT_NO_THROW(vrEngine.pollEvents());
                ASSERT_NO_THROW(vrEngine.pollControllerEvents());
                ASSERT_NO_THROW(vrEngine.updateTracked());
    //        }
    //    });
    auto demo = [&] (const double timer)
    {
        ASSERT_NO_THROW(vrEngine.pollEvents());
        ASSERT_NO_THROW(vrEngine.pollControllerEvents());
        ASSERT_NO_THROW(vrEngine.updateTracked());
    };

    mainLoop(manager, u8"Triangle ScreenSpace Demo ", demo);

    //thread.join();
#else
    // Get allocated item
    context->getDevice().waitIdle();

    auto queueSequences = context->getQueueSequences();
    ASSERT_EQ(2, queueSequences.size());

    // Run one frame
    for (const auto& sequence : *queueSequences.at(0))
    {
        ASSERT_EQ(VK_SUCCESS, sequence->execute(context->getDevice()));
    }

    takeScreenshot(manager, L"VRCompanionTriangle.png");
#endif

    // Clean
    ASSERT_NO_THROW(manager.release());

    // Clean allocate dialog
    ASSERT_NO_THROW(clearDialog(manager));
}