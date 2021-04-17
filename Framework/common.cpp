#include "common.h"

#include <string>
#include <cassert>
#include <cmath>
#include <algorithm>


namespace frm
{
    Point operator+(Point a, Point b) noexcept
    {
        return { a.x + b.x, a.y + b.y };
    }

    Point operator-(Point a, Point b) noexcept
    {
        return { a.x - b.x, a.y - b.y };
    }

    Point operator*(float t, Point a) noexcept
    {
        return { t * a.x, t * a.y };
    }

    Point & operator+=(Point & a, Point b) noexcept
    {
        a = a + b;
        return a;
    }


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

    float angle_between_vectors(Point a, Point b) noexcept
    {
        return atan2(a.x * b.y - a.y * b.x, a.x * b.x + a.y * b.y);
    }

    float angle_to_0_1_vector(Point a) noexcept
    {
        if (is_approximately(a.x, 0.f))
        {
            if (a.y < -epsilon)
            {
                return pi;
            }
            return 0.f;   
        }

        if (a.x > epsilon)
        {
            return angle_between_vectors(a, Point{ 0.f, 1.f });
        }

        return pi + angle_between_vectors(a, Point{ 0.f, -1.f });
    }

    constexpr float dot(Point a, Point b) noexcept
    {
        return a.x * b.x + a.y * b.y;
    }
    
    Point line_from_two_points(Point begin, Point end) noexcept(!IS_DEBUG)
    {
        Point const  target = { end.x - begin.x, end.y - begin.y };

        assert(target.x > epsilon);

        float const m = target.y / target.x;

        return { m, begin.y - m * begin.x };
    }


    float distance_between_point_and_line_segment(Point line_begin, Point line_end, Point point) noexcept
    {
        float const sqr_distance = sqr_distance_between_points(line_begin, line_end);
        if (is_approximately(sqr_distance, 0.f))
        {
            return distance_between_points(line_begin, point);
        }

        float const t = std::max(0.f, std::min(1.f, dot(point - line_begin, line_end - line_begin) / sqr_distance));
        Point const projection = line_begin + t * (line_end - line_begin);

        return distance_between_points(point, projection);
    }

    float distance_between_points(Point a, Point b) noexcept
    {
        return std::sqrtf(sqr_distance_between_points(a, b));
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