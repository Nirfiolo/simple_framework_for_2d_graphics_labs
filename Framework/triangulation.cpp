#include "triangulation.h"

#include "imgui/imgui.h"

#include <set>
#include <optional>


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

    bool is_diagonal(dcel::DCEL const & dcel, size_t from_edge_index, size_t to_edge_index, size_t wall_edge_index, bool is_left_side) noexcept
    {
        size_t const from_vertex_index = dcel.edges[from_edge_index].origin_vertex;
        size_t const to_vertex_index = dcel.edges[to_edge_index].origin_vertex;
        size_t const wall_vertex_index = dcel.edges[wall_edge_index].origin_vertex;

        Point const from_point = dcel.vertices[from_vertex_index].coordinate;
        Point const to_point = dcel.vertices[to_vertex_index].coordinate;
        Point const wall_point = dcel.vertices[wall_vertex_index].coordinate;

        return (is_left_side == std::signbit((to_point.x - from_point.x) * (wall_point.y - from_point.y) - (to_point.y - from_point.y) * (wall_point.x - from_point.x)));
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
                size_t last_edge_index = std::numeric_limits<size_t>::max();

                size_t current_right_edge = edges[i].first;

                while (stack.size() > 1)
                {
                    size_t const current_edge_index = stack.back();
                    stack.pop_back();
                    std::pair<size_t, size_t> new_edges_index = dcel::add_edge_between_two_edges(dcel, current_right_edge, edges[current_edge_index].first);
                    if (last_left_edge == std::numeric_limits<size_t>::max())
                    {
                        last_edge_index = current_edge_index;

                        if (edges[i].second)
                        {
                            if (dcel.edges[new_edges_index.first].origin_vertex == dcel.edges[current_right_edge].origin_vertex)
                            {
                                last_left_edge = new_edges_index.first;
                            }
                            else if (dcel.edges[new_edges_index.second].origin_vertex == dcel.edges[current_right_edge].origin_vertex)
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
                    if (!edges[i].second)
                    {
                        if (dcel.edges[new_edges_index.first].origin_vertex == dcel.edges[edges[current_edge_index].first].origin_vertex)
                        {
                            current_right_edge = new_edges_index.second;
                        }
                        else if (dcel.edges[new_edges_index.second].origin_vertex == dcel.edges[edges[current_edge_index].first].origin_vertex)
                        {
                            current_right_edge = new_edges_index.first;
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

                if (last_left_edge != std::numeric_limits<size_t>::max())
                {
                    if (edges[i].second)
                    {
                        edges[i].first = last_left_edge;
                    }
                    else
                    {
                        edges[last_edge_index].first = last_left_edge;
                    }
                }
            }
            else
            {
                size_t current_edge_index = stack.back();
                stack.pop_back();

                while (!stack.empty() && is_diagonal(dcel, edges[i].first, edges[stack.back()].first, edges[current_edge_index].first, edges[i].second))
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
            }
        }

        while (stack.size() > 2)
        {
            size_t const current_edge_index = stack.back();
            stack.pop_back();
            dcel::add_edge_between_two_edges(dcel, edges.back().first, edges[stack.back()].first);
        }
    }


    std::set<size_t> get_outside_faces(dcel::DCEL const & dcel) noexcept
    {
        size_t const outside_face_index = get_outside_face_index(dcel);

        size_t const main_face_index = get_possibly_main_face_index(dcel);

        std::set<size_t> outside_faces{};
        for (size_t i = 0; i < dcel.faces.size(); ++i)
        {
            if (i != main_face_index)
            {
                outside_faces.insert(i);
            }
        }

        return outside_faces;
    }

    bool is_outside_face(dcel::DCEL const & dcel, std::set<size_t> const & outside_faces, size_t face_index) noexcept
    {
        if (outside_faces.find(face_index) != outside_faces.end())
        {
            return true;
        }
        return false;
    }

    // pair.first vertex index
    // pair.second edge index
    std::vector<std::pair<size_t, size_t>> get_adjacent_vertices_and_edges_without_outside_faces(
        dcel::DCEL const & dcel,
        size_t vertex_index,
        std::set<size_t> const & outside_faces) noexcept
    {
        std::vector<std::pair<size_t, size_t>> adjacents =
            frm::dcel::get_adjacent_vertices_and_edges(dcel, vertex_index);

        std::vector<std::pair<size_t, size_t>> new_adjacents{};

        for (std::pair<size_t, size_t> adjacent : adjacents)
        {
            if (!is_outside_face(dcel, outside_faces, dcel.edges[adjacent.second].incident_face))
            {
                new_adjacents.push_back(adjacent);
            }
            else
            {
                size_t edge_twin = dcel.edges[adjacent.second].twin_edge;
                if (!is_outside_face(dcel, outside_faces, dcel.edges[edge_twin].incident_face))
                {
                    new_adjacents.push_back(adjacent);
                }
            }
        }

        return new_adjacents;
    }

    // first - current
    // second.first - previous
    // second.second - next
    std::optional<std::pair<size_t, std::pair<size_t, size_t>>> get_current_and_previous_and_next_edges_to_current_vertex(
        dcel::DCEL const & dcel,
        size_t current_vertex_index,
        std::set<size_t> const & outside_faces
    ) noexcept
    {
        std::vector<std::pair<size_t, size_t>> adjacents =
            get_adjacent_vertices_and_edges_without_outside_faces(dcel, current_vertex_index, outside_faces);

        if (adjacents.size() == 2)
        {
            size_t const adjacent_index = is_outside_face(dcel, outside_faces, dcel.edges[adjacents[0].second].incident_face) ? 1 : 0;

            size_t const current_edge_index = adjacents[adjacent_index].second;
            size_t const previous_edge_to_current_index = dcel.edges[current_edge_index].previous_edge;
            size_t const next_edge_after_current_index = dcel.edges[current_edge_index].next_edge;

            return { { current_edge_index, { previous_edge_to_current_index, next_edge_after_current_index } } };
        }
        if (adjacents.empty())
        {
            return {};
        }

        assert(false && "Incorrect polygon");

        return {};
    }

    std::vector<std::pair<size_t, std::pair<size_t, size_t>>> get_current_and_previous_and_next_edges_groups_to_current_vertex(
        dcel::DCEL const & dcel,
        size_t current_vertex_index,
        std::set<size_t> const & outside_faces
    ) noexcept
    {
        std::vector<std::pair<size_t, size_t>> adjacents =
            get_adjacent_vertices_and_edges_without_outside_faces(dcel, current_vertex_index, outside_faces);

        auto get_current_and_previous_and_next_edges = [&dcel, &outside_faces](std::pair<size_t, size_t> edges) noexcept -> std::pair<size_t, std::pair<size_t, size_t>>
        {
            size_t const current_edge_index = is_outside_face(dcel, outside_faces, dcel.edges[edges.first].incident_face) ? edges.second : edges.first;
            size_t const previous_edge_to_current_index = dcel.edges[current_edge_index].previous_edge;
            size_t const next_edge_after_current_index = dcel.edges[current_edge_index].next_edge;

            return { current_edge_index, { previous_edge_to_current_index, next_edge_after_current_index } };
        };

        if (adjacents.empty())
        {
            return {};
        }
        if (adjacents.size() == 2)
        {
            return { get_current_and_previous_and_next_edges({adjacents[0].second, adjacents[1].second}) };
        }
        if (adjacents.size() == 4)
        {
            std::vector<std::pair<size_t, std::pair<size_t, size_t>>> edges_groups{};
            
            for (std::pair<size_t, size_t> const adjacent : adjacents)
            {
                if (!is_outside_face(dcel, outside_faces, dcel.edges[adjacent.second].incident_face))
                {
                    edges_groups.push_back(get_current_and_previous_and_next_edges({adjacent.second, dcel.edges[adjacent.second].previous_edge}));
                }
            }

            return edges_groups;
        }

        assert(false && "Incorrect polygon");

        return {};
    }

    // first - current
    // second.first - previous
    // second.second - next
    std::pair<size_t, std::pair<size_t, size_t>> get_current_and_previous_and_next_edges_to_current_vertex_by_neighbours(
        dcel::DCEL const & dcel,
        size_t current_vertex_index,
        std::set<size_t> const & outside_faces,
        size_t previous_neighbour,
        size_t next_neighbour
    ) noexcept
    {
        std::vector<std::pair<size_t, size_t>> adjacents =
            get_adjacent_vertices_and_edges_without_outside_faces(dcel, current_vertex_index, outside_faces);

        if (adjacents.size() == 2)
        {
            return *get_current_and_previous_and_next_edges_to_current_vertex(dcel, current_vertex_index, outside_faces);
        }

        size_t first_neighbour_index_in_adjacents = std::numeric_limits<size_t>::max();
        for (size_t i = 0; i < adjacents.size(); ++i)
        {
            if (adjacents[i].first == previous_neighbour)
            {
                first_neighbour_index_in_adjacents = i;
                break;
            }
        }

        assert(first_neighbour_index_in_adjacents != std::numeric_limits<size_t>::max() && "first neighbour not found");

        size_t current_edge_index;

        if (dcel.edges[dcel.edges[adjacents[first_neighbour_index_in_adjacents].second].previous_edge].origin_vertex == next_neighbour)
        {
            current_edge_index = adjacents[first_neighbour_index_in_adjacents].second;
        }
        else
        {
            current_edge_index = dcel.edges[dcel.edges[adjacents[first_neighbour_index_in_adjacents].second].twin_edge].next_edge;
        }

        size_t const previous_edge_to_current_index = dcel.edges[current_edge_index].previous_edge;
        size_t const next_edge_after_current_index = dcel.edges[current_edge_index].next_edge;

        return { current_edge_index, { previous_edge_to_current_index, next_edge_after_current_index } };
    }

    float get_intersection_line_with_horizontal_line(Point begin, Point end, float y) noexcept
    {
        return begin.x + (y - begin.y) * (begin.x - end.x) / (begin.y - end.y);
    }

    enum VertexType
    {
        Undefined       = 0b0000'0000,
        End             = 0b0000'0001,
        Start           = 0b0000'0010,
        Split           = 0b0000'0100,
        Merge           = 0b0000'1000,
        RegularLeft     = 0b0001'0000,
        RegularRight    = 0b0010'0000
    };


    VertexType operator&(VertexType a, VertexType b)
    {
        return static_cast<VertexType>(static_cast<int>(a) & static_cast<int>(b));
    }

    VertexType & operator&=(VertexType & a, VertexType b)
    {
        a = a & b;
        return a;
    }

    VertexType operator~(VertexType a)
    {
        return static_cast<VertexType>(~static_cast<int>(a));
    }


    struct StatusComponent
    {
        size_t begin_vertex_index;
        size_t end_vertex_index;
        size_t helper;
        size_t previous_neighbour_to_helper;
        size_t next_neighbour_after_helper;
    };

    auto const status_component_comparator = [](StatusComponent const & a, StatusComponent const & b) noexcept -> bool
    {
        if (a.begin_vertex_index == b.begin_vertex_index)
        {
            return a.end_vertex_index < b.end_vertex_index;
        }

        return a.begin_vertex_index < b.begin_vertex_index;
    };

    std::set<StatusComponent, decltype(status_component_comparator)>::iterator get_nearest_left_component(
        dcel::DCEL const & dcel,
        std::set<StatusComponent, decltype(status_component_comparator)> & status,
        size_t current_vertex_index
    ) noexcept
    {
        auto nearest_left_component = status.begin();

        Point const current_point = dcel.vertices[current_vertex_index].coordinate;
        float nearest_x = -1000000000.f;

        auto current_iterator = status.begin();
        for (; current_iterator != status.end(); ++current_iterator)
        {
            StatusComponent const & status_component = *current_iterator;
            Point const begin = dcel.vertices[status_component.begin_vertex_index].coordinate;
            Point const end = dcel.vertices[status_component.end_vertex_index].coordinate;

            if (abs(begin.y - end.y) < epsilon)
            {
                if (begin.x < current_point.x && begin.x > nearest_x)
                {
                    nearest_x = begin.x;
                    nearest_left_component = current_iterator;
                }
                if (end.x < current_point.x && end.x > nearest_x)
                {
                    nearest_x = end.x;
                    nearest_left_component = current_iterator;
                }
            }
            else
            {
                float const x_position = get_intersection_line_with_horizontal_line(begin, end, current_point.y);

                if (x_position < current_point.x && x_position > nearest_x)
                {
                    nearest_x = x_position;
                    nearest_left_component = current_iterator;
                }
            }
        }
        return nearest_left_component;
    }

    void handle_start(
        dcel::DCEL & dcel,
        std::set<StatusComponent, decltype(status_component_comparator)> & status,
        std::set<size_t> const & outside_faces,
        std::vector<std::pair<size_t, size_t>> const & vertex_neighbours,
        size_t vertex_index
    ) noexcept
    {
        std::pair<size_t, size_t> const previous_and_next_edges_to_current =
            get_current_and_previous_and_next_edges_to_current_vertex_by_neighbours(
                dcel,
                vertex_index,
                outside_faces,
                vertex_neighbours[vertex_index].first,
                vertex_neighbours[vertex_index].second).second;
        size_t const previous_edge_to_current_index = previous_and_next_edges_to_current.first;
        size_t const next_edge_after_current_index = previous_and_next_edges_to_current.second;

        StatusComponent status_component{};
        status_component.begin_vertex_index = vertex_index;
        status_component.end_vertex_index = dcel.edges[previous_edge_to_current_index].origin_vertex;
        status_component.helper = vertex_index;
        status_component.previous_neighbour_to_helper = vertex_neighbours[vertex_index].first;
        status_component.next_neighbour_after_helper = vertex_neighbours[vertex_index].second;

        status.insert(status_component);
    }

    void handle_end(
        dcel::DCEL & dcel,
        std::set<StatusComponent, decltype(status_component_comparator)> & status,
        std::vector<VertexType> const & vertex_types,
        std::vector<std::pair<size_t, size_t>> const & vertex_neighbours,
        std::set<size_t> const & outside_faces,
        size_t vertex_index
    ) noexcept
    {
        std::pair<size_t, std::pair<size_t, size_t>> const current_and_previous_and_next_edges_to_current =
            get_current_and_previous_and_next_edges_to_current_vertex_by_neighbours(
                dcel,
                vertex_index,
                outside_faces,
                vertex_neighbours[vertex_index].first,
                vertex_neighbours[vertex_index].second);
        size_t const current_edge_index = current_and_previous_and_next_edges_to_current.first;
        size_t const previous_edge_to_current_index = current_and_previous_and_next_edges_to_current.second.first;
        size_t const next_edge_after_current_index = current_and_previous_and_next_edges_to_current.second.second;

        StatusComponent status_component{};
        status_component.begin_vertex_index = dcel.edges[next_edge_after_current_index].origin_vertex;
        status_component.end_vertex_index = vertex_index;

        auto iterator_to_status_component = status.find(status_component);

        if (iterator_to_status_component == status.end())
        {
            assert(false && "Status error: existing component not found");
        }
        else
        {
            size_t const helper = (*iterator_to_status_component).helper;
            size_t const previous_neighbour_to_helper = (*iterator_to_status_component).previous_neighbour_to_helper;
            size_t const next_neighbour_after_helper = (*iterator_to_status_component).next_neighbour_after_helper;

            if (vertex_types[helper] == VertexType::Merge)
            {
                size_t const helper_edge_index =
                    get_current_and_previous_and_next_edges_to_current_vertex_by_neighbours(
                        dcel,
                        helper,
                        outside_faces,
                        previous_neighbour_to_helper,
                        next_neighbour_after_helper
                    ).first;

                dcel::add_edge_between_two_edges(dcel, current_edge_index, helper_edge_index);
            }
        }

        status.erase(iterator_to_status_component);
    }

    void handle_split(
        dcel::DCEL & dcel,
        std::set<StatusComponent, decltype(status_component_comparator)> & status,
        std::vector<VertexType> const & vertex_types,
        std::vector<std::pair<size_t, size_t>> const & vertex_neighbours,
        std::set<size_t> const & outside_faces,
        size_t vertex_index
    ) noexcept
    {
        auto nearest_left_component = get_nearest_left_component(dcel, status, vertex_index);

        StatusComponent nearest_left = *nearest_left_component;

        std::pair<size_t, std::pair<size_t, size_t>> const current_and_previous_and_next_edges_to_current =
            get_current_and_previous_and_next_edges_to_current_vertex_by_neighbours(
                dcel,
                vertex_index,
                outside_faces,
                vertex_neighbours[vertex_index].first,
                vertex_neighbours[vertex_index].second);
        size_t const current_edge_index = current_and_previous_and_next_edges_to_current.first;
        size_t const previous_edge_to_current_index = current_and_previous_and_next_edges_to_current.second.first;
        size_t const next_edge_after_current_index = current_and_previous_and_next_edges_to_current.second.second;

        size_t const helper = nearest_left.helper;
        size_t const previous_neighbour_to_helper = nearest_left.previous_neighbour_to_helper;
        size_t const next_neighbour_after_helper = nearest_left.next_neighbour_after_helper;
        size_t const helper_edge_index =
            get_current_and_previous_and_next_edges_to_current_vertex_by_neighbours(
                dcel,
                helper,
                outside_faces,
                previous_neighbour_to_helper,
                next_neighbour_after_helper
            ).first;

        dcel::add_edge_between_two_edges(dcel, current_edge_index, helper_edge_index);

        status.erase(nearest_left_component);
        nearest_left.helper = vertex_index;
        nearest_left.previous_neighbour_to_helper = helper;
        nearest_left.next_neighbour_after_helper = dcel.edges[next_edge_after_current_index].origin_vertex;

        status.insert(nearest_left);

        StatusComponent status_component{};
        status_component.begin_vertex_index = vertex_index;
        status_component.end_vertex_index = dcel.edges[previous_edge_to_current_index].origin_vertex;
        status_component.helper = vertex_index;
        status_component.previous_neighbour_to_helper = dcel.edges[previous_edge_to_current_index].origin_vertex;
        status_component.next_neighbour_after_helper = helper;

        status.insert(status_component);
    }

    void handle_merge(
        dcel::DCEL & dcel,
        std::set<StatusComponent, decltype(status_component_comparator)> & status,
        std::vector<VertexType> const & vertex_types,
        std::vector<std::pair<size_t, size_t>> const & vertex_neighbours,
        std::set<size_t> const & outside_faces,
        size_t vertex_index
    ) noexcept
    {
        std::pair<size_t, std::pair<size_t, size_t>> const current_and_previous_and_next_edges_to_current =
            get_current_and_previous_and_next_edges_to_current_vertex_by_neighbours(
                dcel,
                vertex_index,
                outside_faces,
                vertex_neighbours[vertex_index].first,
                vertex_neighbours[vertex_index].second);
        size_t current_edge_index = current_and_previous_and_next_edges_to_current.first;
        size_t const previous_edge_to_current_index = current_and_previous_and_next_edges_to_current.second.first;
        size_t const next_edge_after_current_index = current_and_previous_and_next_edges_to_current.second.second;

        StatusComponent status_component{};
        status_component.begin_vertex_index = dcel.edges[next_edge_after_current_index].origin_vertex;
        status_component.end_vertex_index = vertex_index;

        auto iterator_to_status_component = status.find(status_component);

        size_t future_previous_neighbour = vertex_neighbours[vertex_index].first;
        size_t future_next_neighbour = vertex_neighbours[vertex_index].second;

        if (iterator_to_status_component == status.end())
        {
            assert(false && "Status error: existing component not found");
        }
        else
        {
            size_t const helper = (*iterator_to_status_component).helper;
            size_t const previous_neighbour_to_helper = (*iterator_to_status_component).previous_neighbour_to_helper;
            size_t const next_neighbour_after_helper = (*iterator_to_status_component).next_neighbour_after_helper;

            if (vertex_types[helper] == VertexType::Merge)
            {
                size_t const helper_edge_index =
                    get_current_and_previous_and_next_edges_to_current_vertex_by_neighbours(
                        dcel,
                        helper,
                        outside_faces,
                        previous_neighbour_to_helper,
                        next_neighbour_after_helper
                    ).first;

                dcel::add_edge_between_two_edges(dcel, current_edge_index, helper_edge_index);

                future_next_neighbour = helper;
            }
        }

        status.erase(iterator_to_status_component);

        auto nearest_left_component = get_nearest_left_component(dcel, status, vertex_index);

        StatusComponent nearest_left = *nearest_left_component;

        if (vertex_types[nearest_left.helper] == VertexType::Merge)
        {
            current_edge_index = get_current_and_previous_and_next_edges_to_current_vertex_by_neighbours(
                dcel,
                vertex_index,
                outside_faces,
                future_previous_neighbour,
                future_next_neighbour
            ).first;

            size_t const helper = nearest_left.helper;
            size_t const previous_neighbour_to_helper = nearest_left.previous_neighbour_to_helper;
            size_t const next_neighbour_after_helper = nearest_left.next_neighbour_after_helper;

            size_t const helper_edge_index =
                get_current_and_previous_and_next_edges_to_current_vertex_by_neighbours(
                    dcel,
                    helper,
                    outside_faces,
                    previous_neighbour_to_helper,
                    next_neighbour_after_helper
                ).first;

            dcel::add_edge_between_two_edges(dcel, current_edge_index, helper_edge_index);

            future_previous_neighbour = helper;
        }

        status.erase(nearest_left_component);
        nearest_left.helper = vertex_index;
        nearest_left.previous_neighbour_to_helper = future_previous_neighbour;
        nearest_left.next_neighbour_after_helper = future_next_neighbour;

        status.insert(nearest_left);
    }

    void handle_regular_left(
        dcel::DCEL & dcel,
        std::set<StatusComponent, decltype(status_component_comparator)> & status,
        std::vector<VertexType> const & vertex_types,
        std::vector<std::pair<size_t, size_t>> const & vertex_neighbours,
        std::set<size_t> const & outside_faces,
        size_t vertex_index
    ) noexcept
    {
        std::pair<size_t, std::pair<size_t, size_t>> const current_and_previous_and_next_edges_to_current =
            get_current_and_previous_and_next_edges_to_current_vertex_by_neighbours(
                dcel,
                vertex_index,
                outside_faces,
                vertex_neighbours[vertex_index].first,
                vertex_neighbours[vertex_index].second);
        size_t const current_edge_index = current_and_previous_and_next_edges_to_current.first;
        size_t const previous_edge_to_current_index = current_and_previous_and_next_edges_to_current.second.first;
        size_t const next_edge_after_current_index = current_and_previous_and_next_edges_to_current.second.second;

        StatusComponent status_component{};
        status_component.begin_vertex_index = dcel.edges[next_edge_after_current_index].origin_vertex;
        status_component.end_vertex_index = vertex_index;

        auto iterator_to_status_component = status.find(status_component);

        if (iterator_to_status_component == status.end())
        {
            assert(false && "Status error: existing component not found");
        }
        else
        {
            size_t const helper = (*iterator_to_status_component).helper;
            size_t const previous_neighbour_to_helper = (*iterator_to_status_component).previous_neighbour_to_helper;
            size_t const next_neighbour_after_helper = (*iterator_to_status_component).next_neighbour_after_helper;

            if (vertex_types[helper] == VertexType::Merge)
            {
                size_t const helper_edge_index =
                    get_current_and_previous_and_next_edges_to_current_vertex_by_neighbours(
                        dcel,
                        helper,
                        outside_faces,
                        previous_neighbour_to_helper,
                        next_neighbour_after_helper
                    ).first;

                dcel::add_edge_between_two_edges(dcel, current_edge_index, helper_edge_index);
            }
        }

        status.erase(iterator_to_status_component);

        StatusComponent new_status_component{};
        new_status_component.begin_vertex_index = vertex_index;
        new_status_component.end_vertex_index = dcel.edges[previous_edge_to_current_index].origin_vertex;
        new_status_component.helper = vertex_index;
        new_status_component.previous_neighbour_to_helper = vertex_neighbours[vertex_index].first;
        new_status_component.next_neighbour_after_helper = vertex_neighbours[vertex_index].second;

        status.insert(new_status_component);
    }

    void handle_regular_right(
        dcel::DCEL & dcel,
        std::set<StatusComponent, decltype(status_component_comparator)> & status,
        std::vector<VertexType> const & vertex_types,
        std::vector<std::pair<size_t, size_t>> const & vertex_neighbours,
        std::set<size_t> const & outside_faces,
        size_t vertex_index
    ) noexcept
    {
        std::pair<size_t, std::pair<size_t, size_t>> const current_and_previous_and_next_edges_to_current =
            get_current_and_previous_and_next_edges_to_current_vertex_by_neighbours(
                dcel,
                vertex_index,
                outside_faces,
                vertex_neighbours[vertex_index].first,
                vertex_neighbours[vertex_index].second);
        size_t const current_edge_index = current_and_previous_and_next_edges_to_current.first;
        size_t const previous_edge_to_current_index = current_and_previous_and_next_edges_to_current.second.first;
        size_t const next_edge_after_current_index = current_and_previous_and_next_edges_to_current.second.second;

        size_t future_previous_neighbour = vertex_neighbours[vertex_index].first;

        auto nearest_left_component = get_nearest_left_component(dcel, status, vertex_index);

        StatusComponent nearest_left = *nearest_left_component;

        if (vertex_types[nearest_left.helper] == VertexType::Merge)
        {
            size_t const helper = nearest_left.helper;
            size_t const previous_neighbour_to_helper = nearest_left.previous_neighbour_to_helper;
            size_t const next_neighbour_after_helper = nearest_left.next_neighbour_after_helper;

            size_t const helper_edge_index =
                get_current_and_previous_and_next_edges_to_current_vertex_by_neighbours(
                    dcel,
                    helper,
                    outside_faces,
                    previous_neighbour_to_helper,
                    next_neighbour_after_helper
                ).first;

            dcel::add_edge_between_two_edges(dcel, current_edge_index, helper_edge_index);

            future_previous_neighbour = helper;
        }

        status.erase(nearest_left_component);
        nearest_left.helper = vertex_index;
        nearest_left.previous_neighbour_to_helper = future_previous_neighbour;
        nearest_left.next_neighbour_after_helper = vertex_neighbours[vertex_index].second;

        status.insert(nearest_left);
    }

    void triangulation(dcel::DCEL & dcel) noexcept
    {
        std::set<size_t> const & outside_faces = get_outside_faces(dcel);

        std::vector<VertexType> vertex_types(dcel.vertices.size(), VertexType::Undefined);
        std::vector<VertexType> vertex_types_second(dcel.vertices.size(), VertexType::Undefined);
        std::vector<std::pair<size_t, size_t>> vertex_neighbours(dcel.vertices.size());
        std::vector<std::pair<size_t, size_t>> vertex_neighbours_second(dcel.vertices.size());

        std::vector<size_t> vertices(dcel.vertices.size());
        for (size_t i = 0; i < vertices.size(); ++i)
        {
            vertices[i] = i;
        }

        auto calculate_vertex_type = [&dcel](
            std::vector<std::pair<size_t, size_t>> & vertex_neighbours,
            std::vector<VertexType> & vertex_types,
            std::pair<size_t, size_t> previous_and_next_edges_to_current,
            size_t i)
        {
            size_t const previous_edge_to_current_index = previous_and_next_edges_to_current.first;
            size_t const next_edge_after_current_index = previous_and_next_edges_to_current.second;

            vertex_neighbours[i].first = dcel.edges[previous_edge_to_current_index].origin_vertex;
            vertex_neighbours[i].second = dcel.edges[next_edge_after_current_index].origin_vertex;

            Point const current_point = dcel.vertices[i].coordinate;
            Point const previous_point = dcel.vertices[dcel.edges[previous_edge_to_current_index].origin_vertex].coordinate;
            Point const next_point = dcel.vertices[dcel.edges[next_edge_after_current_index].origin_vertex].coordinate;

            float const angle = angle_between_vectors(previous_point - current_point, next_point - current_point);

            if (current_point.y < previous_point.y && current_point.y <= next_point.y && angle < epsilon)
            {
                vertex_types[i] = VertexType::Start;
            }
            else if (current_point.y <= previous_point.y && current_point.y < next_point.y && angle > -epsilon)
            {
                vertex_types[i] = VertexType::Split;
            }
            else if (current_point.y > previous_point.y && current_point.y >= next_point.y && angle < epsilon)
            {
                vertex_types[i] = VertexType::End;
            }
            else if (current_point.y >= previous_point.y && current_point.y > next_point.y && angle > -epsilon)
            {
                vertex_types[i] = VertexType::Merge;
            }
            else if (abs(current_point.y - next_point.y) < epsilon && abs(current_point.y - previous_point.y) < epsilon)
            {
                if (current_point.x < next_point.x)
                {
                    vertex_types[i] = VertexType::RegularRight;
                }
                else
                {
                    vertex_types[i] = VertexType::RegularLeft;
                }
            }
            else if (current_point.y >= next_point.y && current_point.y <= previous_point.y)
            {
                vertex_types[i] = VertexType::RegularLeft;
            }
            else if (current_point.y <= next_point.y && current_point.y >= previous_point.y)
            {
                vertex_types[i] = VertexType::RegularRight;
            }
        };

        for (size_t i = 0; i < dcel.vertices.size(); ++i)
        {
            std::vector<std::pair<size_t, std::pair<size_t, size_t>>> current_and_previous_and_next_edges_groups_to_current =
                get_current_and_previous_and_next_edges_groups_to_current_vertex(dcel, i, outside_faces);

            if (current_and_previous_and_next_edges_groups_to_current.empty())
            {
                vertices.erase(std::find(vertices.begin(), vertices.end(), i));
                continue;
            }
            if (current_and_previous_and_next_edges_groups_to_current.size() == 1)
            {
                calculate_vertex_type(vertex_neighbours, vertex_types, current_and_previous_and_next_edges_groups_to_current[0].second, i);
            }
            else
            {
                calculate_vertex_type(vertex_neighbours, vertex_types, current_and_previous_and_next_edges_groups_to_current[0].second, i);
                calculate_vertex_type(vertex_neighbours_second, vertex_types_second, current_and_previous_and_next_edges_groups_to_current[1].second, i);
            }
        }


        std::sort(vertices.begin(), vertices.end(), [&dcel](size_t a, size_t b) noexcept -> bool
            {
                frm::Point point_a = dcel.vertices[a].coordinate;
                frm::Point point_b = dcel.vertices[b].coordinate;

                if (abs(point_a.y - point_b.y) < frm::epsilon)
                {
                    return point_a.x < point_b.x;
                }

                return point_a.y < point_b.y;
            });

        std::set<StatusComponent, decltype(status_component_comparator)> status(status_component_comparator);

        auto handle_vertex = [&dcel, &status, &outside_faces](
            std::vector<std::pair<size_t, size_t>> & vertex_neighbours,
            std::vector<VertexType> & vertex_types,
            size_t i) noexcept
        {
            switch (vertex_types[i])
            {
            case VertexType::Start:
                handle_start(dcel, status, outside_faces, vertex_neighbours, i);
                break;
            case VertexType::Split:
                handle_split(dcel, status, vertex_types, vertex_neighbours, outside_faces, i);
                break;
            case VertexType::End:
                handle_end(dcel, status, vertex_types, vertex_neighbours, outside_faces, i);
                break;
            case VertexType::Merge:
                handle_merge(dcel, status, vertex_types, vertex_neighbours, outside_faces, i);
                break;
            case VertexType::RegularLeft:
                handle_regular_left(dcel, status, vertex_types, vertex_neighbours, outside_faces, i);
                break;
            case VertexType::RegularRight:
                handle_regular_right(dcel, status, vertex_types, vertex_neighbours, outside_faces, i);
                break;
            default:
                break;
            }
        };

        for (size_t i = 0; i < vertices.size(); ++i)
        {
            if (vertex_types_second[vertices[i]] == VertexType::Undefined)
            {
                handle_vertex(vertex_neighbours, vertex_types, vertices[i]);
            }
            else
            {
                if (vertex_types[vertices[i]] < vertex_types_second[vertices[i]])
                {
                    handle_vertex(vertex_neighbours, vertex_types, vertices[i]);
                    handle_vertex(vertex_neighbours_second, vertex_types_second, vertices[i]);
                }
                else
                {
                    handle_vertex(vertex_neighbours_second, vertex_types_second, vertices[i]);
                    handle_vertex(vertex_neighbours, vertex_types, vertices[i]);
                }
            }
        }
        
        size_t const face_count = dcel.faces.size();

        for (size_t i = 0; i < face_count; ++i)
        {
            if (!is_outside_face(dcel, outside_faces, i))
            {
                triangulation_y_monotone(dcel, i);
            }
        }
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