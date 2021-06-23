/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "include/MouCaLab.h"

#include "LibRT/include/RTBufferCPU.h"
#include "LibRT/include/RTCamera.h"
#include "LibRT/include/RTCameraComportement.h"
#include "LibRT/include/RTCameraManipulator.h"
#include "LibRT/include/RTFrustum.h"
#include "LibRT/include/RTLight.h"
#include "LibRT/include/RTMesh.h"

#include "LibVulkan/include/VKContextDevice.h"
#include "LibVulkan/include/VKCommand.h"
#include "LibVulkan/include/VKCommandBuffer.h"
#include "LibVulkan/include/VKSequence.h"
#include "LibVulkan/include/VKSubmitInfo.h"
#include "LibVulkan/include/VKWindowSurface.h"

class TessellationTest : public MouCaLabTest
{
public:
    inline RT::BufferDescriptor getMeshDescriptor()
    {
        const std::vector<RT::ComponentDescriptor> descriptors =
        {
            { 3, RT::Type::Float, RT::ComponentUsage::Vertex    },
            { 3, RT::Type::Float, RT::ComponentUsage::Normal    },
            { 2, RT::Type::Float, RT::ComponentUsage::TexCoord0 }
        };
        RT::BufferDescriptor bufferDescriptor;
        bufferDescriptor.initialize(descriptors);
        return bufferDescriptor;
    }

    struct UBOScreen
    {
        glm::mat4 projectionMatrix;
        glm::mat4 viewMatrix;
    };

    static const size_t _nbFrustrumPlanes = 6;
    struct UBOCamera
    {
        glm::mat4 projectionMatrix;
        glm::mat4 modelMatrix;
        glm::mat4 viewMatrix;        
        std::array<glm::vec4, _nbFrustrumPlanes> frustumPlanes;
        glm::vec2 viewportDim;

        UBOCamera():
        modelMatrix(1.0f), viewMatrix(1.0f)
        {}
    };

    struct UBOTessellation
    {
        float displacementFactor  = 32.0f;
        float tessellationFactor  = 0.75f;
        float tessellatedEdgeSize = 20.0f;
    };

    struct Specialization
    {
        int32_t   _sampling;
        float       _ambiant;
    };

    struct Light
    {
        glm::vec4   position;
        glm::vec3   color;
        float       radius;
    };

    static const size_t _nbLights = 6;
    struct Lighting
    {
        std::array<Light, _nbLights> lights;
        glm::vec4 viewPos;
    };

    struct Models
    {
        RT::CameraSPtr             _camera;

        // Textures
        RT::ImageImportSPtr        _imageHeight;
        RT::ImageImportSPtr        _imageTerrain;

        RT::Mesh                   _terrain;
        Lighting                   _lighting;

        UBOScreen                  _screenParameters;
        UBOCamera                  _cameraParameters;
        UBOTessellation            _tessellationParameters;

        RT::BufferLinkedCPUSPtr    _uboLighting;
        RT::BufferLinkedCPUSPtr    _uboScreenVS;
        RT::BufferLinkedCPUSPtr    _uboCameraVS;
        RT::BufferLinkedCPUSPtr    _uboTessellation;

        RT::BufferCPUSPtr          _specialization;
    };

    // CPU Asset
    RT::Array2ui    _resolution;
    Models          _models;
    RT::ObjectSPtr  _supportCamera;

    // Interaction
    float           _tessellationFactor;
    bool            _tessellation;
    bool            _wireframe;
    Vulkan::CommandSwitch* _switchWireframe;

    static const uint32_t   PATCH_SIZE;
    static const float        UV_SCALE;

    TessellationTest():
    _resolution(1280, 720), _tessellationFactor(_models._tessellationParameters.tessellationFactor), _tessellation(true), _wireframe(false)
    {
        _models._cameraParameters.viewportDim = glm::vec2(_resolution.x, _resolution.y);
    }

    void configureEventManager() override
    {
        MouCaLabTest::configureEventManager();

        // Build camera
        _supportCamera = std::make_shared<RT::Object>();
        _supportCamera->getOrientation().setPosition(glm::vec3(0.0f, 1.0f, 0.0f));

        auto trackball = std::make_shared<RT::CameraManipulator>();
        trackball->initialize(_models._camera);
        trackball->installComportement(RT::CameraManipulator::TrackBallCamera);
        static_cast<RT::CameraTrackBall*>(trackball->getComportement())->moveTo({ 1.3f, 1.3f, 15.0f });

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
        _models._camera->getOrientation().setPosition({ -2.15f, -0.3f, 8.75f });
        _models._camera->computePerspectiveCamera(RT::ViewportInt32(0, 0, _resolution.x, _resolution.y));

        configureEventManager();

        // Get manager and path
        auto& resources = _core.getResourceManager();
        auto& loader    = _core.getLoaderManager();

        const auto texturePath = resources.getResourceFolder(MouCaCore::ResourceManager::Textures);
        _models._imageHeight = resources.openImage(texturePath / L"terrain_heightmap_r16.ktx");
#ifdef ASTC
        //_textureCompressed = VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
        _imageTerrain = resources.openImage(texturePath / L"terrain_texturearray_astc_8x8_unorm.ktx");
#else
        //_textureCompressed = VK_FORMAT_BC3_UNORM_BLOCK;
        _models._imageTerrain = resources.openImage(texturePath / L"terrain_texturearray_bc3_unorm.ktx");
#endif

        // Try to load resource
        MouCaCore::LoadingItems items =
        {
            MouCaCore::LoadingItem(_models._imageHeight),
            MouCaCore::LoadingItem(_models._imageTerrain, MouCaCore::LoadingItem::Deferred)
        };
        ASSERT_NO_THROW(loader.loadResources(items));

        // Load CPU data
        ASSERT_TRUE(_models._terrain.isNull());
        generateTerrain();
        ASSERT_FALSE(_models._terrain.isNull());

        makeLight();

        // Wait loading resource
        ASSERT_NO_THROW(loader.synchronize());

        // Make linked buffer (no memory management)
        _models._uboScreenVS = std::make_shared<RT::BufferLinkedCPU>();
        _models._uboScreenVS->create(RT::BufferDescriptor(sizeof(UBOScreen)), 1, &_models._screenParameters);
        _models._uboCameraVS = std::make_shared<RT::BufferLinkedCPU>();
        _models._uboCameraVS->create(RT::BufferDescriptor(sizeof(UBOCamera)), 1, &_models._cameraParameters);
        _models._uboLighting = std::make_shared<RT::BufferLinkedCPU>();
        _models._uboLighting->create(RT::BufferDescriptor(sizeof(Lighting)),  1, &_models._lighting);
        _models._uboTessellation = std::make_shared<RT::BufferLinkedCPU>();
        _models._uboTessellation->create(RT::BufferDescriptor(sizeof(UBOTessellation)), 1, &_models._tessellationParameters);

        // Specialization for shader
        {
            _models._specialization = std::make_shared<RT::BufferCPU>();
            Specialization* buffer = reinterpret_cast<Specialization*>(_models._specialization->create(RT::BufferDescriptor(sizeof(Specialization)), 1));
            ASSERT_FALSE(_models._specialization->isNull());
            buffer->_sampling = VK_SAMPLE_COUNT_8_BIT;
            buffer->_ambiant  = 0.65f;
        }
    }

    void updateScreenUBO()
    {
        _models._screenParameters.projectionMatrix = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);
    }

    void updateCameraUBO()
    {
        _models._cameraParameters.projectionMatrix = _models._camera->getProjectionMatrix();
        _models._cameraParameters.viewMatrix       = _models._camera->getOrientation().getWorldToLocal().convert();

        RT::Frustum frustum;
        frustum.update(_models._cameraParameters.projectionMatrix * _models._cameraParameters.viewMatrix);
        memcpy(_models._cameraParameters.frustumPlanes.data(), frustum.planes.data(), sizeof(glm::vec4) * _nbFrustrumPlanes);

        // Current view position
        _models._lighting.viewPos = glm::vec4(_models._camera->getOrientation().getPosition(), 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);
        //_models._lighting.viewPos = glm::vec4(_models._camera->getOrientation().getWorldToLocal()._position, 0.0f);
    }

    void generateTerrain()
    {
        MOUCA_PRE_CONDITION(!_models._imageHeight->isNull());
        // Define struct to manipulate vertices (WARNING: must be similar to descriptor)
        struct Vertex
        {
            glm::vec3 pos;
            glm::vec3 normal;
            glm::vec2 uv;
        };
        const RT::BufferDescriptor descriptor = getMeshDescriptor();
        ASSERT_EQ(sizeof(Vertex), descriptor.getByteSize()); //CRASH if not perfectly equal !
        
        const size_t nbVertices = PATCH_SIZE * PATCH_SIZE;
        RT::BufferCPUSPtr bufVertices = std::make_shared<RT::BufferCPU>();
        bufVertices->create(descriptor, nbVertices);
        auto* vertices = reinterpret_cast<Vertex*>(bufVertices->lock());

        const float wx = 2.0f;
        const float wy = 2.0f;

        RT::BoundingBox bbox;

        for (uint32_t x = 0; x < PATCH_SIZE; x++)
        {
            for (uint32_t y = 0; y < PATCH_SIZE; y++)
            {
                uint32_t index = (x + y * PATCH_SIZE);
                vertices[index].pos[0] = x * wx + wx / 2.0f - (float)PATCH_SIZE * wx / 2.0f;
                vertices[index].pos[1] = 0.0f;
                vertices[index].pos[2] = y * wy + wy / 2.0f - (float)PATCH_SIZE * wy / 2.0f;
                vertices[index].uv = glm::vec2((float)x / PATCH_SIZE, (float)y / PATCH_SIZE) * UV_SCALE;

                bbox.expand(1, &vertices[index].pos);
            }
        }

        {
            auto imageHeight = _models._imageHeight->getImage().lock();

            const size_t dim      = static_cast<size_t>(imageHeight->getExtents(0).x);
            const auto* heightdata = reinterpret_cast<const uint16_t*>(imageHeight->getRAWData(0, 0));

            const auto scale = static_cast<uint32_t>(dim / PATCH_SIZE);

            auto getHeight = [&](uint32_t x, uint32_t y) -> float
            {
                glm::ivec2 rpos = glm::ivec2(x, y) * glm::ivec2(scale);
                rpos.x = std::max(0, std::min(rpos.x, (int)dim - 1));
                rpos.y = std::max(0, std::min(rpos.y, (int)dim - 1));
                rpos /= glm::ivec2(scale);

                const size_t idMem = (rpos.x + rpos.y * dim) * scale;
                MOUCA_ASSERT_BETWEEN(idMem, 0, dim * dim);
                return *(heightdata + idMem) / 65535.0f;
            };

            // Calculate normals from height map using a sobel filter
            for (uint32_t x = 0; x < PATCH_SIZE; x++)
            {
                for (uint32_t y = 0; y < PATCH_SIZE; y++)
                {
                    // Get height samples centered around current position
                    float heights[3][3];
                    for (auto hx = -1; hx <= 1; hx++)
                    {
                        for (auto hy = -1; hy <= 1; hy++)
                        {
                            heights[hx + 1][hy + 1] = getHeight(x + hx, y + hy);
                        }
                    }

                    // Calculate the normal
                    glm::vec3 normal;
                    // Gx sobel filter
                    normal.x = heights[0][0] - heights[2][0] + 2.0f * heights[0][1] - 2.0f * heights[2][1] + heights[0][2] - heights[2][2];
                    // Gy sobel filter
                    normal.z = heights[0][0] + 2.0f * heights[1][0] + heights[2][0] - heights[0][2] - 2.0f * heights[1][2] - heights[2][2];
                    // Calculate missing up component of the normal using the filtered x and y axis
                    // The first value controls the bump strength
                    normal.y = 0.25f * sqrt(1.0f - normal.x * normal.x - normal.z * normal.z);

                    uint32_t index = (x + y * PATCH_SIZE);
                    vertices[index].normal = glm::normalize(normal * glm::vec3(2.0f, 1.0f, 2.0f));
                }
            }
        }

        // Indices
        RT::BufferDescriptor iboDescriptor;
        iboDescriptor.addDescriptor(RT::ComponentDescriptor(4, RT::Type::UnsignedInt, RT::ComponentUsage::Index));

        const uint32_t w = (PATCH_SIZE - 1);
        RT::BufferCPUSPtr bufIndices = std::make_shared<RT::BufferCPU>();
        const size_t nbIndices = w * w;
        bufIndices->create(iboDescriptor, nbIndices);
        auto* indices = reinterpret_cast<uint32_t*>(bufIndices->lock());

        for(uint32_t x = 0; x < w; x++)
        {
            for(uint32_t y = 0; y < w; y++)
            {
                const auto index = (x + y * w) * 4;
                indices[index] = (x + y * PATCH_SIZE);
                indices[index + 1] = indices[index] + PATCH_SIZE;
                indices[index + 2] = indices[index + 1] + 1;
                indices[index + 3] = indices[index] + 1;
            }
        }

        RT::Mesh::SubMeshDescriptors info = 
        {
            {
                nbVertices, // size_t _nbVertices  = 0;
                0,          // size_t _startIndex  = 0;
                nbIndices   // size_t _nbIndices   = 0;
            }
        };

        _models._terrain.initialize(bufVertices, bufIndices, info, RT::FaceOrder::CounterClockWise, RT::Topology::Patches, bbox);
    }

    void makeLight()
    {
        // Prepare scene: Light
        _models._lighting.lights =
        { {
            { { 0.0f,  -19.0f, 0.0f, 1.0f},  {1.0f, 1.0f, 1.0f}, 8.0f},  // White
            { {-20.0f, -20.0f, 9.0f, 1.0f},  {0.7f, 0.0f, 0.0f}, 20.0f}, // Red
            { { 10.0f, -21.0f, 0.0f, 1.0f},  {0.0f, 0.0f, 0.8f}, 10.0f}, // Blue
            { { 0.0f,  -20.9f, 5.0f, 1.0f},  {1.0f, 1.0f, 0.0f}, 2.0f},  // Yellow
            { { 0.0f,  -20.5f, 10.0f, 1.0f}, {0.0f, 1.0f, 0.2f}, 14.0f}, // Green
            { {-20.0f, -21.0f,-5.0f, 1.0f},  {1.0f, 0.7f, 0.3f}, 50.0f}  // Yellow
        } };
    }

    void releaseScene()
    {
        ASSERT_NO_THROW(_models._terrain.release());

        // Release resources
        auto& resources = _core.getResourceManager();
        ASSERT_NO_THROW(resources.releaseResource(std::move(_models._imageHeight)));
        ASSERT_NO_THROW(resources.releaseResource(std::move(_models._imageTerrain)));

        // Clean buffers
        _models._uboScreenVS->release();
        _models._uboScreenVS.reset();
        _models._uboCameraVS->release();
        _models._uboCameraVS.reset();
        _models._uboLighting->release();
        _models._uboLighting.reset();
    }

    bool updateUIOverlay()
    {
        bool update = false;

        ImGui::NewFrame();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
        ImGui::SetNextWindowPos(ImVec2(10, 10));

        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_FirstUseEver);
        ImGui::Begin(u8"Dynamic terrain tessellation", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        //ImGui::TextUnformatted(title.c_str());
        //ImGui::TextUnformatted(deviceProperties.deviceName);
        //ImGui::Text("%.2f ms/frame (%.1d fps)", (1000.0f / lastFPS), lastFPS);

        ImGui::PushItemWidth(110.0f * 1.0f);

        if (ImGui::CollapsingHeader(u8"Settings", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Checkbox(u8"Tessellation", &_tessellation))
            {
                update = true;
            }
            if (ImGui::InputFloat(u8"Factor", &_tessellationFactor, 0.01f, 0.05f, "%.2f"))
            {
                update = true;
            }
            if (ImGui::InputFloat(u8"Displacement", &_models._tessellationParameters.displacementFactor, 0.01f, 0.05f, "%.2f"))
            {
                update = true;
            }
            if (ImGui::InputFloat(u8"Edge size", &_models._tessellationParameters.tessellatedEdgeSize, 0.01f, 0.05f, "%.2f"))
            {
                update = true;
            }
            if (ImGui::Checkbox(u8"Wireframe", &_wireframe))
            {
                update = true;
                _switchWireframe->setIdNode(_wireframe ? 1 : 0);
            }
        }

        ImGui::PopItemWidth();

        ImGui::End();
        ImGui::PopStyleVar();

        ImGui::Render();

        return update;
    }
};

const uint32_t   TessellationTest::PATCH_SIZE = 64;
const float        TessellationTest::UV_SCALE   = 1.0f;

TEST_F( TessellationTest, run )
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
    loader._cpuBuffers[0] = _models._terrain.getIBOBuffer();
    loader._cpuBuffers[1] = _models._terrain.getVBOBuffer();

    loader._cpuBuffers[6]  = _models._uboScreenVS;
    loader._cpuBuffers[7]  = _models._uboCameraVS;
    loader._cpuBuffers[8]  = _models._uboLighting;
    loader._cpuBuffers[9]  = _models._specialization;
    loader._cpuBuffers[10] = _models._uboTessellation;
    
    loader._cpuImages[5] = _models._imageHeight->getImage();
    loader._cpuImages[6] = _models._imageTerrain->getImage();

    loader._cpuMeshDescriptors[0] = getMeshDescriptor();

    // Create Vulkan objects
    ASSERT_NO_FATAL_FAILURE(loadEngine(loader, u8"Tessellation.xml"));

    // Link event manager
    loader._dialogs[0].lock()->initialize(_eventManager, _resolution);

    _switchWireframe = dynamic_cast<Vulkan::CommandSwitch*>(loader._commandLinks[1]);
    ASSERT_TRUE(_switchWireframe != nullptr);

    ASSERT_NO_FATAL_FAILURE(GUI.initialize(_resolution));

    MouCaGraphic::Engine3DXMLLoader loaderGUI(manager);
    // Transfer Device/Framebuffer/RenderPass from first model
    loaderGUI._devices[0]      = loader._devices[0];
    loaderGUI._frameBuffers[0] = loader._frameBuffers[0];
    loaderGUI._renderPasses[0] = loader._renderPasses[0];

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
        ASSERT_TRUE(container != nullptr);
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
        transfer.indirectCopyCPUToGPU(commandBuffer, *loader._cpuBuffers[0].lock(),   *loader._buffers[0].lock());
        transfer.indirectCopyCPUToGPU(commandBuffer, *loader._cpuBuffers[1].lock(),   *loader._buffers[1].lock());
        // Images
        transfer.indirectCopyCPUToGPU(commandBuffer, *loader._cpuImages[5].lock(),    *loader._images[5].lock());
        transfer.indirectCopyCPUToGPU(commandBuffer, *loader._cpuImages[6].lock(),    *loader._images[6].lock());
        // GUI
        transfer.indirectCopyCPUToGPU(commandBuffer, *loaderGUI._cpuImages[0].lock(), *loaderGUI._images[0].lock());

        // Go
        transfer.transfer(commandBuffer);

        // Release
        commandBuffer->release(context->getDevice());
        pool->release(context->getDevice());
    }

    updateScreenUBO();
    updateCameraUBO();
    
    // Transfer data memory to GPU
    {
        MouCaGraphic::Engine3DTransfer transfer(*context);

        transfer.immediatCopyCPUToGPU(*_models._uboScreenVS,     *loader._buffers[6].lock());
        transfer.immediatCopyCPUToGPU(*_models._uboCameraVS,     *loader._buffers[7].lock());
        transfer.immediatCopyCPUToGPU(*_models._uboLighting,     *loader._buffers[8].lock());
        transfer.immediatCopyCPUToGPU(*_models._uboTessellation, *loader._buffers[10].lock());
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

        // Execute commands
        updateCommandBuffers(loader);
        updateCommandBuffersSurface(loader);

        /// Update Light position / camera
        auto demo = [&](const double)
        {
            // Move camera and light
            updateCameraUBO();

            {
                MouCaGraphic::Engine3DTransfer transfer(*context);
                transfer.immediatCopyCPUToGPU(*_models._uboScreenVS, *loader._buffers[6].lock());
                transfer.immediatCopyCPUToGPU(*_models._uboCameraVS, *loader._buffers[7].lock());
            }

            // Update mouse position into ImGUI
            GUI.updateMouse(_graphic.getRTPlatform().getMouse());

            if (updateUIOverlay() || needUpdateGUI)
            {
                needUpdateGUI = false;

                ASSERT_NO_THROW(GUI.prepareBuffer(*context));
                Vulkan::CommandContainer* container = dynamic_cast<Vulkan::CommandContainer*>(loaderGUI._commandLinks[0]);
                ASSERT_TRUE(container != nullptr);
                ASSERT_NO_THROW(GUI.buildCommands(container->getCommands()));

                // Update data
                MouCaGraphic::Engine3DTransfer transfer(*context);

                _models._tessellationParameters.tessellationFactor = _tessellation ? _tessellationFactor : 0.0f;
                transfer.immediatCopyCPUToGPU(*_models._uboTessellation, *loader._buffers[10].lock());

                // Refresh Command Buffer with new command
                updateCommandBuffers(loader);
                updateCommandBuffersSurface(loader, 0);
            }
        };
        mainLoop(manager, u8"Tessellation Demo ", demo);

        // EnableCodeCoverage
    }
    else
    {
        context->getDevice().waitIdle();

        updateUIOverlay();
        // Build GUI design
        updateUIOverlay();
        ASSERT_NO_THROW(GUI.prepareBuffer(*context));

        Vulkan::CommandContainer* container = dynamic_cast<Vulkan::CommandContainer*>(loaderGUI._commandLinks[0]);
        ASSERT_TRUE(container != nullptr);
        ASSERT_NO_THROW(GUI.buildCommands(container->getCommands()));

        // Refresh Command Buffer with new command
        updateCommandBuffers(loader);
        updateCommandBuffersSurface(loader, 0);

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
        takeScreenshot(manager, L"tessellation.png");
    }

    // Clean
    ASSERT_NO_THROW(GUI.release(*context));
    ASSERT_NO_THROW(manager.release());
    ASSERT_NO_THROW(releaseScene());

    // Clean allocate dialog
    ASSERT_NO_THROW(clearDialog(manager));
    ASSERT_NO_THROW(releaseEventManager());
}