#include "Dependancies.h"

#include "LibRT/include/RTMaths.h"

#include "LibGUI/include/GUIOutline.h"
#include "LibGUI/include/GUIBezier.h"

//#define MOUCA_VALIDATION

namespace GUI
{

const float Outline::_scale = 1.0f / 64.0f;
const float Outline::_scale2 = 1.0f / 6400.0f;

void Outline::convert(FT_Outline& outline)
{
    if (outline.n_points > 0)
    {
		decompose(outline);
		//fixThinLines();
		//makeCells();
	}
    // Validation data
#ifdef MOUCA_VALIDATION
    size_t startLoop = 0;
    size_t id = 0;
    for(const auto& pts : _outlinePoints)
    {
        if (pts.getCode() == Code::EndCurve)
        {
            std::cout << "Curve: " << startLoop << " to " << id << std::endl;
            BT_ASSERT(startLoop==0 || startLoop+1 == pts.getIndex());
            BT_ASSERT(pts.getIndex() < _outlinePoints.size());

            startLoop = id;
        }
        else
        {
            const uint32_t nbCtrl = static_cast<uint32_t>(pts.getCode()) & 0x3;
            for (uint32_t ctrl = 0; ctrl < nbCtrl; ++ctrl)
            {
                BT_ASSERT( (pts.getIndex() + ctrl) < _controlPoints.size() );
            }
        }
        ++id;
    }

#endif
}

static size_t latestCurve;

glm::vec2 scale(const glm::vec2& pts, Outline* o)
{
    return pts;// (pts - o->_bboxv2._min) / o->_bboxv2.getSize();
}

void Outline::decompose(FT_Outline& outline)
{
    FT_BBox outline_bbox;
    if (FT_Outline_Get_BBox(&outline, &outline_bbox))
    {
        BT_THROW_ERROR(u8"FontError", u8"GetBBoxError");
    }

	_bbox = BoundingBox2D(glm::vec2(outline_bbox.xMin, outline_bbox.yMin) * _scale, glm::vec2(outline_bbox.xMax , outline_bbox.yMax) * _scale);
    //_bbox = BoundingBox2D(glm::vec2(outline_bbox.xMin, outline_bbox.yMin) * _scale2, glm::vec2(outline_bbox.xMax, outline_bbox.yMax) * _scale2);

    latestCurve = 0;

    FT_Outline_Funcs funcs =
    {
        reinterpret_cast<FT_Outline_MoveToFunc>(FTMove),
        reinterpret_cast<FT_Outline_LineToFunc>(FTLine),
        reinterpret_cast<FT_Outline_ConicToFunc>(FTConic),
        reinterpret_cast<FT_Outline_CubicToFunc>(FTCubic),
        NULL,
		NULL
    };

    if (FT_Outline_Decompose(&outline, &funcs, this))
    {
        BT_THROW_ERROR(u8"FontError", u8"OutlineDecomposeError");
    }

    // Final loop point
    _outlinePoints.emplace_back(Point(glm::vec2(0.0f), Code::EndCurve, static_cast<uint32_t>(latestCurve)));

    //if (!_contours.empty())
    //{
    //    _contours.back()._end = static_cast<uint32_t>(_points.size() - 1);
    //}
}

int Outline::FTMove(const FT_Vector* point, Outline* o)
{
    //if ( !o->_contours.empty() )
    //{
    //    o->_contours.back()._end = static_cast<uint32_t>(o->_points.size() - 1);
	//	o->_points.emplace_back(glm::vec2(0.0f, 0.0f));
    //}

    const glm::vec2 to_p = glm::vec2(point->x, point->y);

    //BT_ASSERT(o->_points.size() % 2 == 0);

	//o->_contours.emplace_back(ContourRange(static_cast<uint32_t>(o->_points.size()), UINT32_MAX));
    //o->_points.emplace_back(to_p * _scale);

    // New V2
    o->_outlinePoints.emplace_back(Point(scale(to_p * _scale2, o), Code::EndCurve, static_cast<uint32_t>(latestCurve)));
    latestCurve = o->_outlinePoints.size();
    return 0;
}

int Outline::FTLine(const FT_Vector* point, Outline* o)
{
    const glm::vec2 to_p = glm::vec2(point->x, point->y);
	//const glm::vec2 p    = RT::Maths::lerp(o->_points.back(), to_p * _scale, 0.5f);
	//o->_points.emplace_back(p);
	//o->_points.emplace_back(to_p * _scale);

    //o->_outlinePoints[latestCurve].setIndex(static_cast<uint32_t>(o->_outlinePoints.size()));
    o->_outlinePoints.emplace_back(Point(scale(to_p * _scale2, o), Code::Line, 0));

    return 0;
}

int Outline::FTConic(const FT_Vector* control, const FT_Vector* point, Outline* o)
{
    const glm::vec2 to_p = glm::vec2(point->x, point->y);

	//o->_points.emplace_back(glm::vec2(control->x, control->y) * _scale);
	//o->_points.emplace_back(to_p * _scale);

    //o->_outlinePoints[latestCurve].setIndex(static_cast<uint32_t>(o->_outlinePoints.size()));
    o->_outlinePoints.emplace_back(Point(scale(to_p * _scale2, o), Code::Quadratic, static_cast<uint32_t>(o->_controlPoints.size())));
    o->_controlPoints.emplace_back(scale(glm::vec2(control->x, control->y) * _scale2, o));

    return 0;
}

int Outline::FTCubic(const FT_Vector* control1, const FT_Vector* control2, const FT_Vector* to, Outline* o)
{
    const glm::vec2 to_p = glm::vec2(to->x, to->y);
    //const glm::vec2 p = RT::Maths::lerp(o->_points.back(), to_p * _scale, 0.5f);
    //o->_points.emplace_back(p);
    //o->_points.emplace_back(to_p * _scale);

    //o->_outlinePoints[latestCurve].setIndex(static_cast<uint32_t>(o->_outlinePoints.size()));
    o->_outlinePoints.emplace_back(Point(scale(to_p * _scale2, o), Code::Cubic, static_cast<uint32_t>(o->_controlPoints.size())));
    o->_controlPoints.emplace_back(scale(glm::vec2(control1->x, control1->y) * _scale2, o));
    o->_controlPoints.emplace_back(scale(glm::vec2(control2->x, control2->y) * _scale2, o));

    return 0;
}

void Outline::fixThinLines()
{
	//Outline u(_bbox);
	std::vector<glm::vec2> newPoints;

	for(auto& contour : _contours)
	{
        if (newPoints.size() % 2 != 0)
        {
			newPoints.emplace_back(_bbox._min);
        }

		const auto startContour = static_cast<uint32_t>(newPoints.size());

		for (uint32_t i = contour._begin; i < contour._end; i += 2)
		{
			const glm::vec2& p0 = _points[i];
			const glm::vec2& p1 = _points[i + 1];
			const glm::vec2& p2 = _points[i + 2];

			glm::vec2 mid = RT::Maths::lerp(p0, p2, 0.5f);
			glm::vec2 midp1 = p1 - mid;

			BezierCurve bezier(p0, p1 + midp1, p2);
			
			bool subdivide = false;
			for (uint32_t j = contour._begin; j < contour._end; j += 2)
			{
				if (i == contour._begin && j == contour._end - 2) continue;
				if (i == contour._end - 2 && j == contour._begin) continue;
				if (j + 2 >= i && j <= i + 2) continue;

				glm::vec2& q0 = _points[j];
				//float *q1 = o->points[j + 1];
				glm::vec2& q2 = _points[j + 2];

				if (bezier.hasIntersection(q0, q2))
					subdivide = true;
			}

			if (subdivide)
			{
				glm::vec2 q0, q1;
				const glm::vec2 r = bezier.interpolate(0.5f, q0, q1);

				newPoints.emplace_back(p0);
				newPoints.emplace_back(q0);
				newPoints.emplace_back(r);
				newPoints.emplace_back(q1);
			}
			else
			{
				newPoints.emplace_back(p0);
				newPoints.emplace_back(p1);
			}
		}
		const auto endContour = static_cast<uint32_t>(newPoints.size());

		// Add latest points
		newPoints.emplace_back(_points[contour._end]);

        // Change contour limit
        contour._begin = startContour;
        contour._end   = endContour;
	}

	// Keep new points
	_points = std::move(newPoints);
}

void Outline::makeCells()
{
    //if (_points.size() > FD_OUTLINE_MAX_POINTS)
    //    BT_THROW_ERROR("GUI", "MaxPointsReachError");

    float w = _bbox._max.x - _bbox._min.x;
    float h = _bbox._max.y - _bbox._min.y;

    // 	float multiplier = 0.5f;
    // 	if (h > w * 1.8f || w > h * 1.8f)
    // 		multiplier = 1.0f;

    const uint32_t c = RT::Maths::upperPowOfTwo(static_cast<uint32_t>(sqrtf(_points.size() * 0.75f)));

    _cell_count_x = c;
    _cell_count_y = c;

    if (h > w * 1.8f) _cell_count_x /= 2;
    if (w > h * 1.8f) _cell_count_y /= 2;

    while (!tryFitInCell())
    {
        if (_cell_count_x > 64 || _cell_count_y > 64)
        {
            _cell_count_x = 0;
            _cell_count_y = 0;
            return;
        }

        if (_cell_count_x == _cell_count_y)
        {
            if (w > h) _cell_count_x *= 2;
            else _cell_count_y *= 2;
        }
        else
        {
            if (_cell_count_x < _cell_count_y)
                _cell_count_x *= 2;
            else 
				_cell_count_y *= 2;
        }
    }
}

struct Cell
{
	BoundingBox2D _bbox;
    uint32_t _from;
    uint32_t _to;
    uint32_t _value;
    uint32_t _start_len;

	Cell():
	_from(UINT32_MAX), _to(UINT32_MAX), _value(0), _start_len(0)
	{}

    bool addBezier(const uint32_t ucontour_begin, const uint32_t idPoint)
    {
        bool ret = true;

        if (_to != UINT32_MAX && _to != idPoint)
        {
            BT_ASSERT(_to < idPoint);

            if (_from == ucontour_begin)
            {
                BT_ASSERT(_to % 2 == 0);
                BT_ASSERT(_from % 2 == 0);

                _start_len = (_to - _from) / 2;
            }
            else
            {
                _value = addRange(_value, _from, _to);
                if (!_value) ret = false;
            }

            _from = idPoint;
        }
        else
        {
            if (_from == UINT32_MAX)
                _from = idPoint;
        }

        _to = idPoint + 2;
        return ret;
    }

	static uint32_t addRange(uint32_t cell, uint32_t from, uint32_t to)
    {
        BT_ASSERT(from % 2 == 0 && to % 2 == 0);

        from /= 2;
        to   /= 2;

        if (from >= UINT8_MAX) return 0;
        if (to >= UINT8_MAX) return 0;

        uint32_t length = to - from;
        if (length <= 3 && (cell & 0x03) == 0)
        {
            cell |= from << 8;
            cell |= length;
            return cell;
        }

        if (length > 7)
            return 0;

        if ((cell & 0x1C) == 0)
        {
            cell |= from << 16;
            cell |= length << 2;
            return cell;
        }

        if ((cell & 0xE0) == 0)
        {
            cell |= from << 24;
            cell |= length << 5;
            return cell;
        }

        return 0;
    }

    bool finishContour(const Outline::ContourRange& contour, uint32_t& max_start_len)
    {
        bool ret = true;

        if (_to < contour._end)
        {
            _value = addRange(_value, _from, _to);
            if (!_value) ret = false;

            _from = UINT32_MAX;
            _to   = UINT32_MAX;
        }

        BT_ASSERT(_to == UINT32_MAX || _to == contour._end);
        _to = UINT32_MAX;

        if (_from != UINT32_MAX && _start_len != 0)
        {
            _value = addRange(_value, _from, contour._end + _start_len * 2);
            if (!_value) ret = false;

            max_start_len = std::max(max_start_len, _start_len);
            _from = UINT32_MAX;
            _start_len = 0;
        }

        if (_from != UINT32_MAX)
        {
            _value = addRange(_value, _from, contour._end);
            if (!_value) ret = false;

            _from = UINT32_MAX;
        }

        if (_start_len != 0)
        {
            _value = addRange(_value, contour._begin, contour._begin + _start_len * 2);
            if (!_value) ret = false;

            _start_len = 0;
        }

		BT_POST_CONDITION(_from == UINT32_MAX && _to == UINT32_MAX);
        return ret;
    }

    bool isFilled(const std::vector<glm::vec2>& points, const std::vector<Outline::ContourRange>& contours) const
    {
        const glm::vec2 p( (_bbox._max.x + _bbox._min.x) / 2.0f, (_bbox._max.y + _bbox._min.y) / 2.0f);

        float mindist = FLT_MAX;
        float v = FLT_MAX;
        uint32_t j = UINT32_MAX;

        for(const auto& contour : contours)
        {
            for (uint32_t i = contour._begin; i < contour._end; i += 2)
            {
                const glm::vec2& p0 = points[i];
                const glm::vec2& p2 = points[i + 2];

                float t = RT::Maths::positionOnLine(p0, p2, p);

                glm::vec2 p02 = RT::Maths::lerp(p0, p2, t);

                float udist = glm::length(p02 - p);

                if (udist < mindist + 0.0001f)
                {
                    float d = RT::Maths::signedDistanceToLine(p0, p2, p);

                    if (udist >= mindist && i > contour._begin)
                    {
                        float lastd = i == contour._end - 2 && j == contour._begin
                            ? RT::Maths::signedDistanceToLine(p0, p2, points[contour._begin + 2])
                            : RT::Maths::signedDistanceToLine(p0, p2, points[i - 2]);

                        if (lastd < 0.0) v = std::max(d, v);
                        else v = std::min(d, v);
                    }
                    else
                    {
                        v = d;
                    }

                    mindist = std::min(mindist, udist);
                    j = i;
                }
            }
        }

        return v > 0.0f;
    }
};

struct Cells
{
    Cells(const BoundingBox2D& bbox, uint32_t cellX, uint32_t cellY):
	_cells(cellX * cellY)
    {
		BT_PRE_CONDITION(cellX > 0 && cellY > 0);

        const float w = bbox._max.x - bbox._min.x;
		const float h = bbox._max.y - bbox._min.y;
		const glm::vec2 scale(w / static_cast<float>(cellX), h / static_cast<float>(cellY));

		auto itCell = _cells.begin();
        for (uint32_t y = 0; y < cellY; y++)
        {
            for (uint32_t x = 0; x < cellX; x++)
            {
				const glm::vec2 id(static_cast<float>(x), static_cast<float>(y));
                
				itCell->_bbox = BoundingBox2D(bbox._min + id * scale, bbox._min + (id + glm::vec2(1.0f, 1.0f)) * scale);
				++itCell;
            }
        }
		BT_POST_CONDITION(itCell == _cells.end());
    }

    bool for_each_wipcell_add_bezier(const BoundingBox2D& globalBBox, const glm::vec2& cellSize, const BezierCurve& curve, const uint32_t idPoint, const uint32_t ucontour_begin)
    {
        const BoundingBox2D bezier_bbox = curve.computeBbox();

		const auto size = globalBBox.getSize();
        //float outline_bbox_w = globalBBox._max.x - globalBBox._min.x;
        //float outline_bbox_h = globalBBox._max.y - globalBBox._min.y;
		const auto scale = cellSize / size;

        auto min = (bezier_bbox._min - globalBBox._min) * scale;
		auto max = (bezier_bbox._max - globalBBox._min) * scale;
		//BT_ASSERT(glm::all(glm::lessThanEqual(glm::vec2(0.0f, 0.0f), min)) && glm::all(glm::lessThanEqual(min, max)));
        
        if (max.x >= cellSize.x) max.x = cellSize.x - 1;
        if (max.y >= cellSize.y) max.y = cellSize.y - 1;

        bool ret = true;
        for (uint32_t y = static_cast<uint32_t>(min.y); y <= static_cast<uint32_t>(max.y); y++)
        {
            for (uint32_t x = static_cast<uint32_t>(min.x); x <= static_cast<uint32_t>(max.x); x++)
            {
                Cell& cell = _cells[static_cast<size_t>(y * cellSize.x + x)];
                if (curve.hasIntersection(cell._bbox))
                    ret &= cell.addBezier(ucontour_begin, idPoint);
            }
        }

        return ret;
    }

    bool for_each_wipcell_finish_contour(const Outline::ContourRange& contour, uint32_t& max_start_len)
    {
        bool ret = true;
		for(auto& cell : _cells)
        {
            ret &= cell.finishContour(contour, max_start_len);
        }
        return ret;
    }

    void fill(const std::vector<glm::vec2>& points, const std::vector<Outline::ContourRange>& contours, const uint32_t filled_cell)
    {
		for (auto& cell : _cells)
		{
            if (cell._value == 0 && cell.isFilled(points, contours))
                cell._value = filled_cell;
		}
    }

	std::vector<Cell> _cells;
};

bool Outline::tryFitInCell() 
{
    bool ret = true;

	Cells cells(_bbox, _cell_count_x, _cell_count_y);

//     Outline u(o->_bbox);
//     u.cell_count_x = o->cell_count_x;
//     u.cell_count_y = o->cell_count_y;

	std::vector<glm::vec2>		newPoints;
	std::vector<ContourRange>	newContours;

	uint32_t contourIndex = 0;
	for(const auto& contour : _contours)
    {
        if (newPoints.size() % 2 != 0)
        {
            newPoints.emplace_back(_bbox._min);
        }

		auto& currentContour = newContours.emplace_back(ContourRange(static_cast<uint32_t>(newPoints.size()), static_cast<uint32_t>(newPoints.size()) + contour._end - contour._begin));

        for (uint32_t i = contour._begin; i < contour._end; i += 2)
        {
            const uint32_t j = static_cast<uint32_t>(newPoints.size());
			newPoints.emplace_back(_points[i]);
			newPoints.emplace_back(_points[i+1]);

			const BezierCurve bezier(_points[i], _points[i + 1], _points[i + 2]);
            ret &= cells.for_each_wipcell_add_bezier(_bbox, glm::vec2(_cell_count_x, _cell_count_y), bezier, j, currentContour._begin);
        }

        uint32_t max_start_len = 0;
        ret &= cells.for_each_wipcell_finish_contour(currentContour, max_start_len);

        const uint32_t continuation_end = contour._begin + max_start_len * 2;
        for (uint32_t i = contour._begin; i < continuation_end; i += 2)
        {
			newPoints.emplace_back(_points[i]);
			newPoints.emplace_back(_points[i + 1]);
        }
		newPoints.emplace_back(_points[continuation_end]);

		++contourIndex;
    }

    if (!ret)
    {
        return ret;
    }

	// Fill cells
    uint32_t filled_line = addFilledLine(newPoints);
    BT_ASSERT(filled_line % 2 == 0);
	uint32_t filled_cell = filled_line << 7 | 1;
	cells.fill(newPoints, newContours, filled_cell);

    //copy_wipcell_values(&u, cells.data());
	// Move/Copy data inplace
	_points   = std::move(newPoints);
	_contours = std::move(newContours);
	_cells.resize(_cell_count_x * _cell_count_y);
	std::transform(cells._cells.cbegin(), cells._cells.cend(), _cells.begin(),
				   [&](const auto& cell) -> uint32_t
				   { return cell._value; } );

    return ret;
}

uint32_t Outline::addFilledLine(std::vector<glm::vec2>& newPoints) const
{
	// Add odd
    if (newPoints.size() % 2 != 0)
    {
        newPoints.emplace_back(_bbox._min);
    }

	const float y = _bbox._max.y + 1000.0f;
    const auto i = static_cast<uint32_t>(newPoints.size());
	
	newPoints.emplace_back(glm::vec2(_bbox._min.x, y));
    newPoints.emplace_back(glm::vec2(_bbox._min.x + 10.0f, y));
    newPoints.emplace_back(glm::vec2(_bbox._min.x + 20.0f, y));

    return i;
}

}