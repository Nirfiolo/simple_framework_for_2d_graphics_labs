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
        std::vector<std::pair<size_t, size_t>> need_to_connects{};

        for (vvve::VVVE::edge_t edge : vvve.edges)
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

                    need_to_connects.push_back({ outside_adjacent_edge_to_begin, outside_adjacent_edge_to_end });
                }
            }
        }

        Point center{ 0, 0 };
        for (dcel::DCEL::Vertex vertex : dcel.vertices)
        {
            center += vertex.coordinate;
        }
        center = 1.f / static_cast<float>(dcel.vertices.size()) * center;

        

        std::sort(need_to_connects.begin(), need_to_connects.end(), [&dcel, center](std::pair<size_t, size_t> a, std::pair<size_t, size_t> b)
            {
                Point a_medium = 0.5f * (dcel.vertices[dcel.edges[a.first].origin_vertex].coordinate +
                    dcel.vertices[dcel.edges[a.second].origin_vertex].coordinate);
                
                Point b_medium = 0.5f * (dcel.vertices[dcel.edges[b.first].origin_vertex].coordinate +
                    dcel.vertices[dcel.edges[b.second].origin_vertex].coordinate);

                Point a_target = a_medium - center;
                Point b_target = b_medium - center;

                float a_angle_to_0_1 = angle_to_0_1_vector(a_target);
                float b_angle_to_0_1 = angle_to_0_1_vector(b_target);
                
                return b_angle_to_0_1 > a_angle_to_0_1;
            });

        auto handle_first_edges = [&need_to_connects, &dcel](size_t edge_to_handle) noexcept -> bool
        {
            if (edge_to_handle == need_to_connects.back().first ||
                need_to_connects.front().first == need_to_connects.back().second)
            {
                size_t new_edge = dcel::add_edge_between_two_edges(dcel, need_to_connects.front().first, need_to_connects.front().second).first;

                if (edge_to_handle == need_to_connects.back().first)
                {
                    need_to_connects.back().first = new_edge;
                }
                else
                {
                    need_to_connects.back().second = new_edge;
                }
                return true;
            }
            return false;
        };

        bool is_dirty = false;
        is_dirty |= handle_first_edges(need_to_connects.front().first);
        is_dirty |= handle_first_edges(need_to_connects.front().second);

        if (is_dirty)
        {
            need_to_connects.erase(need_to_connects.begin());
        }

        for (std::pair<size_t, size_t> const need_to_connect : need_to_connects)
        {
            dcel::add_edge_between_two_edges(dcel, need_to_connect.first, need_to_connect.second);
        }
    }
}
