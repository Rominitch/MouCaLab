/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace Core
{
    //----------------------------------------------------------------------------
    /// \brief Mathematic function missing into cmath standard.
    ///
    namespace Maths
    {
        //------------------------------------------------------------------------
        /// \brief  Get PI value.
        /// 
        template<typename DataType>
        constexpr DataType PI = static_cast<DataType>(3.14159265358979323846);

        //------------------------------------------------------------------------
        /// \brief  Convert angle from degree to radian.
        ///
        /// \param[in] angleDeg: degree angle to convert.
        /// \returns Angle in radian.
        template<typename DataType>
        static DataType toRad(DataType angleDeg)
        {
            static_assert(!std::numeric_limits<DataType>::is_integer);
            return angleDeg * PI<DataType> / static_cast<DataType>(180.0);
        }

        //------------------------------------------------------------------------
        /// \brief  Convert angle from radian to degree.
        ///
        /// \param[in] angleRad: radian angle to convert.
        /// \returns Angle in degree.
        template<typename DataType>
        static DataType toDeg(DataType angleRad)
        {
            static_assert(!std::numeric_limits<DataType>::is_integer);
            return angleRad * static_cast<DataType>(180.0) / PI<DataType>;
        }

        template<typename DataType, typename FloatData>
        constexpr static DataType lerp(const DataType& a, const DataType& b, const FloatData i)
        {
            return a + (b - a) * i;
        }

        //------------------------------------------------------------------------
        /// \brief  Round upper the result of division.
        ///
        /// \param[in] value: value to divide.
        /// \param[in] divider: divider.
        /// \returns Round upper.
        template<typename DataType>
        constexpr static DataType roundUpperDiv(const DataType value, const DataType divider)
        {
            static_assert(std::numeric_limits<DataType>::is_integer);
            MOUCA_ASSERT(divider > 0);

            return 1 + ((value - 1) / divider);
        }

        //------------------------------------------------------------------------
        /// \brief Compute a normalized radian angle into range [0; 2 PI[.
        /// 
        /// \param[in] angleRad: angle to normalize in radian.
        /// \returns New angle normalized
        template<typename DataType>
        static DataType angleCyclic(DataType angleRad)
        {
            MOUCA_PRE_CONDITION(!std::numeric_limits<DataType>::is_integer);

            const DataType f2PI = PI<DataType> * static_cast<DataType>(2.0);
            bool normalized;
            do 
            {
                if(angleRad < static_cast<DataType>(0.0))
                {
                    angleRad += f2PI;
                    normalized = false;
                }
                else if(angleRad >= f2PI)
                {
                    angleRad -= f2PI;
                    normalized = false;
                }
                else
                {
                    normalized = true;
                }
            }
            while(!normalized);
            MOUCA_POST_CONDITION(static_cast<DataType>(0.0) <= angleRad && angleRad < f2PI); //DEV Issue: more than 1 multiple ?
            return angleRad;
        }

        //------------------------------------------------------------------------
        /// \brief Compute a positive modulo result.
        /// 
        /// \param[in] x: number to manage.
        /// \param[in] m: divider.
        /// \returns Positive modulo result.
        template<typename DataType>
        static DataType modPositive(DataType x, DataType m)
        {
            return (x % m + m) % m;
        }

        template<typename DataType>
        static constexpr bool isPowerOfTwo(const DataType number)
        {
            static_assert(std::numeric_limits<DataType>::is_integer);
            return (number > 0) && !(number & (number - 1));
        }

        template<typename DataType>
        static constexpr bool isBetween(const DataType value, const DataType min, const DataType max)
        {
            return min <= value && value <= max;
        }
    }
}
