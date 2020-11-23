#include "Dependancies.h"

#include <LibRT/include/RTCameraComportement.h>
#include <LibRT/include/RTGeometry.h>
#include <LibRT/include/RTImage.h>
#include <LibRT/include/RTLight.h>
#include <LibRT/include/RTScene.h>

#include <MouCaCore/interface/ILoaderManager.h>

#include <MouCa3DVulkanEngine/include/Renderable/MeshNormalMap.h>

#include <MouCa3DEngine/interface/IRendererManager.h>
#include <MouCa3DEngine/interface/ISceneSynchronizer.h>

#include <include/MouCa3DEngineTest.h>

//#define VULKAN_USER

namespace MouCa3DEngine
{

    class AnimationTest : public MouCa3DDeferredTest
    {
    protected:
        RT::Scene               _scene;     ///< Main scene.
        RT::RenderDialogWPtr    _window;    ///< Weak link to window.

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
        }

        void TearDown() override
        {
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

            ASSERT_NO_THROW(_env3D._eventManager->setRenderer(renderer.lock()));

            renderingSystem->linkRenderingSupport(renderer, MouCa3DEngine::IRenderingSystem::Plane);
        }

        ///------------------------------------------------------------------------
        /// \brief  Build scene
        /// 
        void makeScene(const BT::StringOS& mainFolder)
        {
            auto& resources = _gameEnv.getCore()->getResourceManager();
            auto& loader = _gameEnv.getCore()->getLoaderManager();

            // Load images/mesh
            auto imageColorMap = resources.openImage(mainFolder + BT::StringOS(L"mesh/armor/colormap.ktx"));
            auto imageNormalMap = resources.openImage(mainFolder + BT::StringOS(L"mesh/armor/normalmap.ktx"));
            auto mesh = resources.openMeshImport(mainFolder + BT::StringOS(L"mesh/armor/armor.dae"), MeshNormalMap::getMeshDescriptor(), RT::MeshImport::ComputeAll);

            MouCaCore::LoadingItems items =
            {
                MouCaCore::LoadingItem(imageColorMap,  MouCaCore::LoadingItem::Deferred),
                MouCaCore::LoadingItem(imageNormalMap, MouCaCore::LoadingItem::Deferred),
                MouCaCore::LoadingItem(mesh,           MouCaCore::LoadingItem::Deferred),
            };
            // Demand loading
            ASSERT_NO_THROW(loader.loadResources(items));

            // Prepare scene: Light
            const std::vector<RT::LightSPtr> lights =
            {
                std::make_shared<RT::Light>(RT::Light::Point, RT::Point3(1.0f), 15.0f * 0.25f),         // White
                std::make_shared<RT::Light>(RT::Light::Point, RT::Point3(0.7f, 0.0f, 0.0f), 15.0f),     // Red
                std::make_shared<RT::Light>(RT::Light::Point, RT::Point3(0.0f, 0.0f, 0.8f), 2.0f),      // Blue
                std::make_shared<RT::Light>(RT::Light::Point, RT::Point3(1.0f, 1.0f, 0.0f), 5.0f),      // Yellow
                std::make_shared<RT::Light>(RT::Light::Point, RT::Point3(0.0f, 1.0f, 0.2f), 5.0f),      // Green
                std::make_shared<RT::Light>(RT::Light::Point, RT::Point3(1.0f, 0.7f, 0.3f), 25.0f)      // Yellow
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

            RT::BoundingBox bbox;
            auto massive = std::make_shared<RT::MassiveInstance>();
            massive->addMesh(mesh);

            massive->addImage(imageColorMap);
            massive->addImage(imageNormalMap);

            const std::vector<RT::MassiveInstance::Indirect> indirects =
            {
                {6, 0, 0}
            };
            glm::quat rotate = glm::rotate(glm::quat(), BT::Maths::PI<float>() / 4.0f, glm::vec3(0.0f, 1.0f, 0.0f));
            const std::vector<RT::MassiveInstance::Instance> instances =
            {
                {
                    0,                      // _meshID;
                    RT::Point3(0,0,0),      // _position;
                    RT::Point4(0,0,0,1),    // _quaternion;
                    RT::Point3(1,1,1)       // _scale;
                },

                {
                    0,                      // _meshID;
                    RT::Point3(4,0,5),      // _position;
                    RT::Point4(0,0,0,1),    // _quaternion;
                    RT::Point3(1.5,1.5,1.5) // _scale;
                },

                {
                    0,                      // _meshID;
                    RT::Point3(-4,0,-4),    // _position;
                    RT::Point4(rotate.x, rotate.y, rotate.z, rotate.w),    // _quaternion;
                    RT::Point3(0.8,0.8,0.8) // _scale;
                },

                {
                    0,                      // _meshID;
                    RT::Point3(-8,0,-4),    // _position;
                    RT::Point4(0,0,0,1),    // _quaternion;
                    RT::Point3(0.9,0.9,0.9) // _scale;
                },

                {
                    0,                      // _meshID;
                    RT::Point3(4,0,-4),     // _position;
                    RT::Point4(rotate.x, rotate.y, rotate.z, -rotate.w),    // _quaternion;
                    RT::Point3(1.1,1.1,1.1) // _scale;
                },

                {
                    0,                      // _meshID;
                    RT::Point3(-4,0,4),     // _position;
                    RT::Point4(0,0,0,1),    // _quaternion;
                    RT::Point3(1.1,1.1,1.1) // _scale;
                },
            };

            massive->update(indirects, instances);

            _scene.addMassive(massive);

            /// Create BBox
            ASSERT_TRUE(_bboxes.get() == nullptr);
            _bboxes = std::make_shared<RT::BBoxes>();
            _bboxes->initialize();

            refreshBbox();

            _scene.addObjectRenderable(_bboxes);
        }
    };

    // cppcheck-suppress syntaxError
    TEST_F(AnimationTest, show)
    {
        auto renderingSystem = _env3D.getRenderingSystem().lock();
        // Build scene
        makeScene(MouCaEnvironment::getInputPath());

        // Change camera
        RT::CameraTrackBall* trackball = dynamic_cast<RT::CameraTrackBall*>( _env3D._trackball->getComportement() );
        ASSERT_TRUE(trackball != nullptr);
        trackball->moveTo({ 0.3f, 0.3f, 15.0f });

        // Prepare data to show
        renderingSystem->getSynchronizer().synchronizeScene(_scene, MouCa3DEngine::ISceneSynchronizer::All);

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
                const size_t frameMax = 17; // 1/60
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
        takeScreenshot(BT::StringOS(L"animation_0s.png"));

        // Change Time: +2s

        takeScreenshot(BT::StringOS(L"animation_2s.png"));
#endif
    }

}