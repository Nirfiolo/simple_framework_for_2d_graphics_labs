#include "common.h"

#include <string>
#include <cassert>


namespace frm
{
    std::ostream & operator<<(std::ostream & os, Point const & point) noexcept
    {
        os << "[ " << point.x << " , " << point.y << " ] ";

        return os;
    }

    std::istream & operator>>(std::istream & is, Point & point) noexcept
    {
        std::string additional_symbols;

        is >> additional_symbols >> point.x >> additional_symbols >> point.y >> additional_symbols;

        return is;
    }
    
    constexpr float degree_to_radian(float angle) noexcept
    {
        return angle / 180.f * pi;
    }
    
    constexpr float radian_to_degree(float angle) noexcept
    {
        return angle * 180.f / pi;
    }
    
    float angle_between_vectors(Point a, Point b) noexcept
    {
        return atan2(a.x * b.y - a.y * b.x, a.x * b.x + a.y * b.y);
    }
    
    constexpr Point line_from_two_points(Point begin, Point end) noexcept(!IS_DEBUG)
    {
        Point const  target = { end.x - begin.x, end.y - begin.y };

        assert(target.x > epsilon);

        float const m = target.y / target.x;

        return { m, begin.y - m * begin.x };
    }

    int compare_point_by_x(Point first, Point second) noexcept(!IS_DEBUG)
    {
        if (abs(first.x - second.x) < frm::epsilon)
        {
            if (abs(first.y - second.y) < frm::epsilon)
            {
                return 0;
            }
            if (first.y - second.y < frm::epsilon)
            {
                return 1;
            }
            if (first.y - second.y > frm::epsilon)
            {
                return -1;
            }
        }
        if (first.x - second.x < frm::epsilon)
        {
            return 1;
        }
        if (first.x - second.x > frm::epsilon)
        {
            return -1;
        }

        assert(false && "Compare error");
        return 0;
    }
    
    int compare_point_by_y(Point first, Point second) noexcept(!IS_DEBUG)
    {
        if (abs(first.y - second.y) < frm::epsilon)
        {
            if (abs(first.x - second.x) < frm::epsilon)
            {
                return 0;
            }
            if (first.x - second.x < frm::epsilon)
            {
                return 1;
            }
            if (first.x - second.x > frm::epsilon)
            {
                return -1;
            }
        }
        if (first.y - second.y < frm::epsilon)
        {
            return 1;
        }
        if (first.y - second.y > frm::epsilon)
        {
            return -1;
        }

        assert(false && "Compare error");
        return 0;
    }
}