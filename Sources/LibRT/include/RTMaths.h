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
            BT_ASSERT(glm::dot(lineDir, lineDir) > 0.0f); // DEV Issue: Check line exists !

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
    };
}