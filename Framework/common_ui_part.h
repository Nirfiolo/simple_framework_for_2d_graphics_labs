#pragma once


#include "common.h"

#include "SFML\Graphics.hpp"


namespace frm
{
    void show_indexed_combo(size_t & current, size_t size, char const * lable) noexcept;

    sf::Color float4_to_uint8_t4(float circle_color[4]) noexcept;

    void draw_vertex_highlighted(Point point, float circle_color[4], float radius, sf::RenderWindow & window) noexcept;

    void draw_edge_highlighted(Point begin_point, Point end_point, float color[4], float width, sf::RenderWindow & window) noexcept;
}