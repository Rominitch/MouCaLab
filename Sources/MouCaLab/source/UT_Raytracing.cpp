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

// TMP
#include <LibVulkan/include/VKAccelerationStructure.h>
#include <LibVulkan/include/VKAccelerationStructureGeometry.h>
#include <LibVulkan/include/VKBufferStrided.h>
#include <LibVulkan/include/VKTracingRay.h>
// TMP

class RaytracingTest : public MouCaLabTest
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

    struct UniformData
    {
        glm::mat4 viewInverse;
        glm::mat4 projInverse;
        glm::vec4 lightPos;
        int32_t vertexSize;
    };

    struct Models
    {
        RT::CameraSPtr             _camera;

        // Meshes
        RT::MeshImportSPtr         _armor;
        RT::MeshImportSPtr         _ground;
        /*
        // Textures
        RT::ImageImportSPtr        _armorColor;
        RT::ImageImportSPtr        _armorNormal;
        RT::ImageImportSPtr        _groundColor;
        RT::ImageImportSPtr        _groundNormal;
        */
        // Buffers
        UniformData uniformData;
        
        RT::BufferLinkedCPUSPtr    _ubo;
        /*
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
        */
    };

    // CPU Asset
    RT::Array2ui    _resolution;
    Models          _models;
    RT::ObjectSPtr  _supportCamera;
    float           _animationTime = 0.0f;

    RaytracingTest() :
    _resolution(1280, 720)
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
        _models._camera->getOrientation().setPosition({ -2.15f, -0.3f, 8.75f });
        _models._camera->computePerspectiveCamera(RT::ViewportInt32(0, 0, _resolution.x, _resolution.y));

        configureEventManager();

        // Get manager and path
        auto& resources = _core.getResourceManager();
        auto& loader = _core.getLoaderManager();

        const auto& meshPath = resources.getResourceFolder(MouCaCore::ResourceManager::Meshes);
        //const auto& texturePath = resources.getResourceFolder(MouCaCore::ResourceManager::Textures);

        // Prepare resources images/mesh
        const auto armorPath = meshPath / L"armor";
        //_models._armorColor = resources.openImage(armorPath / L"colormap.ktx");
        //_models._armorNormal = resources.openImage(armorPath / L"normalmap.ktx");
        _models._armor = resources.openMeshImport(armorPath / L"armor.dae", getMeshDescriptor(), RT::MeshImport::ComputeAll);

        //_models._groundColor = resources.openImage(texturePath / L"stonefloor02_color_bc3_unorm.ktx");
        //_models._groundNormal = resources.openImage(texturePath / L"stonefloor02_normal_bc3_unorm.ktx");
        _models._ground = resources.openMeshImport(meshPath / L"openbox.dae", getMeshDescriptor(), RT::MeshImport::ComputeAll);

        // Demand loading (deferred => thread loading)
        MouCaCore::LoadingItems items =
        {
            //MouCaCore::LoadingItem(_models._armorColor,  MouCaCore::LoadingItem::Deferred),
            //MouCaCore::LoadingItem(_models._armorNormal, MouCaCore::LoadingItem::Deferred),
            MouCaCore::LoadingItem(_models._armor,       MouCaCore::LoadingItem::Deferred),

            //MouCaCore::LoadingItem(_models._groundColor,  MouCaCore::LoadingItem::Deferred),
            //MouCaCore::LoadingItem(_models._groundNormal, MouCaCore::LoadingItem::Deferred),
            MouCaCore::LoadingItem(_models._ground,       MouCaCore::LoadingItem::Deferred)
        };
        loader.loadResources(items);

        // Wait all resources are loaded
        ASSERT_NO_THROW(loader.synchronize());
        
        // Make linked buffer (no memory management)
        _models._ubo = std::make_shared<RT::BufferLinkedCPU>();
        _models._ubo->create(RT::BufferDescriptor(sizeof(UniformData)), 1, &_models._ubo);
        /*
        _models._uboVS = std::make_shared<RT::BufferLinkedCPU>();
        _models._uboVS->create(RT::BufferDescriptor(sizeof(UBOGeometry)), 1, &_models._geometryVS);
        _models._uboOffscreenVS = std::make_shared<RT::BufferLinkedCPU>();
        _models._uboOffscreenVS->create(RT::BufferDescriptor(sizeof(UBOGeometry)), 1, &_models._geometryOffscreenVS);
        _models._uboLighting = std::make_shared<RT::BufferLinkedCPU>();
        _models._uboLighting->create(RT::BufferDescriptor(sizeof(Lighting)), 1, &_models._lighting);
        _models._uboModels = std::make_shared<RT::BufferLinkedCPU>();
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
            buffer->_ambiant = 0.15f;
        }
        */
    }

    void releaseScene()
    {
        auto& resources = _core.getResourceManager();

        //resources.releaseResource(std::move(_models._armorColor));
        //resources.releaseResource(std::move(_models._armorNormal));
        resources.releaseResource(std::move(_models._armor));

        //resources.releaseResource(std::move(_models._groundColor));
        //resources.releaseResource(std::move(_models._groundNormal));
        resources.releaseResource(std::move(_models._ground));

        /*
        _models._uboVS->release();
        _models._uboVS.reset();
        _models._uboOffscreenVS->release();
        _models._uboOffscreenVS.reset();
        _models._uboLighting->release();
        _models._uboLighting.reset();
        _models._uboModels->release();
        _models._uboModels.reset();
        */
    }

    void updateUBO(MouCaGraphic::Engine3DXMLLoader& loader, const Vulkan::ContextDevice& context, float time)
    {
//         ASSERT_EQ(1, loader._buffers.size());
//         auto& memory = loader._buffers[0].lock()->getMemory();
//         memory.map(context.getDevice());
// 
//         memcpy(memory.getMappedMemory<char>(), &time, sizeof(time));
// 
//         memory.unmap(context.getDevice());
    }
};

TEST_F(RaytracingTest, run)
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
    loader._cpuBuffers[0] = _models._armor->getMesh().getIBOBuffer();
    loader._cpuBuffers[1] = _models._armor->getMesh().getVBOBuffer();
    loader._cpuBuffers[2] = _models._ground->getMesh().getIBOBuffer();
    loader._cpuBuffers[3] = _models._ground->getMesh().getVBOBuffer();
    loader._cpuBuffers[4] = _models._ubo;

    loader._cpuMesh[0] = _models._armor->getWeakMesh();

    ASSERT_NO_FATAL_FAILURE(loadEngine(loader, u8"Raytracing.xml"));

    MouCaGraphic::Engine3DXMLLoader loaderGUI(manager);
    // Transfer Device/Framebuffer/RenderPass from first model
    loaderGUI._devices[0]      = loader._devices[0];
    loaderGUI._frameBuffers[0] = loader._frameBuffers[0];
    loaderGUI._renderPasses[0] = loader._renderPasses[0];

//-------------------------------------------------------------------------------------------------
//                                      Step 3: Transfer CPU -> GPU
//-------------------------------------------------------------------------------------------------
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
        //transfer.indirectCopyCPUToGPU(commandBuffer, *loader._cpuBuffers[2].lock(), *loader._buffers[2].lock());
        //transfer.indirectCopyCPUToGPU(commandBuffer, *loader._cpuBuffers[3].lock(), *loader._buffers[3].lock());
        
        // Go
        transfer.transfer(commandBuffer);

        commandBuffer->release(context->getDevice());
        pool->release(context->getDevice());
    }

    // Update info
    //updateUniformBuffersScreen();
    //updateUniformBufferDeferredMatrices();
    //updateUniformBufferDeferredLights(0);
    /*
    // Transfer data memory to GPU
    {
        MouCaGraphic::Engine3DTransfer transfer(*context);

        transfer.immediatCopyCPUToGPU(*_models._uboVS, *loader._buffers[6].lock());
        transfer.immediatCopyCPUToGPU(*_models._uboOffscreenVS, *loader._buffers[7].lock());
        transfer.immediatCopyCPUToGPU(*_models._uboLighting, *loader._buffers[8].lock());
        transfer.immediatCopyCPUToGPU(*_models._uboModels, *loader._buffers[9].lock());
    }
    */

    // Build Acceleration structure
    /*
    Vulkan::AccelerationStructureSPtr asBottom = std::make_shared<Vulkan::AccelerationStructure>();
    {
        std::vector<Vulkan::AccelerationBuildGeometry> bgs(1);
        auto triangles = std::make_unique<Vulkan::AccelerationStructureGeometryTriangles>();
        triangles->initialize(_models._armor->getMesh(), loader._buffers[1], loader._buffers[0], VK_GEOMETRY_OPAQUE_BIT_KHR);

        bgs[0].addGeometry(std::move(triangles));
        bgs[0].initialize(*context);


        asBottom->initialize(*context, 0, VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, bgs[0].getBuildInfo());
        asBottom->build(*context, bgs);
    }
    
    Vulkan::AccelerationStructureSPtr asTop = std::make_shared<Vulkan::AccelerationStructure>();
    {
        VkTransformMatrixKHR transformMatrix =
        {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f
        };
        Vulkan::AccelerationStructureGeometryInstance::Instance instance;
        instance.initialize(asBottom, VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR, std::move(transformMatrix));

        std::vector<Vulkan::AccelerationBuildGeometry> bgs(1);
        auto geomInstance = std::make_unique<Vulkan::AccelerationStructureGeometryInstance>();
        geomInstance->initialize(VK_GEOMETRY_OPAQUE_BIT_KHR);
        geomInstance->create(*context);

        bgs[0].addGeometry(std::move(geomInstance));
        bgs[0].initialize(*context);

        asTop->initialize(*context, 0, VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, bgs[0].getBuildInfo());
        asTop->build(*context, bgs);
    }
    */
    //-------------------------------------------------------------------------------------------------
    //                                       Step 4: Build / Execute
    //-------------------------------------------------------------------------------------------------

    // Build AS
    ASSERT_NO_FATAL_FAILURE(loader._accelerationStructures[0].lock()->initialize(context->getDevice()));
    ASSERT_NO_FATAL_FAILURE(loader._accelerationStructures[1].lock()->initialize(context->getDevice()));

    // Transfer AS data
    std::vector<Vulkan::AccelerationStructureWPtr> accelerationStructs{ loader._accelerationStructures[0], loader._accelerationStructures[1] };
    Vulkan::AccelerationStructure::createAccelerationStructure(context->getDevice(), accelerationStructs);

    // Execute commands
    loader._surfaces[0].lock()->updateCommandBuffer(VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
    //loader._commandBuffers[0].lock()->execute();

    context->getDevice().waitIdle();

    //-------------------------------------------------------------------------------------------------
    //                                            Step 5: Play
    //-------------------------------------------------------------------------------------------------

    // Set value
    updateUBO(loader, *context, 0.121f);

    // Execute rendering
    if (MouCaEnvironment::isDemonstrator())
    {
        std::thread thread([&]()
            {
                float fTime = 0.0;
                while (_graphic.getRTPlatform().isWindowsActive())
                {
                    updateUBO(loader, *context, fTime);
                    fTime += 0.00001f;
                }
            });

        mainLoop(manager, u8"Raytracing Demo ");

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

        takeScreenshot(manager, L"Raytracing.png");
    }

//     asTop->release(*context);
//     asBottom->release(*context);
    
    // Clean
    ASSERT_NO_THROW(manager.release());

    // Clean allocate dialog
    ASSERT_NO_THROW(clearDialog(manager));
}