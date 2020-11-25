/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include "LibRT/include/RTObject.h"
#include "LibRT/include/RTCameraComportement.h"

namespace RT
{

CameraTrackBall::CameraTrackBall():
CameraComportement(),
_movement({ -Core::Maths::PI<float> * 0.5f, Core::Maths::PI<float> * 0.25f, 10.0f }),
_sensitive({ 1.0f, 1.0f, 1.0f })
{
    //Reduce limit axis using epsilon (fix computation issue)
    const GeoFloat fEpsylon = 0.001f;
    _limit = { 0.0f, Core::Maths::PI<float> * 2.0f, -Core::Maths::PI<float> * 0.5f+fEpsylon, Core::Maths::PI<float> * 0.5f-fEpsylon, 1.0f, 1000.0f };
}

void CameraTrackBall::attachCamera(const ObjectSPtr& camera)
{
    CameraComportement::attachCamera(camera);

    updateSupportMatrix();
}

void CameraTrackBall::attachSupport(ObjectWPtr support)
{
    MOUCA_PRE_CONDITION(_linkCamera.get() != nullptr);
    MOUCA_PRE_CONDITION(!support.expired() );

    _support = support;

    updateSupportMatrix();

    MOUCA_POST_CONDITION(!_support.expired());
}

void CameraTrackBall::updateSupportMatrix()
{
    const Point3 newPosition( _movement[Depth]*sin(_movement[ThetaRad])*cos(_movement[PhiRad]),
                              _movement[Depth]*sin(_movement[PhiRad]),
                              _movement[Depth]*cos(_movement[ThetaRad])*cos(_movement[PhiRad]));
    const Point3 center = (_support.expired()) ? Point3(0.0f, 0.0f, 0.0f) : _support.lock()->getOrientation().getPosition();
    const auto mat = glm::lookAt(center + newPosition, center, Vector3(0.0f, 1.0f, 0.0f));
    
    // Edit world to local transformation
    auto transform = _linkCamera->getOrientation().getWorldToLocal();
    transform._rotation = glm::quat_cast(mat);
    transform._position = glm::vec3(mat[3][0], mat[3][1], mat[3][2]);
    _linkCamera->getOrientation().setWorldToLocal(transform);
}

void CameraTrackBall::rotate(const Vector2& angleRad)
{
    MOUCA_ASSERT(_linkCamera != nullptr);

    //Add angle
    _movement[ThetaRad] = Core::Maths::angleCyclic(_movement[ThetaRad] + angleRad.x);

    _movement[PhiRad]	+= angleRad.y;
    _movement[PhiRad] = glm::clamp(_movement[PhiRad], _limit[MinPhi], _limit[MaxPhi]);

    //Compute matrix
    updateSupportMatrix();
}

void CameraTrackBall::zoom(const float& fZoom)
{
    MOUCA_ASSERT(_linkCamera != nullptr);
    _movement[Depth] = glm::clamp(_movement[Depth] + fZoom, _limit[MinDepth], _limit[MaxDepth]);

    //Compute matrix
    updateSupportMatrix();
}

void CameraTrackBall::moveTo(const CameraOrientation& orientation)
{
    _movement = orientation;

    //Compute matrix
    updateSupportMatrix();
}

void CameraTrackBall::setDepthRange( const GeoFloat minDepth, const GeoFloat maxDepth )
{
    MOUCA_PRE_CONDITION( 0 < minDepth );
    MOUCA_PRE_CONDITION( minDepth <= maxDepth );
    _limit[MinDepth] = minDepth;
    _limit[MaxDepth] = maxDepth;
}

//////////////////////////////////////////////////////////////////////////
///                          CameraFlying
//////////////////////////////////////////////////////////////////////////

CameraFlying::CameraFlying():
_sensitive({ 1.0f, 1.0f, 1.0f, 1.0f })
{}

void CameraFlying::rotate(const Vector2& angleRad)
{
    const auto& transform = _linkCamera->getOrientation().getLocalToWorld();
    
    // Get X vector
    glm::quat rotation = glm::rotate(transform._rotation, angleRad.y, Vector3(1.0f, 0.0f, 0.0f));

    // Get Y vector
    rotation = glm::rotate(rotation, -angleRad.x, Vector3(0.0f, 1.0f, 0.0f));

    _linkCamera->getOrientation().setQuaternion(rotation);
}

void CameraFlying::move(const float& step)
{
    auto& orientation = _linkCamera->getOrientation();

    // Move on Z
    const glm::vec3 dir = orientation.getLocalToWorld()._rotation * Vector3(0.0f, 0.0f, -1.0f);
    orientation.setPosition(orientation.getPosition() + _sensitive[Forward] * step * dir);
}

void CameraFlying::strafing(const float& step)
{
    auto& orientation = _linkCamera->getOrientation();
    
    // Move on X
    const glm::vec3 dir = orientation.getLocalToWorld()._rotation * Vector3(1.0f, 0.0f, 0.0f);
    orientation.setPosition(orientation.getPosition() + _sensitive[Strafing] * step * dir);
}

void CameraFlying::moveTo(const Point3& position)
{
    _linkCamera->getOrientation().setPosition(position);
}

}