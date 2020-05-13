#include "triangulation.h"

#include "imgui/imgui.h"


namespace frm
{
    size_t get_edges_count_in_face(dcel::DCEL & dcel, size_t face_index) noexcept
    {
        size_t const begin = dcel.faces[face_index].edge;
        size_t current_index = begin;
        size_t edges_count = 0;

        do
        {
            current_index = dcel.edges[current_index].next_edge;
            ++edges_count;
        } while (current_index != begin);

        return edges_count;
    }

    size_t get_top_bottom_edge_index_from_face(dcel::DCEL & dcel, size_t face_index, bool is_top = true) noexcept
    {
        size_t const begin = dcel.faces[face_index].edge;
        size_t current_index = begin;

        size_t finding_vertex_index = current_index;
        frm::Point finding_vertex_point = dcel.vertices[dcel.edges[current_index].origin_vertex].coordinate;

        do
        {
            frm::Point current_point = dcel.vertices[dcel.edges[current_index].origin_vertex].coordinate;

            if (is_top && compare_point_by_y(current_point, finding_vertex_point) > 0)
            {
                finding_vertex_point = current_point;
                finding_vertex_index = current_index;
            }
            if (!is_top && compare_point_by_y(current_point, finding_vertex_point) < 0)
            {
                finding_vertex_point = current_point;
                finding_vertex_index = current_index;
            }

            current_index = dcel.edges[current_index].next_edge;
        } while (current_index != begin);

        return finding_vertex_index;
    }

    bool is_diagonal(dcel::DCEL const & dcel, size_t from_edge_index, size_t to_edge_index, size_t wall_edge_index) noexcept
    {
        size_t const from_vertex_index = dcel.edges[from_edge_index].origin_vertex;
        size_t const to_vertex_index = dcel.edges[to_edge_index].origin_vertex;
        size_t const wall_vertex_index = dcel.edges[wall_edge_index].origin_vertex;

        Point const from_point = dcel.vertices[from_vertex_index].coordinate;
        Point const to_point = dcel.vertices[to_vertex_index].coordinate;
        Point const wall_point = dcel.vertices[wall_vertex_index].coordinate;

        return !std::signbit((to_point.x - from_point.x) * (wall_point.y - from_point.y) - (to_point.y - from_point.y) * (wall_point.x - from_point.x));
    }

    void triangulation_y_monotone(dcel::DCEL & dcel, size_t face_index) noexcept
    {
        size_t const edges_count = get_edges_count_in_face(dcel, face_index);

        if (edges_count == 3)
        {
            return;
        }

        // first - edge index
        // second - is left
        std::vector<std::pair<size_t, bool>> edges{};
        edges.reserve(edges_count);

        size_t const top_edge_index = get_top_bottom_edge_index_from_face(dcel, face_index, true);
        size_t const bottom_edge_index = get_top_bottom_edge_index_from_face(dcel, face_index, false);

        size_t left_edge_index = dcel.edges[top_edge_index].previous_edge;
        size_t right_edge_index = dcel.edges[top_edge_index].next_edge;

        edges.emplace_back(std::make_pair(top_edge_index, true));

        while (left_edge_index != bottom_edge_index || right_edge_index != bottom_edge_index)
        {
            Point const left_point = dcel.vertices[dcel.edges[left_edge_index].origin_vertex].coordinate;
            Point const right_point = dcel.vertices[dcel.edges[right_edge_index].origin_vertex].coordinate;

            int const compared_point = compare_point_by_y(left_point, right_point);
            if (compared_point >= 0)
            {
                edges.emplace_back(std::make_pair(left_edge_index, true));
                left_edge_index = dcel.edges[left_edge_index].previous_edge;
            }
            if (compared_point < 0)
            {
                edges.emplace_back(std::make_pair(right_edge_index, false));
                right_edge_index = dcel.edges[right_edge_index].next_edge;
            }
        }

        edges.emplace_back(std::make_pair(bottom_edge_index, true));

        std::vector<size_t> stack{ 0, 1 };

        for (size_t i = 2; i < edges.size() - 1; ++i)
        {
            bool const last_edge_is_left = edges[stack.back()].second;

            if (last_edge_is_left != edges[i].second)
            {
                size_t last_left_edge = std::numeric_limits<size_t>::max();
                size_t current_edge_index = std::numeric_limits<size_t>::max();

                while (stack.size() > 1)
                {
                    current_edge_index = stack.back();
                    stack.pop_back();
                    std::pair<size_t, size_t> new_edges_index = dcel::add_edge_between_two_edges(dcel, edges[i].first, edges[current_edge_index].first);
                    if (edges[i].second)
                    {
                        if (dcel.edges[new_edges_index.first].origin_vertex == dcel.edges[edges[i].first].origin_vertex)
                        {
                            last_left_edge = new_edges_index.first;
                        }
                        else if (dcel.edges[new_edges_index.second].origin_vertex == dcel.edges[edges[i].first].origin_vertex)
                        {
                            last_left_edge = new_edges_index.second;
                        }
                        else
                        {
                            assert(false && "Error edge adding");
                        }
                    }
                    else
                    {
                        if (dcel.edges[new_edges_index.first].origin_vertex == dcel.edges[edges[current_edge_index].first].origin_vertex)
                        {
                            last_left_edge = new_edges_index.first;
                        }
                        else if (dcel.edges[new_edges_index.second].origin_vertex == dcel.edges[edges[current_edge_index].first].origin_vertex)
                        {
                            last_left_edge = new_edges_index.second;
                        }
                        else
                        {
                            assert(false && "Error edge adding");
                        }
                    }
                }
                stack.pop_back();

                stack.push_back(i - 1);
                stack.push_back(i);

                if (current_edge_index != std::numeric_limits<size_t>::max())
                {
                    if (edges[i].second)
                    {
                        edges[i].first = last_left_edge;
                    }
                    else
                    {
                        edges[current_edge_index].first = last_left_edge;
                    }
                }
            }
            else
            {
                size_t current_edge_index = stack.back();
                stack.pop_back();

                while (!stack.empty() && is_diagonal(dcel, edges[i].first, edges[stack.back()].first, edges[current_edge_index].first))
                {
                    std::pair<size_t, size_t> new_edges_index = dcel::add_edge_between_two_edges(dcel, edges[i].first, edges[stack.back()].first);
                    if (edges[i].second)
                    {
                        if (dcel.edges[new_edges_index.first].origin_vertex == dcel.edges[edges[i].first].origin_vertex)
                        {
                            edges[i].first = new_edges_index.first;
                        }
                        else if (dcel.edges[new_edges_index.second].origin_vertex == dcel.edges[edges[i].first].origin_vertex)
                        {
                            edges[i].first = new_edges_index.second;
                        }
                        else
                        {
                            assert(false && "Error edge adding");
                        }
                    }
                    else
                    {
                        if (dcel.edges[new_edges_index.first].origin_vertex == dcel.edges[edges[stack.back()].first].origin_vertex)
                        {
                            edges[stack.back()].first = new_edges_index.first;
                        }
                        else if (dcel.edges[new_edges_index.second].origin_vertex == dcel.edges[edges[stack.back()].first].origin_vertex)
                        {
                            edges[stack.back()].first = new_edges_index.second;
                        }
                        else
                        {
                            assert(false && "Error edge adding");
                        }
                    }

                    current_edge_index = stack.back();
                    stack.pop_back();
                }

                stack.push_back(current_edge_index);
                stack.push_back(i);

                size_t end_line = 0;
            }
        }

        while (stack.size() > 2)
        {
            size_t const current_edge_index = stack.back();
            stack.pop_back();
            dcel::add_edge_between_two_edges(dcel, edges.back().first, edges[current_edge_index].first);
        }

        size_t end_line = 0;
    }


    void triangulation(dcel::DCEL & dcel) noexcept
    {
        size_t const outside_face_index = get_outside_face_index(dcel);


        triangulation_y_monotone(dcel, (outside_face_index + 1) % 2);
    }

    bool spawn_triangulation_button(dcel::DCEL & dcel) noexcept
    {
        bool is_dirty = false;

        static bool is_active = false;
        if (ImGui::Begin("Triangulation", &is_active))
        {
            if (ImGui::Button("Triangulate"))
            {
                triangulation(dcel);
                is_dirty = true;
            }
        }
        ImGui::End();

        return is_dirty;
    }
}