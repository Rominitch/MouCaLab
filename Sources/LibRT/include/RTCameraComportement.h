/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace RT
{
    class Object;
    using ObjectSPtr = std::shared_ptr<Object>;
    using ObjectWPtr = std::weak_ptr<Object>;

    class Camera;

    class CameraComportement
    {
        MOUCA_NOCOPY_NOMOVE(CameraComportement);

        protected:
            Object*		_lockedObject;
            ObjectSPtr	_linkCamera;            ///< [LINK] Link to camera object (use sharedPtr for speed access BUT not ownership).

            CameraComportement():
            _linkCamera(nullptr), _lockedObject(nullptr)
            {}

        public:
            virtual ~CameraComportement()
            {
                MouCa::postCondition( _linkCamera.use_count() != 1 ); // DEV Issue: We are not ownership !!!
            }

            virtual void attachCamera(const ObjectSPtr& support)
            {
                MouCa::preCondition(support.get() != nullptr);

                _linkCamera = support;

                MouCa::postCondition(_linkCamera.get() != nullptr);
            }
     };

    class CameraStatic final : public CameraComportement
    {
        public:
            CameraStatic() = default;
    
            ~CameraStatic() override = default;
    };

    class CameraTrackBall final : public CameraComportement
    {
        public:
            enum
            {
                MinTheta,
                MaxTheta,
                MinPhi,
                MaxPhi,
                MinDepth,
                MaxDepth,
                NbLimits
            };
            using Limitation = std::array<GeoFloat, NbLimits>;

            enum
            {
                ThetaRad,
                PhiRad,
                Depth,
                NbMovements
            };
            using CameraOrientation = std::array<GeoFloat, NbMovements>;

            /// Constructor
            CameraTrackBall();

            /// Destructor
            ~CameraTrackBall() override = default;

            void attachCamera(const ObjectSPtr& camera) override;
            void attachSupport(ObjectWPtr support);

            ObjectWPtr getSupport() const
            {
                return _support;
            }

            void rotate(const Vector2& angleRad);
            void zoom(const float& zoom);

            void moveTo(const CameraOrientation& orientation);

            void setDepthRange(const GeoFloat minDepth, const GeoFloat maxDepth);

            const Limitation& getLimitation() const
            {
                return _limit;
            }

            void refresh()
            {
                updateSupportMatrix();
            }

        protected:
            ObjectWPtr          _support;           ///< Support of camera.
            CameraOrientation	_movement;
            Limitation	        _limit;

            CameraOrientation	_sensitive;         ///< Global sensitive.

            void updateSupportMatrix();
    };

    //----------------------------------------------------------------------------
    /// \brief Simple flying camera to see all scene quickly.
    /// 
    class CameraFlying final : public CameraComportement
    {
        public:
            enum
            {
                Forward,
                Strafing,
                RotateX,
                RotateY,
                NbMovements
            };
            using CameraOrientation = std::array<GeoFloat, NbMovements>;

            CameraFlying();
            ~CameraFlying() override = default;

            void rotate(const Vector2& angleRad);
            void move(const float& step);
            void strafing(const float& step);

            void moveTo(const Point3& position);

        protected:
            CameraOrientation	_sensitive;         ///< Global sensitive.
    };

    class CameraAvatar : public CameraComportement
    {
        protected:
            Object*		_object;

        public:
            CameraAvatar() = default;
                
            ~CameraAvatar() override = default;
    };
}