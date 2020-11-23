#pragma once

#include <LibGUI/include/GUIBezier.h>

#define FD_OUTLINE_MAX_POINTS (255 * 2)

namespace GUI
{
	class Outline
	{
		public:
			//------------------------------------------------------------------------
			enum class Code : uint8_t
			{
				Line		= 0x0,
				Quadratic   = 0x1,
				Cubic		= 0x2,
				EndCurve	= 0x4
			};

			struct Point
			{
				static const size_t shift = 3;
				alignas(8) glm::vec2	_point;
				alignas(4) uint32_t	_codeControl; ///< Code (4bits) + index of control (28bits)

				Point(const glm::vec2& point, const Code codeControl, const uint32_t index):
				_point(point), _codeControl(static_cast<uint32_t>(codeControl) | (index << shift))
				{}

				void setIndex(const uint32_t index)
				{
					_codeControl = (_codeControl & 0x7) | index << shift;
				}

				Code getCode() const
				{
					return static_cast<Code>(_codeControl & 0x7);
				}

				uint32_t getIndex() const
				{
					return _codeControl >> shift;
				}
			};

            struct ControlPoint
            {
				glm::vec2 _controlPoint;
				ControlPoint(const glm::vec2& controlPoint):
				_controlPoint(controlPoint)
				{}
            };

			//------------------------------------------------------------------------

            struct ContourRange
            {
				uint32_t _begin;
				uint32_t _end;

				ContourRange(const uint32_t begin=0, const uint32_t end=0):
				_begin(begin), _end(end)
				{}
            };

			void convert(FT_Outline& outline);

			const BoundingBox2D& getBoundingBox() const
			{
				return _bbox;
			}

			const std::vector<glm::vec2>& getPoints() const
			{
				return _points;
			}

            const std::vector<uint32_t>& getCells() const
            {
                return _cells;
            }

			uint32_t getCellCountX() const
			{
				return _cell_count_x;
			}

            uint32_t getCellCountY() const
            {
                return _cell_count_y;
            }

			const std::vector<Point>& getOutlinePoint() const
			{
				return _outlinePoints;
			}

            const std::vector<ControlPoint>& getControlPoint() const
            {
                return _controlPoints;
            }

			Outline():
			_cell_count_x(0), _cell_count_y(0)
			{}

			static const float _scale;
			static const float _scale2;

		private:
			void decompose(FT_Outline& outline);
			void fixThinLines();
			void makeCells();

			bool tryFitInCell();

			uint32_t addFilledLine(std::vector<glm::vec2>& newPoints) const; 

			static int FTMove(const FT_Vector* point, Outline* outline);
			static int FTLine(const FT_Vector* point, Outline* outline);
			static int FTConic(const FT_Vector* control, const FT_Vector* point, Outline* o);
			static int FTCubic(const FT_Vector* control1, const FT_Vector* control2, const FT_Vector* to, Outline* o);

			BoundingBox2D				_bbox;
			std::vector<glm::vec2>		_points;
			std::vector<ContourRange>	_contours;

            std::vector<uint32_t> _cells;
            uint32_t _cell_count_x;
            uint32_t _cell_count_y;

			std::vector<Point>			_outlinePoints;
			std::vector<ControlPoint>	_controlPoints;
	};
}