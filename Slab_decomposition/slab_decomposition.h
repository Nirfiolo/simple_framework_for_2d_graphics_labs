#pragma once


#include "dcel.h"


struct LineComponent
{
    size_t face_over_line;
    size_t face_under_line;
    // y = k * x + c
    float k;
    float c;
};

// first parameter is outside face
using vertical_lines = std::pair<size_t, std::vector<std::pair<float, std::vector<LineComponent>>>>;

// O(n^2)
vertical_lines generate_vertical_lines(frm::dcel::DCEL const & dcel) noexcept(!IS_DEBUG);

// O(log(n))
size_t get_face_index(vertical_lines const & lines, frm::Point point) noexcept;