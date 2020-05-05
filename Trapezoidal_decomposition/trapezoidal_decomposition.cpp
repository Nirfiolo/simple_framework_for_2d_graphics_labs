#include "trapezoidal_decomposition.h"

#include <set>
#include <cassert>


size_t get_outside_face(frm::dcel::DCEL const & dcel, size_t left_vertex_index) noexcept(!IS_DEBUG)
{
    auto const get_vector_from_edge = [&dcel](size_t edge_index) noexcept -> frm::Point
    {
        size_t const current_edge_index = edge_index;
        size_t const next_after_current_edge_index = dcel.edges[current_edge_index].next_edge;

        size_t const current_vertex_index = dcel.edges[current_edge_index].origin_vertex;
        size_t const next_after_current_vertex_index = dcel.edges[next_after_current_edge_index].origin_vertex;

        frm::Point const current = dcel.vertices[current_vertex_index].coordinate;
        frm::Point const next = dcel.vertices[next_after_current_vertex_index].coordinate;

        return { next.x - current.x, next.y - current.y };
    };

    size_t outside_face_index = std::numeric_limits<size_t>::max();

    size_t const current_vertex_index = left_vertex_index;

    std::vector<std::pair<size_t, size_t>> adjacents = frm::dcel::get_adjacent_vertices_and_edges(dcel, current_vertex_index);

    for (size_t i = 0; i < adjacents.size(); ++i)
    {
        size_t const current_edge_index = adjacents[i].second;
        frm::Point const current = get_vector_from_edge(current_edge_index);
        frm::Point const previuos = get_vector_from_edge(dcel.edges[current_edge_index].previous_edge);

        float const angle = frm::angle_between_vectors(current, previuos);

        if (angle > frm::epsilon)
        {
            outside_face_index = dcel.edges[current_edge_index].incident_face;
        }

        size_t f = 0;
    }

    assert(outside_face_index != std::numeric_limits<size_t>::max());

    return outside_face_index;
}


vertical_lines generate_vertical_lines(frm::dcel::DCEL const & dcel) noexcept(!IS_DEBUG)
{
    vertical_lines result{};

    std::vector<size_t> vertices(dcel.vertices.size());
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        vertices[i] = i;
    }

    std::sort(vertices.begin(), vertices.end(), [&dcel](size_t a, size_t b) noexcept -> bool
        {
            frm::Point point_a = dcel.vertices[a].coordinate;
            frm::Point point_b = dcel.vertices[b].coordinate;

            if (abs(point_a.x - point_b.x) < frm::epsilon)
            {
                return point_a.y < point_b.y;
            }

            return point_a.x < point_b.x;
        });

    struct StatusComponent
    {
        size_t begin_vertex_index;
        size_t end_vertex_index;
        size_t face_over_line;
        size_t face_under_line;
    };

    auto const status_component_comparator = [](StatusComponent const & a, StatusComponent const & b) noexcept -> bool
    {
        if (a.begin_vertex_index == b.begin_vertex_index)
        {
            return a.end_vertex_index < b.end_vertex_index;
        }

        return a.begin_vertex_index < b.begin_vertex_index;
    };

    std::set<StatusComponent, decltype(status_component_comparator)> status(status_component_comparator);

    size_t const outside_face_index = get_outside_face(dcel, vertices[0]);
    result.first = outside_face_index;

    float const offset_to_both_side = 100.f * frm::epsilon;
    float last_x = dcel.vertices[vertices[0]].coordinate.x - offset_to_both_side;

    for (size_t i = 0; i < vertices.size(); ++i)
    {
        size_t const current_vertex_index = vertices[i];
        float const current_x = dcel.vertices[current_vertex_index].coordinate.x;

        if (abs(current_x - last_x) > frm::epsilon)
        {
            std::vector<LineComponent> current_vertical{};

            for (StatusComponent const & current_element : status)
            {
                frm::Point begin_point = dcel.vertices[current_element.begin_vertex_index].coordinate;
                frm::Point end_point = dcel.vertices[current_element.end_vertex_index].coordinate;

                float const k = (end_point.y - begin_point.y) / (end_point.x - begin_point.x);
                float const c = end_point.y - k * end_point.x;

                current_vertical.push_back({ current_element.face_over_line, current_element.face_under_line, k, c });
            }

            result.second.emplace_back(current_x, std::move(current_vertical));
        }
        last_x = current_x;

        std::vector<std::pair<size_t, size_t>> adjacents = frm::dcel::get_adjacent_vertices_and_edges(dcel, current_vertex_index);

        for (std::pair<size_t, size_t> const & adjacent : adjacents)
        {
            StatusComponent already_in_status{};
            already_in_status.begin_vertex_index = adjacent.first;
            already_in_status.end_vertex_index = current_vertex_index;
            auto iterator_to_already_in_status = status.find(already_in_status);

            if (iterator_to_already_in_status != status.end())
            {
                iterator_to_already_in_status = status.erase(iterator_to_already_in_status);
            }
            else
            {
                StatusComponent new_status{};
                new_status.begin_vertex_index = current_vertex_index;
                new_status.end_vertex_index = adjacent.first;
                new_status.face_over_line = dcel.edges[adjacent.second].incident_face;
                new_status.face_under_line = dcel.edges[dcel.edges[adjacent.second].twin_edge].incident_face;

                bool const is_inserted = status.insert(new_status).second;
                assert(is_inserted);
            }
        }
    }

    // additional part on right side
    result.second.emplace_back(last_x + offset_to_both_side, std::vector<LineComponent>{});

    return result;
}

size_t get_face_index(vertical_lines const & lines, frm::Point point) noexcept
{
    size_t left = 0;
    size_t right = lines.second.size();

    while (left + 1 != right && left != right)
    {
        size_t const middle = (right + left) / 2;

        if (abs(lines.second[middle - 1].first - point.x) < frm::epsilon)
        {
            left = middle;
            right = middle + 1;
        }
        else if (lines.second[middle - 1].first < point.x)
        {
            left = middle;
        }
        else
        {
            right = middle;
        }
    }

    size_t const current_lines_list_index = right - 1;

    if (current_lines_list_index == 0 || current_lines_list_index == lines.second.size() - 1)
    {
        return lines.first;
    }

    left = 0;
    right = lines.second[current_lines_list_index].second.size();

    while (left + 1 != right && left != right)
    {
        size_t const middle = (right + left) / 2;

        LineComponent const & current_line = lines.second[current_lines_list_index].second[middle - 1];
        bool const is_point_over_line = frm::is_point_over_line(point, { current_line.k, current_line.c });
        
        if (is_point_over_line)
        {
            right = middle;
        }
        else
        {
            left = middle;
        }
    }

    size_t const current_line_index = right - 1;

    LineComponent const & current_line = lines.second[current_lines_list_index].second[current_line_index];
    bool const is_point_over_line = frm::is_point_over_line(point, { current_line.k, current_line.c });
    
    if (is_point_over_line)
    {
        return current_line.face_over_line;
    }
    else
    {
        return current_line.face_under_line;
    }
}
