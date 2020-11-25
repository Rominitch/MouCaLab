/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace RT
{
    class Scene;
    using SceneSPtr = std::shared_ptr<Scene>;

    class ContextExecution;
    class Layer;
    class CameraManipulator;

    class Camera;
    class Renderer;
    using RendererSPtr = std::shared_ptr<Renderer>;

    class Viewer
    {
        public:
            enum ViewerState
            {
                StateNone = 0x00000000,

                BitRunning = 0x00000001,
                BitClosed = 0x00000002,
                BitVisualization = 0x00000008,

                StateRun = BitRunning | BitVisualization,
                StateFinish = BitClosed,
                MaskApplicationState = 0x000000FF,

                MaskStateGlobal = 0xFFFFFFFF
            };

        protected:
            ViewerState	    _state = StateNone;
            SceneSPtr	    _scene;
            std::shared_ptr<Renderer> 	_renderer;

            std::shared_ptr<CameraManipulator>  _cameraManipulator;
            std::shared_ptr<Camera>             _camera;

        public:
            Viewer(void)
            {
                MOUCA_ASSERT(isNull());
            }
            virtual ~Viewer(void)
            {
                _renderer.reset();
                _cameraManipulator.reset();
                _camera.reset();
                //_eventManager.reset();

                MOUCA_ASSERT(isNull());
            }

            void setRenderer(const RT::RendererSPtr& renderer);

            void setCameraManipulator(std::shared_ptr<CameraManipulator>& cameraManipulator);

            void setCamera(std::shared_ptr<Camera>& camera);

            bool isNull() const;

            bool isState(const ViewerState eState) const
            {
                return (_state & eState) == eState;
            }

            ViewerState getState(const ViewerState eMask) const
            {
                return static_cast<ViewerState>(_state & eMask);
            }

            void setState(const ViewerState eState, const ViewerState eMask)
            {
                _state = static_cast<ViewerState>((_state & ~eMask) | (eState & eMask));
            }

            std::shared_ptr<Renderer> getRenderer() const
            {
                return _renderer;
            }

            SceneSPtr GetScene() const
            {
                return _scene;
            }

            const std::shared_ptr<CameraManipulator>& getCameraManipulator() const
            {
                return _cameraManipulator;
            }
    };

}