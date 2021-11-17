/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibCore/include/CoreMaths.h>

namespace RT
{
    //----------------------------------------------------------------------------
    /// \brief Mathematic function and helper using glm.
    ///
    struct Maths
    {
        static glm::quat rotateFromTo(const glm::vec3& direction, const glm::vec3& up)
        {
            glm::mat3 Result;

            Result[2] = -direction;
            Result[0] = glm::normalize(glm::cross(up, Result[2]));
            Result[1] = glm::cross(Result[2], Result[0]);

            return glm::quat_cast(Result);
        }

        static glm::vec2 lerp(const glm::vec2& a, const glm::vec2& b, const float i)
        {
            return a + (b - a) * i;
        }

        /// Compute signed distance between P and AB line.
        static float signedDistanceToLine(const glm::vec2& a, const glm::vec2& b, const glm::vec2& p)
        {
            const glm::vec2 lineDir(b - a);
            MouCa::assertion(glm::dot(lineDir, lineDir) > 0.0f); // DEV Issue: Check line exists !

            // Compute normal (basic CCW 90deg rotation)
            const glm::vec2 normal = glm::normalize(glm::vec2(-lineDir.y, lineDir.x));
            return glm::dot(normal, a - p);
        }

        /// Compute normalized position from A point of P on AB segment.
        /// \note Return ALWAYS number between [0, 1]
        static float positionOnLine(const glm::vec2& a, const glm::vec2& b, const glm::vec2& p)
        {
            const glm::vec2 ab(b - a);
            const glm::vec2 ap(p - a);
            // Force to be in [0, 1 range]
            return std::clamp(glm::dot(ap, ab) / glm::dot(ab, ab), 0.0f, 1.0f);
        }

        /// Compute upper or equal pow of two from number.
        /// Ex: 4 = upperPowOfTwo(3);
        ///     4 = upperPowOfTwo(4);
        ///     8 = upperPowOfTwo(5);
        template<typename DataType>
        static constexpr DataType upperPowOfTwo(DataType v)
        {
            static_assert(!std::numeric_limits<DataType>::is_signed);
            static_assert(std::numeric_limits<DataType>::is_integer);
            v--;
            v |= v >> 1;
            v |= v >> 2;
            v |= v >> 4;
            if constexpr (sizeof(DataType) > 1)
                v |= v >> 8;
            if constexpr (sizeof(DataType) > 2)
                v |= v >> 16;
            if constexpr (sizeof(DataType) > 4)
                v |= v >> 32;
            v++;
            return v;
        }

//         static template <typename T> int sgn(T val)
//         {
//             return (T(0) < val) - (val < T(0));
//         }

        template <typename T>
        static constexpr T sgn(const T val)
        {
            return (T(0) < val) ? T(-1) : T(1);
        }

        class Bezier3
        {
            public:
            static std::array<float, 3> cubicRoots(const glm::vec4& P, uint32_t& nbSolutions)
            {
                float A=P[1]/P[0];
                float B=P[2]/P[0];
                float C=P[3]/P[0];
 
                float Im;
 
                float Q = B - std::pow(A, 2.0f)/9.0f;
                float R = 0.5f*(A*B - C) - std::pow(A, 3.0f)/27.0f;
                float D = std::pow(Q, 3.0f) + std::pow(R, 2.0f);    // polynomial discriminant
 
                const float A_3 = A / 3.0f;

                std::array<float, 3> t;
                nbSolutions = 3;
                if (D >= 0)                                 // complex or duplicate roots
                {
                    const float sD = std::sqrt(D);
                    const float pRD = R + sD;
                    const float mRD = R - sD;

                    float S = sgn(pRD) * std::pow( std::abs(pRD), 1.0f/3.0f );
                    float T = sgn(mRD) * std::pow( std::abs(mRD), 1.0f/3.0f );
 
                    t[0] = -A_3 + (S + T);                        // real root
                    t[1] = -A_3 - (S + T) * 0.5f;                 // real part of complex root
                    t[2] = -A_3 - (S + T) * 0.5f;                 // real part of complex root
                    Im = std::abs(std::sqrt(3.0f)*(S - T)* 0.5f); // complex part of root pair   
 
                    /*discard complex roots*/
                    if (Im!=0.0f)
                    {
                        t[1]=-1.0f;
                        t[2]=-1.0f;
                    }
 
                }
                else                                          // distinct real roots
                {
                    constexpr float PI2_3 = 2.0f * Core::Maths::PI<float> / 3.0f;
                    constexpr float PI4_3 = 4.0f * Core::Maths::PI<float> / 3.0f;
                    const float sQ2 = 2.0f * std::sqrt(-Q);
                    const float th = std::acos(R/std::sqrt(-std::pow(Q, 3.0f)))/3.0f;
 
                    t[0] = sQ2 * std::cos(th) - A_3;
                    t[1] = sQ2 * std::cos(th + PI2_3) - A_3;
                    t[2] = sQ2 * std::cos(th + PI4_3) - A_3;
                    Im = 0.0f;
                }
 
                /*discard out of spec roots*/
                for (int i = 0; i < 3; i++)
                {
                    if (t[i] < 0.0f || t[i] > 1.0f)
                    {
                        t[i] = -1.0f;
                        --nbSolutions;
                    }
                }
                return t;
            }

            static std::array<glm::vec2, 4> bezierCoeffs(const std::array<glm::vec2, 4>& P)
            {
                return {
                -P[0] + 3.0f * (P[1] - P[2]) + P[3],
                3.0f * (P[0] - 2.0f * P[1] + P[2]),
                (-P[0] + P[1]),
                P[0]
                };
            }

            static std::array<glm::vec2, 3> computeIntersections(const std::array<glm::vec2, 4>& bezierPts, const std::array<glm::vec2, 2>& line, uint32_t& nbSolutions, const float epsilon = 1e-5f)
            {
                std::array<glm::vec2, 3> X = {};
 
                const glm::vec2 displacement(
                    line[1].y - line[0].y,                            //A=y2-y1
                    line[0].x - line[1].x);                           //B=x1-x2
                float C = glm::dot(line[0], -displacement);	          //C=x1*(y1-y2)+y1*(x2-x1)
 
                const auto coeffs = bezierCoeffs(bezierPts);
                const glm::vec4 P(glm::dot(displacement, coeffs[0]),       // t^3
                                  glm::dot(displacement, coeffs[1]),       // t^2
                                  glm::dot(displacement, coeffs[2]),       // t
                                  glm::dot(displacement, coeffs[3]) + C);  // 1
                
                // No solution
                if (abs(P[0]) < epsilon)
                {
                    nbSolutions = 0;
                    return X;
                }

                auto r = cubicRoots(P, nbSolutions);
 
                /*verify the roots are in bounds of the linear segment*/	
                for (int i=0;i<3;i++)
                {
                    float t = r[i];
                    if(t <= -1.0f)
                        continue;
                    X[i] = ((coeffs[0]*t+coeffs[1])*t+3.0f *coeffs[2])*t+coeffs[3];
                }
                return X;
            }
        };
    };
}