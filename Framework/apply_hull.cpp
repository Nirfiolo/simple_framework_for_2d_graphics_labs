#include "apply_hull.h"

#include <cassert>


namespace frm
{
    std::vector<size_t> get_adjacent_faces(dcel::DCEL const & dcel, size_t vertex_index) noexcept
    {
        std::vector<size_t> faces{};

        std::vector<std::pair<size_t, size_t>> const adjacent_vertices_and_edges =
            dcel::get_adjacent_vertices_and_edges(dcel, vertex_index);


        for (std::pair<size_t, size_t> const & adjacent_vertex_and_edge : adjacent_vertices_and_edges)
        {
            size_t const edge_index = adjacent_vertex_and_edge.second;
            size_t const face_index = dcel.edges[edge_index].incident_face;

            if (std::find(faces.begin(), faces.end(), face_index) == std::end(faces))
            {
                faces.push_back(face_index);
            }
        }

        return faces;
    }

    bool is_same_polygon(dcel::DCEL const & dcel, size_t begin_vertex, size_t end_vertex) noexcept
    {
        std::vector<size_t> adjacent_faces_to_begin = get_adjacent_faces(dcel, begin_vertex);
        std::vector<size_t> adjacent_faces_to_end = get_adjacent_faces(dcel, end_vertex);

        size_t amount_of_common = 0;

        for (size_t adjacent_face_to_begin : adjacent_faces_to_begin)
        {
            if (std::find(adjacent_faces_to_end.begin(), adjacent_faces_to_end.end(), adjacent_face_to_begin)
                != std::end(adjacent_faces_to_end))
            {
                ++amount_of_common;
            }
        }

        // 2 because 1 common outside face and 1 common inside face 
        return amount_of_common >= 2;
    }


    size_t get_outside_adjacent_edge(dcel::DCEL const & dcel, size_t vertex_index) noexcept
    {
        size_t const outside_face_index = dcel::get_outside_face_index(dcel);

        std::vector<std::pair<size_t, size_t>> const adjacent_vertices_and_edges =
            dcel::get_adjacent_vertices_and_edges(dcel, vertex_index);

        for (std::pair<size_t, size_t> const & adjacent_vertex_and_edge : adjacent_vertices_and_edges)
        {
            size_t const edge_index = adjacent_vertex_and_edge.second;

            if (dcel.edges[edge_index].incident_face == outside_face_index)
            {
                return edge_index;
            }
        }

        assert(false && "Outside adjacent edge does not exist");

        return adjacent_vertices_and_edges[0].second;
    }

    void apply_hull(dcel::DCEL & dcel, vvve::VVVE const & vvve) noexcept
    {
        for (vvve::VVVE::edge_t const edge : vvve.edges)
        {
            size_t const begin_vertex = edge.first;
            size_t const end_vertex = edge.second;

            if (!dcel::is_points_connected(dcel, begin_vertex, end_vertex))
            {
                if (!is_same_polygon(dcel, begin_vertex, end_vertex))
                {
                    size_t outside_adjacent_edge_to_begin =
                        get_outside_adjacent_edge(dcel, begin_vertex);

                    size_t outside_adjacent_edge_to_end =
                        get_outside_adjacent_edge(dcel, end_vertex);

                    dcel::add_edge_between_two_edges(dcel, outside_adjacent_edge_to_begin, outside_adjacent_edge_to_end);
                }
            }
        }
    }
}
