/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

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
            MOUCA_ASSERT(glm::dot(lineDir, lineDir) > 0.0f); // DEV Issue: Check line exists !

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
            static std::array<float, 3> cubicRoots(const std::array<float, 4>& P, uint32_t& nbSolutions)
            {
                float a=P[0];
                float b=P[1];
                float c=P[2];
                float d=P[3];
 
                float A=b/a;
                float B=c/a;
                float C=d/a;
 
                float Im;
 
                float Q = (3.0f*B - std::pow(A, 2.0f))/9.0f;
                float R = (9.0f*A*B - 27.0f*C - 2.0f*std::pow(A, 3.0f))/54.0f;
                float D = std::pow(Q, 3.0f) + std::pow(R, 2.0f);    // polynomial discriminant
 
                std::array<float, 3> t;
                nbSolutions = 3;
                if (D >= 0)                                 // complex or duplicate roots
                {
                    float S = sgn(R + std::sqrt(D)) * std::pow( std::abs(R + std::sqrt(D)),(1.0f/3.0f));
                    float T = sgn(R - std::sqrt(D)) * std::pow( std::abs(R - std::sqrt(D)),(1.0f/3.0f));
 
                    t[0] = -A/3 + (S + T);                    // real root
                    t[1] = -A/3 - (S + T)/2;                  // real part of complex root
                    t[2] = -A/3 - (S + T)/2;                  // real part of complex root
                    Im = std::abs(std::sqrt(3.0f)*(S - T)/2.0f);    // complex part of root pair   
 
                    /*discard complex roots*/
                    if (Im!=0.0f)
                    {
                        t[1]=-1.0f;
                        t[2]=-1.0f;
                    }
 
                }
                else                                          // distinct real roots
                {
                    float th = std::acos(R/std::sqrt(-std::pow(Q, 3.0f)));
 
                    t[0] = 2.0f * std::sqrt(-Q)*std::cos(th/3.0f) - A/3.0f;
                    t[1] = 2.0f * std::sqrt(-Q)*std::cos((th + 2.0f * Core::Maths::PI<float>)/3.0f) - A/3.0f;
                    t[2] = 2.0f * std::sqrt(-Q)*std::cos((th + 4.0f * Core::Maths::PI<float>)/3.0f) - A/3.0f;
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

            static std::array<float, 4> bezierCoeffs(std::array<float, 4> P)
            {
                return {
                -P[0] + 3 * P[1] + -3 * P[2] + P[3],
                3 * P[0] - 6 * P[1] + 3 * P[2],
                -3 * P[0] + 3 * P[1],
                P[0]
                };
            }

            static std::array<float, 6> computeIntersections(std::array<float, 4> px, std::array<float, 4>py, std::array<float, 2>lx, std::array<float, 2>ly, uint32_t& nbSolutions, const float epsilon = 1e-5f)
            {
                std::array<float, 6> X = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
 
                float A=ly[1]-ly[0];	    //A=y2-y1
                float B=lx[0]-lx[1];	    //B=x1-x2
                float C=lx[0]*(ly[0]-ly[1]) + 
                      ly[0]*(lx[1]-lx[0]);	//C=x1*(y1-y2)+y1*(x2-x1)
 
                std::array<float, 4> bx = bezierCoeffs(px);
                std::array<float, 4> by = bezierCoeffs(py);
 
                std::array<float, 4> P
                {
                    A * bx[0] + B * by[0],		/*t^3*/
                    A * bx[1] + B * by[1],		/*t^2*/
                    A * bx[2] + B * by[2],		/*t*/
                    A * bx[3] + B * by[3] + C 	/*1*/
                };
                
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

                    X[i*2]  = bx[0]*t*t*t+bx[1]*t*t+bx[2]*t+bx[3];
                    X[i*2+1]= by[0]*t*t*t+by[1]*t*t+by[2]*t+by[3];            
                }
                return X;
            }
        };
    };
}