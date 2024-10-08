/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "include/MouCaLab.h"

#include "LibVulkan/include/VKContextDevice.h"
#include "LibVulkan/include/VKCommand.h"
#include "LibVulkan/include/VKCommandBuffer.h"
#include "LibVulkan/include/VKFence.h"
#include "LibVulkan/include/VKSequence.h"
#include "LibVulkan/include/VKSubmitInfo.h"
#include "LibVulkan/include/VKWindowSurface.h"

namespace MouCa3DEngine
{

class TriangleScreenSpaceTest : public MouCaLabTest
{};

TEST_F(TriangleScreenSpaceTest, run)
{
    MouCaGraphic::VulkanManager manager;

    MouCaGraphic::Engine3DXMLLoader loader(manager);
    ASSERT_NO_FATAL_FAILURE(loadEngine(loader, "TriangleScreenSpace.xml"));

    // Execute commands
    updateCommandBuffersSurface(loader);

    // Execute rendering
    if (MouCaEnvironment::isDemonstrator())
    {
        mainLoop(manager, "Triangle ScreenSpace Demo ");
    }
    else
    {
        // Get allocated item
        auto context = manager.getDevices().at(0);
        context->getDevice().waitIdle();

        auto queueSequences = context->getQueueSequences();
        ASSERT_EQ(1, queueSequences.size());

        // Run one frame
        for (const auto& sequence : *queueSequences.at(0))
        {
            ASSERT_EQ(VK_SUCCESS, sequence->execute(context->getDevice()));
        }

        takeScreenshot(manager, "triangleScreenSpace.png");
    }

    // Clean
    ASSERT_NO_THROW(manager.release());

    // Clean allocate dialog
    ASSERT_NO_THROW(clearDialog(manager));
}

}