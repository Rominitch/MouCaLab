/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include "include/MouCaLab.h"

#include <LibRT/include/RTRenderDialog.h>

#include <LibVulkan/include/VKBuffer.h>
#include <LibVulkan/include/VKContextDevice.h>
#include <LibVulkan/include/VKCommand.h>
#include <LibVulkan/include/VKCommandBuffer.h>
#include <LibVulkan/include/VKSequence.h>
#include <LibVulkan/include/VKWindowSurface.h>

class TriangleTest : public MouCaLabTest
{
    struct SimpleTransfer
    {
        Vulkan::Buffer      transferCPUGPU;

        void prepare(const Vulkan::Device& device, const Vulkan::BufferWPtr dst, const size_t size, const void* data, Vulkan::CommandUPtr&& cmd)
        {
            transferCPUGPU.initialize(device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                size, data);

            const VkBufferCopy copyInfo
            {
                0,
                0,
                size
            };
            cmd = std::make_unique<Vulkan::CommandCopy>(transferCPUGPU, *dst.lock(), copyInfo);
        }

        void release(const Vulkan::Device& device)
        {
            transferCPUGPU.release(device);
        }
    };

    public:
        struct
        {
            glm::mat4 projectionMatrix;
            glm::mat4 modelMatrix;
            glm::mat4 viewMatrix;
        }
        uboVS;

        glm::vec3 rotation;
        float     zoom;

        TriangleTest():
        rotation(0.0f, 0.0f, 0.0f), zoom(-2.5f)
        {}
        
        void transferData(const Vulkan::ContextDeviceSPtr context, const Vulkan::BufferWPtr& vbo, const Vulkan::BufferWPtr& ibo)
        {
            // Synchronize a with device
            context->getDevice().waitIdle();

            // Build descriptor
            RT::BufferDescriptor vboDescriptor;
            {
                const std::vector<RT::ComponentDescriptor> descriptors
                {
                    { 3, RT::Type::Float, RT::ComponentUsage::Vertex },
                    { 3, RT::Type::Float, RT::ComponentUsage::Color }
                };
                vboDescriptor.initialize(descriptors);
            }
            RT::BufferDescriptor iboDescriptor;
            {
                const std::vector<RT::ComponentDescriptor> descriptors
                {
                    { 3, RT::Type::UInt32, RT::ComponentUsage::Index }
                };
                iboDescriptor.initialize(descriptors);
            }

            // Prepare data
            struct Vertex
            {
                float position[3];
                float color[3];
            };
            using Index = uint32_t;

            const float length = 1.0f;
            std::array<Vertex, 3> vertexBuffer
            { {
                { {  length,  length, 0.0f }, { 1.0f, 0.0f, 0.0f } },
                { { -length,  length, 0.0f }, { 0.0f, 1.0f, 0.0f } },
                { {  0.0f,   -length, 0.0f }, { 0.0f, 0.0f, 1.0f } }
            } };

            // Setup indices
            std::array<Index, 3> indexBuffer { { 0, 1, 2 } };

            auto cpuVBO = std::make_shared<RT::BufferLinkedCPU>();
            cpuVBO->create(vboDescriptor, vertexBuffer.size(), vertexBuffer.data());
            auto iboVBO = std::make_shared<RT::BufferLinkedCPU>();
            iboVBO->create(iboDescriptor, indexBuffer.size(), indexBuffer.data());

            // Transfer CPU -> GPU
            {
                auto pool = std::make_shared<Vulkan::CommandPool>();
                pool->initialize(context->getDevice(), context->getDevice().getQueueFamilyGraphicId());

                auto commandBuffer = std::make_shared<Vulkan::CommandBuffer>();
                commandBuffer->initialize(context->getDevice(), pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 0);

                MouCaGraphic::Engine3DTransfer transfer(*context);
                transfer.indirectCopyCPUToGPU(commandBuffer, *cpuVBO, *vbo.lock());
                transfer.indirectCopyCPUToGPU(commandBuffer, *iboVBO, *ibo.lock());

                transfer.transfer(commandBuffer);

                commandBuffer->release(context->getDevice());

                pool->release(context->getDevice());
            }
        }

        void updateUBO(const RT::RenderDialogWPtr dialog, const Vulkan::ContextDeviceSPtr context, const Vulkan::BufferWPtr& ubo)
        {
            const auto& resolution = dialog.lock()->getSize();
            MOUCA_ASSERT(resolution.x > 0 && resolution.y > 0);
            const float ratio = static_cast<float>(resolution.x) / static_cast<float>(resolution.y);

            // Update matrices
            uboVS.projectionMatrix = glm::perspective(glm::radians(60.0f), ratio, 0.1f, 256.0f);

            uboVS.viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, zoom));

            uboVS.modelMatrix = glm::mat4(1.0f);
            uboVS.modelMatrix = glm::rotate(uboVS.modelMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            uboVS.modelMatrix = glm::rotate(uboVS.modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            uboVS.modelMatrix = glm::rotate(uboVS.modelMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

            // Transfer GPU to CPU: unified memory
            auto& memory = ubo.lock()->getMemory();
            memory.map(context->getDevice());

            memcpy(memory.getMappedMemory<char>(), &uboVS, sizeof(uboVS));

            memory.unmap(context->getDevice());
        }
};

TEST_F(TriangleTest, run)
{
    MouCaGraphic::VulkanManager manager;

    MouCaGraphic::Engine3DXMLLoader loader(manager);
    ASSERT_NO_FATAL_FAILURE(loadEngine(loader, u8"Triangle.xml"));
    
    // Get allocated item
    auto context = manager.getDevices().at(0);

    // Transfer data memory to GPU
    transferData(context, loader._buffers[1], loader._buffers[2]);

    // Set 
    updateUBO(manager.getSurfaces().at(0)->_linkWindow, context, loader._buffers[0]);

    // Execute rendering
#ifdef VULKAN_DEMO
    mainLoop(manager, u8"Triangle Demo ");
#else
    context->getDevice().waitIdle();
    auto queueSequences = context->getQueueSequences();
    ASSERT_EQ(1, queueSequences.size());

    // Run one frame
    for (const auto& sequence : *queueSequences.at(0))
    {
        ASSERT_EQ(VK_SUCCESS, sequence->execute(context->getDevice()));
    }
    
    takeScreenshot(manager, L"triangle.png");
#endif

    // Clean
    ASSERT_NO_THROW(manager.release());

    // Clean allocate dialog
    ASSERT_NO_THROW(clearDialog(manager));
}