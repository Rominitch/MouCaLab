#pragma once

#include <LibRT/include/RTMaths.h>

namespace GUI
{
    struct BoundingBox2D
    {
        BoundingBox2D(const glm::vec2& min = glm::vec2(), const glm::vec2& max = glm::vec2()):
        _min(min), _max(max)
        {}

        inline bool isInside(const glm::vec2& point) const
        {
            return glm::all(glm::lessThanEqual(_min, point)) && glm::all(glm::lessThanEqual(point, _max));
        }

        inline bool isInsideExclusive(const glm::vec2& point) const
        {
            return glm::all(glm::lessThan(_min, point)) && glm::all(glm::lessThan(point, _max));
        }

        glm::vec2 getSize() const
        {
            return _max - _min;
        }

        alignas(8) glm::vec2 _min;
        alignas(8) glm::vec2 _max;
    };

	class BezierCurve
	{
        public:
            BezierCurve(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c);

            BoundingBox2D computeBbox() const;

            /// Compute intersection between aligned bbox and bezier curve
		    bool hasIntersection(const BoundingBox2D& bbox) const;

            /// Compute intersection between line and bezier curve
            bool hasIntersection(const glm::vec2& line0, const glm::vec2& line1) const;

            inline glm::vec2 interpolate(const float t) const
            {
                const glm::vec2 q0 = RT::Maths::lerp(_control[0], _control[1], t);
                const glm::vec2 q1 = RT::Maths::lerp(_control[1], _control[2], t);
                return RT::Maths::lerp(q0, q1, t);
            }

            // Interpolate curve based on control point + interpolation and return intermediate state and final position.
            inline glm::vec2 interpolate(const float t, glm::vec2& q0, glm::vec2& q1) const
            {
                q0 = RT::Maths::lerp(_control[0], _control[1], t);
                q1 = RT::Maths::lerp(_control[1], _control[2], t);
                return RT::Maths::lerp(q0, q1, t);
            }

        private:
            using Bezier3Control = std::array<glm::vec2, 3>;
            const Bezier3Control _control; /// Bezier: using 3 controller point.

            enum class SolveSolution : uint8_t
            {
                QUADRATIC_SOLUTION_NONE,
                QUADRATIC_SOLUTION_ALL,
                QUADRATIC_SOLUTION_TOUCH,
                QUADRATIC_SOLUTION_ONE,
                QUADRATIC_SOLUTION_TWO
            };

            inline std::array<glm::vec2, 2> derivative() const
            {
                return { (_control[1] - _control[0]) * 2.0f, (_control[2] - _control[1]) * 2.0f };
            }

            /// Compute oriented line to Length/Sin/Cos
            static inline void alignedLSC(const glm::vec2& p0, const glm::vec2& p1, float& l, float& s, float& c)
            {
                const glm::vec2 dir(p1 - p0);

                l = glm::length(dir);
                s = -dir.y / l;
                c = dir.x / l;
            }

            /// Compute point to new space-coord (line + sin/cos)
            static inline glm::vec2 alignedPoint(const glm::vec2& p, const glm::vec2& origin, const float s, const float c)
            {
                const glm::vec2 dir(p - origin);
                const glm::vec2 sDir = dir * s;
                const glm::vec2 cDir = dir * c;
                return { cDir.x - sDir.y, sDir.x + cDir.y };

                //return { dir.x * c - dir.y * s, dir.x * s + dir.y * c };
            }

            static SolveSolution solveQuadratic(const float a, const float b, const float c, float& x1, float& x2);

            static constexpr float component(float p0, float p1, float p2, float t)
            {
                return Core::Maths::lerp(Core::Maths::lerp(p0, p1, t), Core::Maths::lerp(p1, p2, t), t);
            }
	};
}