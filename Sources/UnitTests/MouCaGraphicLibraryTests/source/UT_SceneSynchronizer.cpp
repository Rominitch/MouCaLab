#include "Dependancies.h"

#include <LibRT/include/RTCameraComportement.h>
#include <LibRT/include/RTGeometry.h>
#include <LibRT/include/RTImage.h>
#include <LibRT/include/RTLight.h>
#include <LibRT/include/RTMassiveInstance.h>
#include <LibRT/include/RTRenderDialog.h>
#include <LibRT/include/RTScene.h>
#include <LibRT/include/RTViewport.h>
#include <LibRT/include/RTWindow.h>

#include <LibVulkan/interface/VKDefines.h> // Need to remove !!! Don't use Vulkan item only generic Lib

#include <MouCaCore/interface/ILoaderManager.h>

#include <MouCa3DEngine/interface/IRendererManager.h>
#include <MouCa3DEngine/interface/ISceneSynchronizer.h>

#include <include/MouCa3DEngineTest.h>

//#define VULKAN_USER

namespace MouCa3DEngine
{

//----------------------------------------------------------------------------
/// \brief Scene Synchronize test: allow to create window/deferred renderer and make all tests about sync.
///     
/// \author Romain MOURARET
class SceneSynchronizerTest : public MouCa3DDeferredTest
{
    protected:
        RT::Scene                   _scene;             ///< Main scene
        
        RT::RenderDialogWPtr        _window;            ///< Weak link to window
    public:
        SceneSynchronizerTest() = default;

        ~SceneSynchronizerTest() override
        {
            _scene.release();
        }

        void SetUp() override
        {
            MouCa3DDeferredTest::SetUp();

            _env3D._camera->computePerspectiveCamera( _env3D._resolution);
            _env3D._camera->getOrientation().setPosition(RT::Point3(2.5f, 2.5f, -7.5f));

            // Give ownership to scene
            _scene.addCamera( _env3D._camera);

            createRenderer();
        }

        void TearDown() override
        {
            _env3D.releaseRenderingSystem( _gameEnv );
            
            MouCa3DDeferredTest::TearDown();
        }

        ///------------------------------------------------------------------------
        /// \brief  Replace default renderer by deferred.
        /// 
        void createRenderer()
        {
            BT_PRE_CONDITION(!_scene.getCameras().empty());

            auto renderingSystem = _env3D.setupRenderingSystem(_gameEnv, u8"SceneSynchronizerTest", MouCa3DDeferredEnvironment::getMode()).lock();
            
            ASSERT_TRUE( renderingSystem.get() != nullptr );

            // Allocate renderer
            auto renderer = renderingSystem->buildRenderer(MouCa3DEngine::IRenderingSystem::Deferred, _env3D._resolution.getSize());

            ASSERT_NO_THROW(_env3D._eventManager->setRenderer(renderer.lock()));

            renderingSystem->linkRenderingSupport( renderer, MouCa3DEngine::IRenderingSystem::Plane );
        }

        ///------------------------------------------------------------------------
        /// \brief  Build scene
        /// 
        void makeScene(const BT::Path& mainFolder)
        {
            auto& resources = _gameEnv.getCore()->getResourceManager();
            auto& loader    = _gameEnv.getCore()->getLoaderManager();

            // Load images/mesh
            const auto armorPath = mainFolder / L"mesh" / L"armor";
            auto imageColorMap  = resources.openImage(     armorPath / L"colormap.ktx");
            auto imageNormalMap = resources.openImage(     armorPath / L"normalmap.ktx");
            auto mesh           = resources.openMeshImport(armorPath / L"armor.dae", Vulkan::getMeshDescriptor(), RT::MeshImport::ComputeAll);
            MouCaCore::LoadingItems items =
            {
                MouCaCore::LoadingItem(imageColorMap,  MouCaCore::LoadingItem::Deferred),
                MouCaCore::LoadingItem(imageNormalMap, MouCaCore::LoadingItem::Deferred),
                MouCaCore::LoadingItem(mesh,           MouCaCore::LoadingItem::Deferred)
            };
            // Demand loading
            ASSERT_NO_THROW(loader.loadResources(items));

            // Prepare scene: Light
            const std::vector<RT::LightSPtr> lights =
            {
                std::make_shared<RT::Light>(RT::Light::Form::Point, RT::Point3(1.0f), 15.0f * 0.25f),         // White
                std::make_shared<RT::Light>(RT::Light::Form::Point, RT::Point3(0.7f, 0.0f, 0.0f), 15.0f),     // Red
                std::make_shared<RT::Light>(RT::Light::Form::Point, RT::Point3(0.0f, 0.0f, 0.8f), 2.0f),      // Blue
                std::make_shared<RT::Light>(RT::Light::Form::Point, RT::Point3(1.0f, 1.0f, 0.0f), 5.0f),      // Yellow
                std::make_shared<RT::Light>(RT::Light::Form::Point, RT::Point3(0.0f, 1.0f, 0.2f), 5.0f),      // Green
                std::make_shared<RT::Light>(RT::Light::Form::Point, RT::Point3(1.0f, 0.7f, 0.3f), 25.0f)      // Yellow
            };
            lights[0]->getOrientation().setPosition(RT::Point3(0.0f, 5.0f, 2.0f));
            lights[1]->getOrientation().setPosition(RT::Point3(-2.0f, 0.0f, 0.0f));
            lights[2]->getOrientation().setPosition(RT::Point3(2.0f, 1.0f, 0.0f));
            lights[3]->getOrientation().setPosition(RT::Point3(0.0f, 0.9f, 0.5f));
            lights[4]->getOrientation().setPosition(RT::Point3(0.0f, 0.5f, 0.0f));
            lights[5]->getOrientation().setPosition(RT::Point3(0.0f, 1.0f, 0.0f));

            // Add into scene
            for(const auto& light : lights)
            {
                _scene.addLight(light);
            }

            ASSERT_NO_THROW(loader.synchronize());

            RT::BoundingBox bbox;
            auto geometry = std::make_shared<RT::GeometryExternal>();
            geometry->setLabel("Armor");
            geometry->initialize(mesh, bbox);

            geometry->addImage(imageColorMap->getImage());
            geometry->addImage(imageNormalMap->getImage());

            _scene.addGeometry(geometry);
        }
};

// cppcheck-suppress syntaxError
TEST_F(SceneSynchronizerTest, synchronize)
{
    auto renderingSystem = _env3D.getRenderingSystem().lock();
    // Build scene
    makeScene(MouCaEnvironment::getInputPath());

    // Change camera
    RT::CameraTrackBall* trackball = dynamic_cast<RT::CameraTrackBall*>(_env3D._trackball->getComportement());
    ASSERT_TRUE(trackball != nullptr);
    trackball->moveTo({ 0.0f, 0.0f, 5.0f });

    ASSERT_EQ(1, _scene.getGeometries().size());

    // Prepare data to show
    renderingSystem->getSynchronizer(MouCa3DEngine::IRenderingSystem::Deferred).synchronizeScene(_scene, MouCa3DEngine::ISceneSynchronizer::All);

    // Sync device (now data and all are ready to render)
    renderingSystem->synchronizeDevice();

    _env3D._eventManager->synchronizeCamera();

#ifdef VULKAN_USER
    renderingSystem->enableRender();

    bool end=false;
    auto demo = [&]()
    {
        float angle = 0.0f;
        while( !end )
        {
            RT::Point4 center;
            std::this_thread::sleep_for( std::chrono::milliseconds(60) );
            for( const auto& light : _scene.getLights() )
            {
                auto pos = light->getPosition();

                float distance = glm::length( pos - center );

                pos = distance * RT::Point4( std::cos(angle), 0.0f, std::sin(angle), 0.0f );
                light->setPosition( pos );
                
                angle += 0.01f;
                if( angle >= BT::Maths::PI<float>*2.0f )
                    angle -= BT::Maths::PI<float>*2.0f;
            }
            renderingSystem->getSynchronizer(MouCa3DEngine::IRenderingSystem::Deferred).synchronizeScene(_scene, MouCa3DEngine::ISceneSynchronizer::All);
        }
    };

    std::thread animation( demo );

    // Launch event loop
    _engine3D->loopEvent();

    end = true;
    animation.join();

    // Stop rendering
    renderingSystem->disableRender();
#else
    takeScreenshot(BT::Path(u8"scene_synchronizer.png"));
#endif
}

TEST_F(SceneSynchronizerTest, allItems)
{
    ///--------------------------------------------------------------------------------------------
    /// Load shared data
    const std::vector<RT::MassiveInstance::Indirect> indirects =
    {
        { 1, 0, 0 }
    };

    const std::vector<RT::MassiveInstance::Instance> instances =
    {
        {
            0,                      // _meshID;
            RT::Point3(0,0,0),      // _position;
            RT::Point4(0,0,0,1),    // _quaternion;
            RT::Point3(1,1,1)       // _scale;
        }
    };

    RT::BoundingBox bbox;

    auto& resources = _gameEnv.getCore()->getResourceManager();
    auto& loader    = _gameEnv.getCore()->getLoaderManager();

    const auto pathArmor = MouCaEnvironment::getInputPath() / L"mesh" / L"armor";
    // Load images/mesh
    auto imageColorMap  = resources.openImage(pathArmor / L"colormap.ktx");
    auto imageNormalMap = resources.openImage(pathArmor / L"normalmap.ktx");
    auto mesh           = resources.openMeshImport(pathArmor / L"armor.dae", Vulkan::getMeshDescriptor(), RT::MeshImport::ComputeAllInvert);
    MouCaCore::LoadingItems items =
    {
        MouCaCore::LoadingItem(imageColorMap,  MouCaCore::LoadingItem::Deferred),
        MouCaCore::LoadingItem(imageNormalMap, MouCaCore::LoadingItem::Deferred),
        MouCaCore::LoadingItem(mesh,           MouCaCore::LoadingItem::Deferred)
    };
    // Demand loading
    ASSERT_NO_THROW(loader.loadResources(items));
    ASSERT_NO_THROW(loader.synchronize());

    ///--------------------------------------------------------------------------------------------
    /// Create scene
    auto renderingSystem = _env3D.getRenderingSystem().lock();

    // Make scene 1
    {
        // Add new light
        _scene.addLight(std::make_shared<RT::Light>(RT::Light::Form::Point, RT::Point3(1.0f), 15.0f * 0.25f));

        // Add new geometry
        auto geometry = std::make_shared<RT::GeometryExternal>();
        geometry->setLabel("Armor");
        geometry->initialize(mesh, bbox);

        geometry->addImage(imageColorMap->getImage());
        geometry->addImage(imageNormalMap->getImage());

        _scene.addGeometry(geometry);

        // Add new Massive
        auto massive = std::make_shared<RT::MassiveInstance>();
        massive->addMesh(mesh);

        massive->addImage(imageColorMap->getImage());
        massive->addImage(imageNormalMap->getImage());

        
        massive->update(indirects, instances);
        _scene.addMassive(massive);

        // Update scene
        renderingSystem->getSynchronizer(MouCa3DEngine::IRenderingSystem::Deferred).synchronizeScene(_scene, MouCa3DEngine::ISceneSynchronizer::All);
    }

    // Make scene 2
    {
        // Add similar geometry
        auto geometry = std::make_shared<RT::GeometryExternal>();
        geometry->setLabel("Armor2");
        geometry->initialize(mesh, bbox);
        geometry->addImage(imageColorMap->getImage());
        geometry->addImage(imageNormalMap->getImage());
        _scene.addGeometry(geometry);

        // Add similar massive
        auto massive = std::make_shared<RT::MassiveInstance>();
        massive->addMesh(mesh);
        massive->addImage(imageColorMap->getImage());
        massive->addImage(imageNormalMap->getImage());
        massive->update(indirects, instances);
        _scene.addMassive(massive);

        // Update scene
        renderingSystem->getSynchronizer(MouCa3DEngine::IRenderingSystem::Deferred).synchronizeScene(_scene, MouCa3DEngine::ISceneSynchronizer::All);
    }
}

}