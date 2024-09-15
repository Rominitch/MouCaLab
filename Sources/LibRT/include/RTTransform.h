/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace RT
{
#pragma warning(push)
#pragma warning(disable:4324) // Just to say align fill with blank because aligned.

    struct Transform final
    {
        alignas(16) glm::quat _rotation;
        alignas(16) glm::vec3 _position;
        alignas(16) glm::vec3 _homothetie;

        Transform(const glm::vec3& position = glm::vec3(0.0f), const glm::vec3& homothetie = glm::vec3(1.0f), const glm::quat& rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f)) :
        _position(position), _homothetie(homothetie), _rotation(rotation)
        {}

        // Copy constructor/operator
        Transform(const Transform&) = default;
        Transform& operator=(const Transform&) = default;

        // Move constructor/operator
        Transform(Transform&&) = default;
        Transform& operator= (Transform&&) = default;

        /// Destructor
        ~Transform() = default;

        static Transform inverse(const Transform& orientation)
        {
            Transform newOrientation(orientation);
            newOrientation.inverse();
            return newOrientation;
        }

        void inverse()
        {
            _homothetie = 1.0f / _homothetie;
            _rotation = glm::inverse(_rotation);
            _position = (_rotation * (_homothetie * -_position));
        }

        glm::mat4 convert() const
        {
            auto mat = glm::mat4_cast(_rotation);
            mat[0][0] *= _homothetie.x;
            mat[0][1] *= _homothetie.x;
            mat[0][2] *= _homothetie.x;

            mat[1][0] *= _homothetie.y;
            mat[1][1] *= _homothetie.y;
            mat[1][2] *= _homothetie.y;

            mat[2][0] *= _homothetie.z;
            mat[2][1] *= _homothetie.z;
            mat[2][2] *= _homothetie.z;

            mat[3][0] = _position.x;
            mat[3][1] = _position.y;
            mat[3][2] = _position.z;
            return mat;
        }

        bool operator==(const Transform& compare) const
        {
            return _rotation == compare._rotation
                && _position == compare._position
                && _homothetie == compare._homothetie;
        }

        // World = This * Local
        Transform operator * (const Transform& localSpace) const
        {
            Transform worldSpace;
            worldSpace._position = _position + _rotation * (localSpace._position * _homothetie);
            worldSpace._rotation = _rotation * localSpace._rotation;
            worldSpace._homothetie = _homothetie * localSpace._homothetie;
            return worldSpace;
        }

        glm::vec3 operator * (const glm::vec3& localPoint) const
        {
            return _position + _rotation * (localPoint * _homothetie);
        }

        static Transform lerp(const Transform& a, const Transform& b, const float factor)
        {
            Transform interpole;
            interpole._homothetie = a._homothetie + (b._homothetie - a._homothetie) * factor;
            interpole._rotation = glm::lerp(a._rotation, b._rotation, factor);
            interpole._position = a._position + (b._position - a._position) * factor;
            return interpole;
        }

        static Transform slerp(const Transform& a, const Transform& b, const float factor)
        {
            Transform interpole;
            interpole._homothetie = a._homothetie + (b._homothetie - a._homothetie) * factor;
            interpole._rotation = glm::slerp(a._rotation, b._rotation, factor);
            interpole._position = a._position + (b._position - a._position) * factor;
            return interpole;
        }
    };

#pragma warning(pop)
}