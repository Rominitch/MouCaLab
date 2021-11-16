/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace RT
{
    class Camera;
    using CameraWPtr = std::weak_ptr<Camera>;

    class CameraComportement;

    class CameraManipulator final
    {
        MOUCA_NOCOPY_NOMOVE(CameraManipulator);

        public:
            enum EState
            {
                ComputerControl,
                UserControl
            };
        
            enum EComportment
            {
                StaticCamera,
                TrackBallCamera,
                FlyingCamera,
                AvatarCamera
                //SliderCamera  //For 
            };

            CameraManipulator():
            _comportement(nullptr)
            {}

            ~CameraManipulator();

            void initialize(CameraWPtr camera)
            {
                MouCa::preCondition(!camera.expired());

                _linkCamera = camera;

                installComportement(StaticCamera);
            }

            void installComportement(const EComportment eState);

            CameraComportement* getComportement() const
            {
                return _comportement;
            }

            CameraWPtr  getCamera() const { return _linkCamera; }

        protected:
            CameraComportement*	_comportement;
            CameraWPtr  		_linkCamera;			///< Camera
    };
}
