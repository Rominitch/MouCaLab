#include "Dependancies.h"

#include <LibRT/include/RTAnimationBones.h>
#include <LibRT/include/RTCameraComportement.h>
#include <LibRT/include/RTGeometry.h>
#include <LibRT/include/RTImage.h>
#include <LibRT/include/RTLight.h>
#include <LibRT/include/RTScene.h>

#include <LibVulkan/interface/IRendererDeferred.h> // MEGA STUPID ---> Need to remove !!!

#include <MouCaCore/interface/ILoaderManager.h>

#include <MouCa3DVulkanEngine/include/Renderable/AnimatedGeometry.h>

#include <MouCa3DEngine/interface/IRendererManager.h>
#include <MouCa3DEngine/interface/ISceneSynchronizer.h>

#include <include/MouCa3DEngineTest.h>

namespace MouCa3DEngine
{
    struct VertexComponent
    {
        float   _vertex[3];
        float   _texCoord[2];
        float   _color[3];
        float   _normal[3];
        float   _tangent[3];
        float   _weight[4];
        int     _ID[4];
    };

    class AnimationTest : public MouCa3DDeferredTest
    {
    protected:
        RT::Scene               _scene;     ///< Main scene.
        RT::RenderDialogWPtr    _window;    ///< Weak link to window.
        
        RT::AnimationImporterSPtr   _animation;
        std::array<RT::AnimatedGeometrySPtr, 100> _geometries;

    public:
        AnimationTest() = default;

        ~AnimationTest() override
        {
            _scene.release();
        }

        void SetUp() override
        {
            MouCa3DDeferredTest::SetUp();

            _env3D._camera->computePerspectiveCamera(_env3D._resolution);

            // Give ownership to scene
            _scene.addCamera(_env3D._camera);

            createRenderer();

            // Build scene
            makeScene(MouCaEnvironment::getInputPath());
        }

        void TearDown() override
        {
            _animation.reset();

            //Release
            for( auto& geometry : _geometries )
            {
                geometry.reset();
            }

            //_eventManager->release(); // NEED TO FIX event manager as a SPtr (need a weak ?).
            _env3D.releaseRenderingSystem(_gameEnv);

            MouCa3DDeferredTest::TearDown();
        }

        ///------------------------------------------------------------------------
        /// \brief  Replace default renderer by deferred.
        /// 
        void createRenderer()
        {
            BT_PRE_CONDITION(!_scene.getCameras().empty());

            auto renderingSystem = _env3D.setupRenderingSystem(_gameEnv, u8"Animation", MouCa3DDeferredEnvironment::getMode()).lock();
            ASSERT_TRUE(renderingSystem.get() != nullptr);

            // Allocate renderer
            auto renderer = renderingSystem->buildRenderer(MouCa3DEngine::IRenderingSystem::Deferred, _env3D._resolution.getSize());

            // Change renderer properties
            auto currentRenderer = renderer.lock();

            // MEGA STUPID ---> Need to remove !!!
            dynamic_cast<Vulkan::IRendererDeferred*>( currentRenderer->getProperties() )->setGlobalAmbiantFactor(0.5f);
            // MEGA STUPID ---> Need to remove !!!

            ASSERT_NO_THROW(_env3D._eventManager->setRenderer(currentRenderer));

            renderingSystem->linkRenderingSupport(renderer, MouCa3DEngine::IRenderingSystem::Plane);
        }

        ///------------------------------------------------------------------------
        /// \brief  Build scene
        /// 
        void makeScene(const BT::Path& mainFolder)
        {
            auto& resources = _gameEnv.getCore()->getResourceManager();
            auto& loader = _gameEnv.getCore()->getLoaderManager();

            // Load images/mesh
            auto mesh          = resources.openMeshImport(mainFolder / L"mesh"    / L"goblin.dae", AnimatedGeometry::getMeshAnimationDescriptor(), RT::MeshImport::SpecialExport);
            auto imageColorMap = resources.openImage(     mainFolder / L"textures"/ L"goblin.ktx");
            _animation         = resources.openAnimation( mainFolder / L"mesh"    / L"goblin.dae");

            MouCaCore::LoadingItems items =
            {
                MouCaCore::LoadingItem(mesh,           MouCaCore::LoadingItem::Deferred),
                MouCaCore::LoadingItem(imageColorMap,  MouCaCore::LoadingItem::Deferred),
                MouCaCore::LoadingItem(_animation,     MouCaCore::LoadingItem::Deferred)
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
            for( auto light : lights )
            {
                _scene.addLight(light);
            }

            ASSERT_NO_THROW(loader.synchronize());

            BT::uint32 idGob = 0;
            for( auto& geometry : _geometries  )
            {
                const float distance = 30.0f;
                RT::BoundingBox bbox;
                geometry = std::make_shared<RT::AnimatedGeometry>();

                geometry->getOrientation().setQuaternion(glm::rotate(glm::quat(), -BT::Maths::PI<float> * 0.5f, glm::vec3(1, 0, 0)));
                geometry->getOrientation().setHomothetie(RT::Point3(0.01f, 0.01f, 0.01f));
                geometry->getOrientation().setPosition(RT::Point3(( idGob % 10) * distance, 0.0f, (idGob / 10) * distance));

                geometry->setLabel("GoblinAnimated "+std::to_string(idGob));
                geometry->initialize(mesh, bbox);
                geometry->setBones(&_animation->getAnimation()._hierarchy);

                geometry->addImage(imageColorMap->getImage());

                _scene.addObjectRenderable(geometry);
                ++idGob;
            }

            RT::CameraTrackBall* trackball = dynamic_cast<RT::CameraTrackBall*>( _env3D._trackball->getComportement() );
            trackball->attachSupport(_geometries[55]);
        }
    };

// cppcheck-suppress syntaxError
TEST_F(AnimationTest, show)
{
    auto renderingSystem = _env3D.getRenderingSystem().lock();

    // Change camera
    RT::CameraTrackBall* trackball = dynamic_cast<RT::CameraTrackBall*>( _env3D._trackball->getComportement() );
    ASSERT_TRUE(trackball != nullptr);
    trackball->setDepthRange(50.0f, 1000.0f);
    trackball->moveTo({ 0.3f, 0.3f, 200.0f });

    double animationTime = 0.0;
    // Prepare data to show
    for( auto& geometry : _geometries )
    {
        _animation->getAnimation().updateAnimation(*geometry, 0, animationTime);
    }
    renderingSystem->getSynchronizer(MouCa3DEngine::IRenderingSystem::Deferred).synchronizeScene(_scene, MouCa3DEngine::ISceneSynchronizer::All);

    // Sync device (now data and all are ready to render)
    renderingSystem->synchronizeDevice();

    _env3D._eventManager->synchronizeCamera();

#ifdef VULKAN_USER
    renderingSystem->enableRender();

    // Build thread to update agent position
    bool threadEnd = false;
    auto demo = [&]()
    {
        try
        {
            const size_t frameMax = 0;
            std::chrono::time_point<std::chrono::system_clock> timeStart, timeEnd;
            timeStart = std::chrono::system_clock::now();
            while( !threadEnd )
            {
                timeEnd = std::chrono::system_clock::now();
                size_t elapse = std::chrono::duration_cast<std::chrono::milliseconds>( timeEnd - timeStart ).count();
                if( elapse > frameMax )
                {
                    // World to scene (build/delete missing data)
                    trackball->refresh();
                    _env3D._eventManager->synchronizeCamera();

                    animationTime += elapse * 0.001;

                    // Prepare data to show
                    for( auto& geometry : _geometries )
                    {
                        _animation->getAnimation().updateAnimation(*geometry, 0, animationTime);
                    }
                    renderingSystem->getSynchronizer(MouCa3DEngine::IRenderingSystem::Deferred).synchronizeScene(_scene, MouCa3DEngine::ISceneSynchronizer::All);

                    std::swap(timeStart, timeEnd);
                }
                else // Sleep if too quick
                    std::this_thread::sleep_for(std::chrono::milliseconds(frameMax - elapse));
            }
        }
        catch( const BT::Exception& e )
        {
            std::cout << e.read(0).getLibraryLabel() << u8" " << e.read(0).getErrorLabel() << std::endl;
        }
        catch( ... )
        {
            std::cout << u8"Unknown error caught !" << std::endl;
        }
    };

    std::thread animation(demo);

    // Launch event loop
    _gameEnv.get3DEngine().loopEvent();

    // Finish animation
    threadEnd = true;
    animation.join();

    // Stop rendering
    renderingSystem->disableRender();
#else
    takeScreenshot(BT::Path(u8"animation_0s.png"));

    // Change Time: +2s
    // Prepare data to show
    for( auto& geometry : _geometries )
    {
        _animation->getAnimation().updateAnimation(*geometry, 0, 2.0);
    }
    renderingSystem->getSynchronizer(IRenderingSystem::Deferred).synchronizeScene(_scene, ISceneSynchronizer::All);

    takeScreenshot(BT::Path(u8"animation_2s.png"));
#endif
}

TEST_F(AnimationTest, PERFORMANCE_animation_2000)
{
    auto renderingSystem = _env3D.getRenderingSystem().lock();

    // Compute many frames: 20
    for( double animationTime = 0.0; animationTime < 2.0; animationTime += 0.1 )
    {
        // For all characters: 100
        for( auto& geometry : _geometries )
        {
            _animation->getAnimation().updateAnimation(*geometry, 0, animationTime);
        }
        
        // Sync GPU
        renderingSystem->getSynchronizer(IRenderingSystem::Deferred).synchronizeScene(_scene, ISceneSynchronizer::All);
    }
}

}