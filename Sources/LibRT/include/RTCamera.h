/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibRT/include/RTObject.h>
#include <LibRT/include/RTViewport.h>

namespace RT
{
    class Camera : public RT::Object
    {
        protected:
            Matrix4 projectionMatrix;

        public:
            Camera(const Core::String& label = "Default Camera"):
            Object(TCamera, label)
            {}

            ~Camera() override = default;

            virtual void computePerspectiveCamera(const ViewportInt32& screenViewport)
            {
                MOUCA_PRE_CONDITION(screenViewport.getWidth() > 0);
                MOUCA_PRE_CONDITION(screenViewport.getHeight() > 0);

                const float ratio = static_cast<float>(screenViewport.getWidth()) / static_cast<float>(screenViewport.getHeight());
                projectionMatrix = glm::perspective(45.0f, ratio, 1.0f, 100000.0f);
            }

            virtual void computePerspectiveCamera(const ViewportInt32& screenViewport, const RT::BoundingBox& scene)
            {
                MOUCA_PRE_CONDITION(screenViewport.getWidth() > 0);
                MOUCA_PRE_CONDITION(screenViewport.getHeight() > 0);
                MOUCA_PRE_CONDITION(scene.isValid());

                const float ratio = static_cast<float>(screenViewport.getWidth()) / static_cast<float>(screenViewport.getHeight());

                const glm::vec3 distScene = scene.getCenter() - getOrientation().getPosition();
                const glm::vec3 dir = glm::normalize(getOrientation().getLocalToWorld()._rotation * glm::vec3(0.0, 0.0, -1.0));
                const float middle = glm::dot(dir, distScene);

                //printf(" Near: %f Far: %f\n", middle - scene.getRadius(), middle + scene.getRadius());
                //projectionMatrix = glm::perspective(45.0f, ratio, 0.0001f, 1000.0f);
                const float nearC = std::max(0.00001f, middle - scene.getRadius());
                const float farC  = std::max(0.00001f, middle + scene.getRadius());
                MOUCA_ASSERT(nearC <= farC);

                projectionMatrix = glm::perspective(45.0f, ratio, nearC, farC);
            }

            RT::Matrix4 getProjectionMatrix() const
            {
                return projectionMatrix;
            }
    };

    using CameraSPtr = std::shared_ptr<Camera>;
    using CameraWPtr = std::weak_ptr<Camera>;
}