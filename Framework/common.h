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
}