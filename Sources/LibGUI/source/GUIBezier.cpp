#include "Dependencies.h"

#include "LibRT/include/RTMaths.h"

#include "LibGUI/include/GUIBezier.h"

namespace GUI
{
	BezierCurve::BezierCurve(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c):
	_control({a,b,c})
	{}

    bool BezierCurve::hasIntersection(const BoundingBox2D& bbox) const
    {
        if (bbox.isInsideExclusive(_control[0]) || bbox.isInsideExclusive(_control[2]))
        {
            return true;
        }

        const std::array<glm::vec2, 4> corners =
        {
            bbox._min,						// Bottom-Left
            { bbox._max.x, bbox._min.y },	// Bottom Right
            bbox._max,						// Top-Right
            { bbox._min.x, bbox._max.y },   // Top-Left
        };

        // Search first intersection
        return hasIntersection(corners[0], corners[1]) ||
               hasIntersection(corners[1], corners[2]) ||
               hasIntersection(corners[2], corners[3]) ||
               hasIntersection(corners[3], corners[0]);
    }

    bool BezierCurve::hasIntersection(const glm::vec2& line0, const glm::vec2& line1) const
    {
        float l, si, co;
		alignedLSC(line0, line1, l, si, co);

		/// Change space coordinate
		const Bezier3Control bez =
		{
			alignedPoint(_control[0], line0, si, co),
			alignedPoint(_control[1], line0, si, co),
			alignedPoint(_control[2], line0, si, co)
		};

        float a = bez[0].y - 2 * bez[1].y + bez[2].y;
        float b = 2 * (bez[1].y - bez[0].y);
        float c = bez[0].y;

        float t0, t1;
		const SolveSolution sol = solveQuadratic(a, b, c, t0, t1);

        switch(sol)
        {
			case SolveSolution::QUADRATIC_SOLUTION_NONE:
			case SolveSolution::QUADRATIC_SOLUTION_ALL:
				return false;

			case SolveSolution::QUADRATIC_SOLUTION_TOUCH:
			case SolveSolution::QUADRATIC_SOLUTION_ONE:
            {
                const float xt0 = component(bez[0].x, bez[1].x, bez[2].x, t0);
                return Core::Maths::isBetween(t0, 0.0f, 1.0f) && Core::Maths::isBetween(xt0, 0.0f, l);
            }

			case SolveSolution::QUADRATIC_SOLUTION_TWO:
            {
                const float xt0 = component(bez[0].x, bez[1].x, bez[2].x, t0);
                const float xt1 = component(bez[0].x, bez[1].x, bez[2].x, t1);

                return (Core::Maths::isBetween(t0, 0.0f, 1.0f) && Core::Maths::isBetween(xt0, 0.0f, l)) ||
                       (Core::Maths::isBetween(t1, 0.0f, 1.0f) && Core::Maths::isBetween(xt1, 0.0f, l));
            }

			default:
				MouCa::assertion(false); // Unknown state : missing enum ?
				return false;
        }
    }

    BezierCurve::SolveSolution BezierCurve::solveQuadratic(const float a, const float b, const float c, float& x1, float& x2)
    {
        float discriminant = b * b - 4.0f * a * c;

        if (discriminant > 0.0f)
        {
            float sqrt_d = std::sqrt(discriminant);
            float common = b >= 0.0f ? -b - sqrt_d : -b + sqrt_d;

            x1 = 2.0f * c / common;

            if (a == 0.0f)
                return SolveSolution::QUADRATIC_SOLUTION_ONE;

            x2 = common / (2.0f * a);
            return SolveSolution::QUADRATIC_SOLUTION_TWO;
        }
        else if (discriminant == 0.0f)
        {
            if (b == 0.0f)
            {
                if (a == 0.0f)
                {
                    if (c == 0.0f) return SolveSolution::QUADRATIC_SOLUTION_ALL;
                    else return SolveSolution::QUADRATIC_SOLUTION_NONE;
                }
                else
                {
                    x1 = 0.0f;
                    return SolveSolution::QUADRATIC_SOLUTION_TOUCH;
                }
            }
            else
            {
                x1 = 2.0f * c / -b;
                return SolveSolution::QUADRATIC_SOLUTION_TOUCH;
            }
        }

        return SolveSolution::QUADRATIC_SOLUTION_NONE;
    }

	BoundingBox2D BezierCurve::computeBbox() const
	{
		BoundingBox2D bbox;

		const std::array<glm::vec2, 2> deriv = derivative();
        float tx = deriv[0][0] / (deriv[0][0] - deriv[1][0]);
        float ty = deriv[0][1] / (deriv[0][1] - deriv[1][1]);

        bbox._min.x = std::min(_control[0][0], _control[2][0]);
        bbox._min.y = std::min(_control[0][1], _control[2][1]);
        bbox._max.x = std::max(_control[0][0], _control[2][0]);
        bbox._max.y = std::max(_control[0][1], _control[2][1]);

        if (0.0f <= tx && tx <= 1.0f)
        {
            float x = component(_control[0][0], _control[1][0], _control[2][0], tx);

            if (deriv[0][0] < deriv[1][0])
                bbox._min.x = std::min(bbox._min.x, x);
            else
                bbox._max.x = std::max(bbox._max.x, x);
        }

        if (0.0f <= ty && ty <= 1.0f)
        {
            float y = component(_control[0][1], _control[1][1], _control[2][1], ty);

            if (deriv[0][1] < deriv[1][1])
                bbox._min.y = std::min(bbox._min.y, y);
            else
                bbox._max.y = std::max(bbox._max.y, y);
        }

		return bbox;
	}
}