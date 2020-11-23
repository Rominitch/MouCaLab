/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibRT/include/RTTransform.h>

namespace RT
{
    /*
    //------------------------------------------------------------------------
    ///	\class	Orientation
    ///	\brief	Main class for object which define matrices.
    ///
    class OrientationMat final
    {
        public:
            ///	Constructor: default orientation is Identity.
            OrientationMat():
            _matrixW2L( 1.0f ), _matrixL2W( 1.0f )
            {}

            ///	Destructor
            ~OrientationMat() = default;

            //------------------------------------------------------------------------
            ///	\brief Set matrix and compute invert matrix.
            ///	\param[in] matrix : matrix to set
            ///	\return	Link to complex station.
            void setMatrixW2L( const Matrix4& matrix )
            {
                _matrixW2L = matrix;
                _matrixL2W = glm::inverse( matrix );
            }

            //------------------------------------------------------------------------
            /// \brief  Change current position.
            /// 
            /// \param[in] position: new position.
            void setPosition( const Point4& position )
            {
                _matrixL2W = Matrix4( _matrixL2W[0], _matrixL2W[1], _matrixL2W[2], position );
                _matrixW2L = glm::inverse( _matrixL2W );
            }

            //------------------------------------------------------------------------
            /// \brief  Get current position.
            /// 
            /// \returns Current position.
            const Point4& getPosition() const
            {
                return _matrixL2W[3];
            }

            //------------------------------------------------------------------------
            /// \brief  Get matrix world to local transformation.
            /// 
            /// \returns Matrix world to local.
            const Matrix4& getWorldToLocal() const
            {
                return _matrixW2L;
            }

            //------------------------------------------------------------------------
            /// \brief  Get matrix local to world transformation.
            /// 
            /// \returns Matrix local to world.
            const Matrix4& getLocalToWorld() const
            {
                return _matrixL2W;
            }

            //------------------------------------------------------------------------
            /// \brief  Print on standard output the representation of instance (both matrices).
            /// 
            void print()
            {
                std::cout << std::fixed << std::setprecision(4) << std::setw(6) << std::setfill('0');
                std::cout << "W2L: "<< std::endl
                          << "   |" << _matrixW2L[0][0] << ", " << _matrixW2L[0][1] << ", " << _matrixW2L[0][2] << ", " << _matrixW2L[0][3] << " |" << std::endl
                          << "   |" << _matrixW2L[1][0] << ", " << _matrixW2L[1][1] << ", " << _matrixW2L[1][2] << ", " << _matrixW2L[1][3] << " |" << std::endl
                          << "   |" << _matrixW2L[2][0] << ", " << _matrixW2L[2][1] << ", " << _matrixW2L[2][2] << ", " << _matrixW2L[2][3] << " |" << std::endl
                          << "   |" << _matrixW2L[3][0] << ", " << _matrixW2L[3][1] << ", " << _matrixW2L[3][2] << ", " << _matrixW2L[3][3] << " |" << std::endl;
                std::cout << "L2W: "<< std::endl
                          << "   |" << _matrixL2W[0][0] << ", " << _matrixL2W[0][1] << ", " << _matrixL2W[0][2] << ", " << _matrixL2W[0][3] << " |" << std::endl
                          << "   |" << _matrixL2W[1][0] << ", " << _matrixL2W[1][1] << ", " << _matrixL2W[1][2] << ", " << _matrixL2W[1][3] << " |" << std::endl
                          << "   |" << _matrixL2W[2][0] << ", " << _matrixL2W[2][1] << ", " << _matrixL2W[2][2] << ", " << _matrixL2W[2][3] << " |" << std::endl
                          << "   |" << _matrixL2W[3][0] << ", " << _matrixL2W[3][1] << ", " << _matrixL2W[3][2] << ", " << _matrixL2W[3][3] << " |" << std::endl;
            }

        private:
            Matrix4	    _matrixW2L;			///< World to local.
            Matrix4	    _matrixL2W;			///< Local to World.
    };
    */

    //------------------------------------------------------------------------
    ///	\class	Orientation
    ///	\brief	Main class for object which define matrices.
    ///
    class Orientation final
    {
    public:
        ///	Constructor: default orientation is Identity.
        Orientation() = default;

        ///	Destructor
        ~Orientation() = default;

        //------------------------------------------------------------------------
        /// \brief  Change current position.
        /// 
        /// \param[in] position: new position.
        void setPosition(const Point3& position)
        {
            _local2world._position = position;
            _world2Local = Transform::inverse(_local2world);
        }

        void setHomothetie(const Point3& scaling)
        {
            _local2world._homothetie = scaling;
            _world2Local = Transform::inverse(_local2world);
        }

        void setQuaternion(const glm::quat& quaternion)
        {
            _local2world._rotation = quaternion;
            _world2Local = Transform::inverse(_local2world);
        }

        //------------------------------------------------------------------------
        /// \brief  Get current position.
        /// 
        /// \returns Current position.
        const Point3& getPosition() const
        {
            return _local2world._position;
        }

        //------------------------------------------------------------------------
        /// \brief  Get current rotation.
        /// 
        /// \returns Current rotation.
        const glm::quat& getQuaternion() const
        {
            return _local2world._rotation;
        }

        //------------------------------------------------------------------------
        /// \brief  Get matrix world to local transformation.
        /// 
        /// \returns Matrix world to local.
        const Transform& getWorldToLocal() const
        {
            return _world2Local;
        }

        //------------------------------------------------------------------------
        /// \brief  Get matrix world to local transformation.
        /// 
        /// \returns Matrix world to local.
        void setWorldToLocal(const Transform& world2Local)
        {
            _world2Local = world2Local;
            _local2world = Transform::inverse(_world2Local);
        }

        //------------------------------------------------------------------------
        /// \brief  Get matrix local to world transformation.
        /// 
        /// \returns Matrix local to world.
        const Transform& getLocalToWorld() const
        {
            return _local2world;
        }

        //------------------------------------------------------------------------
        /// \brief  Print on standard output the representation of instance (both matrices).
        /// 
        void print()
        {
            std::cout << std::fixed << std::setprecision(4) << std::setw(6) << std::setfill('0');
            std::cout << "W2L: " << std::endl
                << "   Position:   " << _world2Local._position[0] << ", " << _world2Local._position[1] << ", " << _world2Local._position[2] << std::endl
                << "   Quaternion: " << _world2Local._rotation.x << ", " << _world2Local._rotation.y << ", " << _world2Local._rotation.z << ", " << _world2Local._rotation.w << " |" << std::endl
                << "   Scaling:    " << _world2Local._homothetie[0] << ", " << _world2Local._homothetie[1] << ", " << _world2Local._homothetie[2] << std::endl;
            std::cout << "L2W: " << std::endl
                << "   Position:   " << _local2world._position[0] << ", " << _local2world._position[1] << ", " << _local2world._position[2] << std::endl
                << "   Quaternion: " << _local2world._rotation.x << ", " << _local2world._rotation.y << ", " << _local2world._rotation.z << ", " << _local2world._rotation.w << " |" << std::endl
                << "   Scaling:    " << _local2world._homothetie[0] << ", " << _local2world._homothetie[1] << ", " << _local2world._homothetie[2] << std::endl;
        }

    private:
        RT::Transform   _world2Local;
        RT::Transform   _local2world;
    };

}