#pragma once


#include <iostream>


namespace frm
{
    static constexpr float epsilon = 0.001f;
    static constexpr float one_minus_epsilon = 1.f - epsilon;
    static constexpr float pi = 3.14159265f;

    struct Point
    {
        float x;
        float y;
    };

    Point operator-(Point a, Point b) noexcept;

    std::ostream & operator<<(std::ostream & os, Point const & point) noexcept;
    std::istream & operator>>(std::istream & is, Point & point) noexcept;

    static constexpr float degree_to_radian(float angle) noexcept;

    static constexpr float radian_to_degree(float angle) noexcept;

    float angle_between_vectors(Point a, Point b) noexcept;

    // line: y = k * x + c
    // In returnable Point:
    // x - k
    // y - c
    static constexpr Point line_from_two_points(Point begin, Point end) noexcept(!IS_DEBUG);

    // line: y = k * x + c
    // Point line:
    // x - k
    // y - c
    static constexpr bool is_point_over_line(Point point, Point line) noexcept
    {
        return point.y > line.x * point.x + line.y;
    }

    // -1 - first > second
    //  0 - equal
    //  1 - first < second
    int compare_point_by_x(Point first, Point second) noexcept(!IS_DEBUG);
    // -1 - first > second
    //  0 - equal
    //  1 - first < second
    int compare_point_by_y(Point first, Point second) noexcept(!IS_DEBUG);

    template<typename T>
    static constexpr T lerp(T begin, T end, float alpha)
    {
        float const begin_float = static_cast<float>(begin);
        float const end_float = static_cast<float>(end);

        return static_cast<T>(begin_float * (1.f - alpha) + end_float * alpha);
    }

    template<>
    static constexpr float lerp<float>(float begin, float end, float alpha)
    {
        return begin * (1.f - alpha) + end * alpha;
    }

    template<>
    static constexpr Point lerp<Point>(Point begin, Point end, float alpha)
    {
        return
        {
            lerp(begin.x, end.x, alpha),
            lerp(begin.y, end.y, alpha)
        };
    }
}