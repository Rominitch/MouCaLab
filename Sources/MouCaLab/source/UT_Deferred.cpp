/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "include/MouCaLab.h"

#include <MouCaGraphicEngine/include/Engine3DTransfer.h>
#include <MouCaGraphicEngine/include/ImGUIManager.h>

#include <LibRT/include/RTBufferCPU.h>
#include <LibRT/include/RTCamera.h>
#include <LibRT/include/RTCameraComportement.h>
#include <LibRT/include/RTCameraManipulator.h>
#include <LibRT/include/RTLight.h>
#include <LibRT/include/RTMesh.h>
#include <LibRT/include/RTObject.h>

#include <LibVulkan/include/VKContextDevice.h>
#include <LibVulkan/include/VKCommand.h>
#include <LibVulkan/include/VKCommandBuffer.h>
#include <LibVulkan/include/VKFence.h>
#include <LibVulkan/include/VKSequence.h>
#include <LibVulkan/include/VKSubmitInfo.h>
#include <LibVulkan/include/VKWindowSurface.h>

#include "include/EventManager3D.h"

class DeferredTest : public MouCaLabTest
{
    public:
        inline RT::BufferDescriptor getMeshDescriptor()
        {
            const std::vector<RT::ComponentDescriptor> descriptors =
            {
                { 3, RT::Type::Float, RT::ComponentUsage::Vertex    },
                { 2, RT::Type::Float, RT::ComponentUsage::TexCoord0 },
                { 3, RT::Type::Float, RT::ComponentUsage::Color     },
                { 3, RT::Type::Float, RT::ComponentUsage::Normal    },
                { 3, RT::Type::Float, RT::ComponentUsage::Tangent   }
            };
            RT::BufferDescriptor bufferDescriptor;
            bufferDescriptor.initialize(descriptors);
            return bufferDescriptor;
        }
        
        struct Specialization
        {
            int32_t   _sampling;
            float       _ambiant;
        };

        struct Light
        {
            glm::vec4 position;
            glm::vec3 color;
            float radius;
        };

        struct Lighting
        {
            std::array<Light, 6> lights;
            glm::vec4 viewPos;
        };

        struct UBOGeometry
        {
            glm::mat4 projectionMatrix;
            glm::mat4 viewMatrix;
        };

        struct UBOModel
        {
            glm::mat4 modelMatrix[4];
        };

        struct Models
        {
            RT::CameraSPtr             _camera;

            // Meshes
            RT::MeshImportSPtr         _armor;
            RT::MeshImportSPtr         _ground;

            // Textures
            RT::ImageImportSPtr        _armorColor;
            RT::ImageImportSPtr        _armorNormal;
            RT::ImageImportSPtr        _groundColor;
            RT::ImageImportSPtr        _groundNormal;

            // Buffers
            Lighting                   _lighting;
            UBOGeometry                _geometryVS;
            UBOGeometry                _geometryOffscreenVS;
            UBOModel                   _models;

            RT::BufferLinkedCPUSPtr    _uboLighting;
            RT::BufferLinkedCPUSPtr    _uboVS;
            RT::BufferLinkedCPUSPtr    _uboOffscreenVS;           
            RT::BufferLinkedCPUSPtr    _uboModels;

            RT::BufferCPUSPtr          _specialization;
            RT::BufferCPUSPtr          _specializationNoMSAA;
        };

        // CPU Asset
        RT::Array2ui    _resolution;
        Models          _models;    
        RT::ObjectSPtr  _supportCamera;
        float           _animationTime = 0.0f;

        Vulkan::CommandSwitch* _switchScreen;
        Vulkan::CommandSwitch* _switchOffScreeen;

        bool useMSAA;
        bool debugDisplay;
        bool useSampleShading;

        DeferredTest():
        _resolution(1280, 720), useMSAA(true), debugDisplay(false), useSampleShading(true),
        _switchScreen(nullptr), _switchOffScreeen(nullptr)
        {}

        void configureEventManager() override
        {
            MouCaLabTest::configureEventManager();

            // Build camera
            _supportCamera = std::make_shared<RT::Object>();
            _supportCamera->getOrientation().setPosition(glm::vec3(0.0f, 1.0f, 0.0f));

            auto trackball = std::make_shared<RT::CameraManipulator>();
            trackball->initialize(_models._camera);
            trackball->installComportement(RT::CameraManipulator::TrackBallCamera);
            static_cast<RT::CameraTrackBall*>(trackball->getComportement())->moveTo({ 0.3f, 0.3f, 10.0f });

            // Attach support
            static_cast<RT::CameraTrackBall*>(trackball->getComportement())->attachSupport(_supportCamera);

            auto flying = std::make_shared<RT::CameraManipulator>();
            flying->initialize(_models._camera);
            flying->installComportement(RT::CameraManipulator::FlyingCamera);

            // Register into event Manager
            ASSERT_NO_THROW(dynamic_cast<EventManager3D*>(_eventManager.get())->addManipulator(trackball));
            ASSERT_NO_THROW(dynamic_cast<EventManager3D*>(_eventManager.get())->addManipulator(flying));
        }

        void makeScene()
        {
            _models._camera = std::make_shared<RT::Camera>();

            // Configure camera
            _models._camera->getOrientation().setPosition( { -2.15f, -0.3f, 8.75f });
            _models._camera->computePerspectiveCamera(RT::ViewportInt32(0,0, _resolution.x, _resolution.y));

            configureEventManager();

            // Get manager and path
            auto& resources = _core.getResourceManager();
            auto& loader    = _core.getLoaderManager();

            const auto& meshPath    = resources.getResourceFolder(MouCaCore::ResourceManager::Meshes);
            const auto& texturePath = resources.getResourceFolder(MouCaCore::ResourceManager::Textures);

            // Prepare resources images/mesh
            const auto armorPath = meshPath / L"armor";
            _models._armorColor  = resources.openImage(armorPath / L"colormap.ktx");
            _models._armorNormal = resources.openImage(armorPath / L"normalmap.ktx");
            _models._armor       = resources.openMeshImport(armorPath / L"armor.dae", getMeshDescriptor(), RT::MeshImport::ComputeAll);

            _models._groundColor  = resources.openImage(texturePath / L"stonefloor02_color_bc3_unorm.ktx");
            _models._groundNormal = resources.openImage(texturePath / L"stonefloor02_normal_bc3_unorm.ktx");
            _models._ground       = resources.openMeshImport(meshPath / L"openbox.dae", getMeshDescriptor(), RT::MeshImport::ComputeAll);
            
            // Demand loading (deferred => thread loading)
            MouCaCore::LoadingItems items =
            {
                MouCaCore::LoadingItem(_models._armorColor,  MouCaCore::LoadingItem::Deferred),
                MouCaCore::LoadingItem(_models._armorNormal, MouCaCore::LoadingItem::Deferred),
                MouCaCore::LoadingItem(_models._armor,       MouCaCore::LoadingItem::Deferred),

                MouCaCore::LoadingItem(_models._groundColor,  MouCaCore::LoadingItem::Deferred),
                MouCaCore::LoadingItem(_models._groundNormal, MouCaCore::LoadingItem::Deferred),
                MouCaCore::LoadingItem(_models._ground,       MouCaCore::LoadingItem::Deferred)
            };
            loader.loadResources(items);

            // Wait all resources are loaded
            ASSERT_NO_THROW(loader.synchronize());

            // Make linked buffer (no memory management)
            _models._uboVS          = std::make_shared<RT::BufferLinkedCPU>();
            _models._uboVS->create(RT::BufferDescriptor(sizeof(UBOGeometry)), 1, &_models._geometryVS);
            _models._uboOffscreenVS = std::make_shared<RT::BufferLinkedCPU>();
            _models._uboOffscreenVS->create(RT::BufferDescriptor(sizeof(UBOGeometry)), 1, &_models._geometryOffscreenVS);
            _models._uboLighting    = std::make_shared<RT::BufferLinkedCPU>();
            _models._uboLighting->create(RT::BufferDescriptor(sizeof(Lighting)), 1, &_models._lighting);
            _models._uboModels      = std::make_shared<RT::BufferLinkedCPU>();
            _models._uboModels->create(RT::BufferDescriptor(sizeof(UBOModel)), 1, &_models._models);

            // Specialization for shader
            {
                _models._specialization = std::make_shared<RT::BufferCPU>();
                Specialization* buffer = reinterpret_cast<Specialization*>(_models._specialization->create(RT::BufferDescriptor(sizeof(Specialization)), 1));
                buffer->_sampling = VK_SAMPLE_COUNT_8_BIT;
                buffer->_ambiant = 0.15f;
            }
            {
                _models._specializationNoMSAA = std::make_shared<RT::BufferCPU>();
                Specialization* buffer = reinterpret_cast<Specialization*>(_models._specializationNoMSAA->create(RT::BufferDescriptor(sizeof(Specialization)), 1));
                buffer->_sampling = VK_SAMPLE_COUNT_1_BIT;
                buffer->_ambiant  = 0.15f;
            }
        }

        void releaseScene()
        {
            auto& resources = _core.getResourceManager();

            resources.releaseResource(std::move(_models._armorColor));
            resources.releaseResource(std::move(_models._armorNormal));
            resources.releaseResource(std::move(_models._armor));

            resources.releaseResource(std::move(_models._groundColor));
            resources.releaseResource(std::move(_models._groundNormal));
            resources.releaseResource(std::move(_models._ground));

            _models._uboVS->release();
            _models._uboVS.reset();
            _models._uboOffscreenVS->release();
            _models._uboOffscreenVS.reset();
            _models._uboLighting->release();
            _models._uboLighting.reset();
            _models._uboModels->release();
            _models._uboModels.reset();
        }

        void updateUniformBuffersScreen()
        {
            if (debugDisplay)
            {
                _models._geometryVS.projectionMatrix = glm::ortho(0.0f, 2.0f, 0.0f, 2.0f, -1.0f, 1.0f);
            }
            else
            {
                _models._geometryVS.projectionMatrix = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);
            }
        }

        void updateUniformBufferDeferredMatrices()
        {
            _models._geometryOffscreenVS.projectionMatrix = _models._camera->getProjectionMatrix();
            _models._geometryOffscreenVS.viewMatrix       = _models._camera->getOrientation().getWorldToLocal().convert();
            
            _models._models.modelMatrix[0] = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(15.0f)), glm::vec3(0.0f, -2.35f/15.0f, 0.0f));
            _models._models.modelMatrix[1] = glm::mat4(1.0f);
            _models._models.modelMatrix[2] = glm::translate(glm::mat4(1.0f), glm::vec3(-4.0f, 0.0, -4.0f));
            _models._models.modelMatrix[3] = glm::translate(glm::mat4(1.0f), glm::vec3(4.0f, 0.0, -4.0f));
        }

        // Update fragment shader light position uniform block
        void updateUniformBufferDeferredLights(const float timer)
        {
            // White
            _models._lighting.lights[0].position = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
            _models._lighting.lights[0].color = glm::vec3(1.5f);
            _models._lighting.lights[0].radius = 15.0f * 0.25f;
            // Red
            _models._lighting.lights[1].position = glm::vec4(-2.0f, 0.0f, 0.0f, 0.0f);
            _models._lighting.lights[1].color = glm::vec3(1.0f, 0.0f, 0.0f);
            _models._lighting.lights[1].radius = 15.0f;
            // Blue
            _models._lighting.lights[2].position = glm::vec4(2.0f, 1.0f, 0.0f, 0.0f);
            _models._lighting.lights[2].color = glm::vec3(0.0f, 0.0f, 2.5f);
            _models._lighting.lights[2].radius = 5.0f;
            // Yellow
            _models._lighting.lights[3].position = glm::vec4(0.0f, 0.9f, 0.5f, 0.0f);
            _models._lighting.lights[3].color = glm::vec3(1.0f, 1.0f, 0.0f);
            _models._lighting.lights[3].radius = 2.0f;
            // Green
            _models._lighting.lights[4].position = glm::vec4(0.0f, 0.5f, 0.0f, 0.0f);
            _models._lighting.lights[4].color = glm::vec3(0.0f, 1.0f, 0.2f);
            _models._lighting.lights[4].radius = 5.0f;
            // Yellow
            _models._lighting.lights[5].position = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
            _models._lighting.lights[5].color = glm::vec3(1.0f, 0.7f, 0.3f);
            _models._lighting.lights[5].radius = 25.0f;

            _models._lighting.lights[0].position.x = sin(glm::radians(360.0f * timer)) * 5.0f;
            _models._lighting.lights[0].position.z = cos(glm::radians(360.0f * timer)) * 5.0f;

            _models._lighting.lights[1].position.x = -4.0f + sin(glm::radians(360.0f * timer) + 45.0f) * 2.0f;
            _models._lighting.lights[1].position.z = 0.0f + cos(glm::radians(360.0f * timer) + 45.0f) * 2.0f;

            _models._lighting.lights[2].position.x = 4.0f + sin(glm::radians(360.0f * timer)) * 2.0f;
            _models._lighting.lights[2].position.z = 0.0f + cos(glm::radians(360.0f * timer)) * 2.0f;

            _models._lighting.lights[4].position.x = 0.0f + sin(glm::radians(360.0f * timer + 90.0f)) * 5.0f;
            _models._lighting.lights[4].position.z = 0.0f - cos(glm::radians(360.0f * timer + 45.0f)) * 5.0f;

            _models._lighting.lights[5].position.x = 0.0f + sin(glm::radians(-360.0f * timer + 135.0f)) * 10.0f;
            _models._lighting.lights[5].position.z = 0.0f - cos(glm::radians(-360.0f * timer - 45.0f)) * 10.0f;

            // Current view position
            _models._lighting.viewPos = glm::vec4(_models._camera->getOrientation().getPosition(), 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);
        }

        bool updateUIOverlay()
        {
            bool update = false;
            
            ImGui::NewFrame();

            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
            ImGui::SetNextWindowPos(ImVec2(10, 10));
            
            ImGuiIO& io = ImGui::GetIO();
            ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_FirstUseEver);
            ImGui::Begin(u8"Vulkan Example", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
            //ImGui::TextUnformatted(title.c_str());
            //ImGui::TextUnformatted(deviceProperties.deviceName);
            //ImGui::Text("%.2f ms/frame (%.1d fps)", (1000.0f / lastFPS), lastFPS);

            ImGui::PushItemWidth(110.0f * 1.0f);

            if (ImGui::CollapsingHeader(u8"Settings", ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (ImGui::Checkbox(u8"Display render targets", &debugDisplay))
                {
                    update = true;

                    _switchScreen->setIdNode(debugDisplay ? 2 : (useMSAA ? 0 : 1));
                }
                if (ImGui::Checkbox(u8"MSAA",  &useMSAA))
                {
                    update = true;

                    _switchScreen->setIdNode(debugDisplay ? 2 : (useMSAA ? 0 : 1));
                }
                if (ImGui::Checkbox(u8"Sample rate shading", &useSampleShading))
                {
                    update = true;
                    _switchScreen->setIdNode(useSampleShading ? 0 : 1);
                }
            }

            ImGui::PopItemWidth();

            ImGui::End();
            ImGui::PopStyleVar();

            ImGui::Render();

            return update;
        }
};

TEST_F(DeferredTest, run)
{
    MouCaGraphic::VulkanManager manager;
    MouCaGraphic::ImGUIManager  GUI;

//-------------------------------------------------------------------------------------------------
//                                         Step 1: Build CPU scene
//-------------------------------------------------------------------------------------------------
    makeScene();

//-------------------------------------------------------------------------------------------------
//                                         Step 2: Build GPU scene
//-------------------------------------------------------------------------------------------------
    MouCaGraphic::Engine3DXMLLoader loader(manager);
    // Register CPU memory
    loader._cpuBuffers[0]  = _models._armor->getMesh().getIBOBuffer();
    loader._cpuBuffers[1]  = _models._armor->getMesh().getVBOBuffer();
    loader._cpuBuffers[2]  = _models._ground->getMesh().getIBOBuffer();
    loader._cpuBuffers[3]  = _models._ground->getMesh().getVBOBuffer();
                           
    loader._cpuBuffers[6]  = _models._uboVS;
    loader._cpuBuffers[7]  = _models._uboOffscreenVS;
    loader._cpuBuffers[8]  = _models._uboLighting;
    loader._cpuBuffers[9]  = _models._uboModels;
    loader._cpuBuffers[10] = _models._specialization;
    loader._cpuBuffers[11] = _models._specializationNoMSAA;

    loader._cpuImages[5] = _models._armorColor->getImage();
    loader._cpuImages[6] = _models._armorNormal->getImage();
    loader._cpuImages[7] = _models._groundColor->getImage();
    loader._cpuImages[8] = _models._groundNormal->getImage();

    loader._cpuMeshDescriptors[0] = getMeshDescriptor();

    // Create Vulkan objects
    ASSERT_NO_FATAL_FAILURE(loadEngine(loader, u8"Deferred.xml"));

    // Link event manager
    loader._dialogs[0].lock()->initialize(_eventManager, _resolution);

    _switchScreen     = dynamic_cast<Vulkan::CommandSwitch*>(loader._commandLinks[1]);
    MouCa::assertion(_switchScreen != nullptr);
    _switchOffScreeen = dynamic_cast<Vulkan::CommandSwitch*>(loader._commandLinks[2]);
    MouCa::assertion(_switchOffScreeen != nullptr);

    ASSERT_NO_FATAL_FAILURE(GUI.initialize(_resolution));

    MouCaGraphic::Engine3DXMLLoader loaderGUI(manager);
    // Transfer Device/Framebuffer/RenderPass from first model
    loaderGUI._devices[0]            = loader._devices[0];
    loaderGUI._frameBuffers[0]       = loader._frameBuffers[0];
    loaderGUI._renderPasses[0]       = loader._renderPasses[0];

    // Register CPU memory
    loaderGUI._cpuMeshDescriptors[0] = GUI.getMeshDescriptor();
    loaderGUI._cpuImages[0]          = GUI.getImageFont();

    loaderGUI._cpuBuffers[0] = GUI.getPush();

    loaderGUI._buffers[0] = GUI.getVertexBuffer();
    loaderGUI._buffers[1] = GUI.getIndexBuffer();

    // Create Vulkan objects
    ASSERT_NO_FATAL_FAILURE(loadEngine(loaderGUI, u8"ImGUI.xml"));

    // Transfer commands
    {
        Vulkan::CommandContainer* container = dynamic_cast<Vulkan::CommandContainer*>(loader._commandLinks[0]);
        MouCa::assertion(container != nullptr);
        container->transfer(std::move(*loaderGUI._commandsGroup[0].get()));
    }

//-------------------------------------------------------------------------------------------------
//                                      Step 3: Transfer CPU -> GPU
//-------------------------------------------------------------------------------------------------
    // Get allocated item
    auto context = manager.getDevices().at(0);

    // Transfers data memory to GPU
    {
        auto pool = std::make_shared<Vulkan::CommandPool>();
        pool->initialize(context->getDevice(), context->getDevice().getQueueFamilyGraphicId());

        Vulkan::CommandBufferSPtr commandBuffer = std::make_shared<Vulkan::CommandBuffer>();
        commandBuffer->initialize(context->getDevice(), pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 0);

        MouCaGraphic::Engine3DTransfer transfer(*context);
        // Buffers
        transfer.indirectCopyCPUToGPU(commandBuffer, *loader._cpuBuffers[0].lock(), *loader._buffers[0].lock());
        transfer.indirectCopyCPUToGPU(commandBuffer, *loader._cpuBuffers[1].lock(), *loader._buffers[1].lock());
        transfer.indirectCopyCPUToGPU(commandBuffer, *loader._cpuBuffers[2].lock(), *loader._buffers[2].lock());
        transfer.indirectCopyCPUToGPU(commandBuffer, *loader._cpuBuffers[3].lock(), *loader._buffers[3].lock());
        // Images
        transfer.indirectCopyCPUToGPU(commandBuffer, *loader._cpuImages[5].lock(), *loader._images[5].lock());
        transfer.indirectCopyCPUToGPU(commandBuffer, *loader._cpuImages[6].lock(), *loader._images[6].lock());
        transfer.indirectCopyCPUToGPU(commandBuffer, *loader._cpuImages[7].lock(), *loader._images[7].lock());
        transfer.indirectCopyCPUToGPU(commandBuffer, *loader._cpuImages[8].lock(), *loader._images[8].lock());
        // GUI
        transfer.indirectCopyCPUToGPU(commandBuffer, *loaderGUI._cpuImages[0].lock(), *loaderGUI._images[0].lock());

        // Go
        transfer.transfer(commandBuffer);

        commandBuffer->release(context->getDevice());
        pool->release(context->getDevice());
    }

    // Update info
    updateUniformBuffersScreen();
    updateUniformBufferDeferredMatrices();
    updateUniformBufferDeferredLights(0);

    // Transfer data memory to GPU
    {
        MouCaGraphic::Engine3DTransfer transfer(*context);

        transfer.immediatCopyCPUToGPU(*_models._uboVS,           *loader._buffers[6].lock());
        transfer.immediatCopyCPUToGPU(*_models._uboOffscreenVS,  *loader._buffers[7].lock());
        transfer.immediatCopyCPUToGPU(*_models._uboLighting,     *loader._buffers[8].lock());
        transfer.immediatCopyCPUToGPU(*_models._uboModels,       *loader._buffers[9].lock());
    }

//-------------------------------------------------------------------------------------------------
//                                             Step 4: Play
//-------------------------------------------------------------------------------------------------
    // Execute rendering
    if (MouCaEnvironment::isDemonstrator())
    {
        // DisableCodeCoverage

        updateUIOverlay();
        bool needUpdateGUI = true;

        updateCommandBuffers(loader);
        updateCommandBuffersSurface(loader);

        /// Update Light position / camera
        auto demo = [&](const double time)
        {
            // Move camera and light
            _models._geometryOffscreenVS.viewMatrix = _models._camera->getOrientation().getWorldToLocal().convert();
            updateUniformBufferDeferredLights((float)time);
            {
                MouCaGraphic::Engine3DTransfer transfer(*context);
                transfer.immediatCopyCPUToGPU(*_models._uboVS, *loader._buffers[6].lock());
                transfer.immediatCopyCPUToGPU(*_models._uboOffscreenVS, *loader._buffers[7].lock());
                transfer.immediatCopyCPUToGPU(*_models._uboLighting, *loader._buffers[8].lock());
            }

            // Update mouse position into ImGUI
            GUI.updateMouse(_graphic.getRTPlatform().getMouse());
            
            if ( updateUIOverlay() || needUpdateGUI )
            {
                needUpdateGUI = false;
                ASSERT_NO_THROW(GUI.prepareBuffer(*context));
                Vulkan::CommandContainer* container = dynamic_cast<Vulkan::CommandContainer*>(loaderGUI._commandLinks[0]);
                ASSERT_TRUE(container != nullptr);
                ASSERT_NO_THROW(GUI.buildCommands(container->getCommands()));

                // Refresh Command Buffer with new command
                updateCommandBuffers(loader);
                updateCommandBuffersSurface(loader);
            }
        };
        mainLoop(manager, u8"Deferred Demo ", demo);     

        // EnableCodeCoverage
    }
    else
    {
        updateUIOverlay();
        // Build GUI design
        updateUIOverlay();
        ASSERT_NO_THROW(GUI.prepareBuffer(*context));
        Vulkan::CommandContainer* container = dynamic_cast<Vulkan::CommandContainer*>(loaderGUI._commandLinks[0]);
        MouCa::assertion(container != nullptr);
        ASSERT_NO_THROW(GUI.buildCommands(container->getCommands()));

        // Refresh Command Buffer with new command
        updateCommandBuffers(loader);
        updateCommandBuffersSurface(loader);

        // Get Sequencer
        context->getDevice().waitIdle();
        auto queueSequences = context->getQueueSequences();
        ASSERT_EQ(1, queueSequences.size());

        // Run one frame
        for (const auto& sequence : *queueSequences.at(0))
        {
            ASSERT_EQ(VK_SUCCESS, sequence->execute(context->getDevice()));
        }

        // Compare to expected result
        takeScreenshot(manager, L"deferred.png");
    }

    // Clean
    ASSERT_NO_THROW(GUI.release(*context));
    ASSERT_NO_THROW(manager.release());
    ASSERT_NO_THROW(releaseScene());

    // Clean allocate dialog
    ASSERT_NO_THROW(clearDialog(manager));
    ASSERT_NO_THROW(releaseEventManager());
}