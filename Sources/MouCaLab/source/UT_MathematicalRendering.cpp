/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "include/MouCaLab.h"

#include "LibVulkan/include/VkBuffer.h"
#include "LibVulkan/include/VKContextDevice.h"
#include "LibVulkan/include/VKSequence.h"

class MathematicalRenderingTest : public MouCaLabTest
{
public:
    void updateUBO(MouCaGraphic::Engine3DXMLLoader& loader, const Vulkan::ContextDevice& context, float time)
    {
        ASSERT_EQ(1, loader._buffers.size());
        auto& memory = loader._buffers[0].lock()->getMemory();
        memory.map(context.getDevice());

        memcpy(memory.getMappedMemory<char>(), &time, sizeof(time));

        memory.unmap(context.getDevice());
    }
};

TEST_F(MathematicalRenderingTest, run)
{
    MouCaGraphic::VulkanManager manager;

    MouCaGraphic::Engine3DXMLLoader loader(manager);
    ASSERT_NO_FATAL_FAILURE(loadEngine(loader, u8"MathematicalRendering.xml"));

    auto context = manager.getDevices().at(0);

    // Set value
    updateUBO(loader, *context, 0.121f);

    // Execute commands
    updateCommandBuffersSurface(loader);

    // Execute rendering
    if (MouCaEnvironment::isDemonstrator())
    {
        std::thread thread([&]()
            {
                float fTime = 0.0;
                while(_graphic.getRTPlatform().isWindowsActive())
                {
                    updateUBO(loader, *context, fTime);
                    fTime += 0.00001f;
                }
            });

        mainLoop(manager, u8"Triangle ScreenSpace Demo ");

        thread.join();
    }
    else
    {
        // Get allocated item
        context->getDevice().waitIdle();

        auto queueSequences = context->getQueueSequences();
        ASSERT_EQ(1, queueSequences.size());

        // Run one frame
        for (const auto& sequence : *queueSequences.at(0))
        {
            ASSERT_EQ(VK_SUCCESS, sequence->execute(context->getDevice()));
        }

        takeScreenshot(manager, L"MathematicalRendering.png");
    }

    // Clean
    ASSERT_NO_THROW(manager.release());

    // Clean allocate dialog
    ASSERT_NO_THROW(clearDialog(manager));
}