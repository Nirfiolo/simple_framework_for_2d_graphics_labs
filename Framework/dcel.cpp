#include "dcel.h"

#include "imgui/imgui.h"

#include <string>
#include <fstream>
#include <filesystem>


namespace frm
{
    namespace dcel
    {
        std::ostream & operator<<(std::ostream & os, DCEL const & dcel) noexcept
        {
            os << "{ " << dcel.vertices.size() << '\n';
            for (size_t i = 0; i < dcel.vertices.size(); ++i)
            {
                os << "[ " << dcel.vertices[i].coordinate << " , " <<
                    dcel.vertices[i].incident_edge << " , " <<
                    dcel.vertices[i].is_exist << " ] ";
            }
            os << '\n';

            os << dcel.faces.size() << '\n';
            for (size_t i = 0; i < dcel.faces.size(); ++i)
            {
                os << "[ " << dcel.faces[i].edge << " , " <<
                    dcel.faces[i].is_exist << " ] ";
            }
            os << '\n';

            os << dcel.edges.size() << '\n';
            for (size_t i = 0; i < dcel.edges.size(); ++i)
            {
                os << "[ " << dcel.edges[i].origin_vertex << " , " <<
                    dcel.edges[i].twin_edge << " , " <<
                    dcel.edges[i].incident_face << " , " <<
                    dcel.edges[i].next_edge << " , " <<
                    dcel.edges[i].previous_edge << " , " <<
                    dcel.edges[i].is_exist << " ] ";
            }
            os << '\n';

            os << dcel.free_vertices.size() << '\n';
            for (size_t i = 0; i < dcel.free_vertices.size(); ++i)
            {
                os << "[ " << dcel.free_vertices[i] << " ] ";
            }
            os << '\n';

            os << dcel.free_faces.size() << '\n';
            for (size_t i = 0; i < dcel.free_faces.size(); ++i)
            {
                os << "[ " << dcel.free_faces[i] << " ] ";
            }
            os << '\n';

            os << dcel.free_edges.size() << '\n';
            for (size_t i = 0; i < dcel.free_edges.size(); ++i)
            {
                os << "[ " << dcel.free_edges[i] << " ] ";
            }
            os << '\n';

            os << "}\n";

            return os;
        }

        std::istream & operator>>(std::istream & is, DCEL & dcel) noexcept
        {
            std::string additional_symbols;

            is >> additional_symbols;
            size_t vertices_size;
            is >> vertices_size;

            dcel.vertices.resize(vertices_size);
            for (size_t i = 0; i < dcel.vertices.size(); ++i)
            {
                is >> additional_symbols >> dcel.vertices[i].coordinate >>
                    additional_symbols >> dcel.vertices[i].incident_edge >>
                    additional_symbols >> dcel.vertices[i].is_exist >> additional_symbols;
            }

            size_t faces_size;
            is >> faces_size;

            dcel.faces.resize(faces_size);
            for (size_t i = 0; i < dcel.faces.size(); ++i)
            {
                is >> additional_symbols >> dcel.faces[i].edge >>
                    additional_symbols >> dcel.faces[i].is_exist >> additional_symbols;
            }

            size_t edges_size;
            is >> edges_size;

            dcel.edges.resize(edges_size);
            for (size_t i = 0; i < dcel.edges.size(); ++i)
            {
                is >> additional_symbols >> dcel.edges[i].origin_vertex >>
                    additional_symbols >> dcel.edges[i].twin_edge >>
                    additional_symbols >> dcel.edges[i].incident_face >>
                    additional_symbols >> dcel.edges[i].next_edge >>
                    additional_symbols >> dcel.edges[i].previous_edge >>
                    additional_symbols >> dcel.edges[i].is_exist >> additional_symbols;
            }

            size_t free_vertices_size;
            is >> free_vertices_size;

            dcel.free_vertices.resize(free_vertices_size);
            for (size_t i = 0; i < dcel.free_vertices.size(); ++i)
            {
                is >> additional_symbols >> dcel.free_vertices[i] >> additional_symbols;
            }

            size_t free_faces_size;
            is >> free_faces_size;

            dcel.free_faces.resize(free_faces_size);
            for (size_t i = 0; i < dcel.free_faces.size(); ++i)
            {
                is >> additional_symbols >> dcel.free_faces[i] >> additional_symbols;
            }

            size_t free_edges_size;
            is >> free_edges_size;

            dcel.free_edges.resize(free_edges_size);
            for (size_t i = 0; i < dcel.free_edges.size(); ++i)
            {
                is >> additional_symbols >> dcel.free_edges[i] >> additional_symbols;
            }

            return is;
        }

        void safe_to_file(std::string const & path, DCEL const & dcel) noexcept
        {
            std::ofstream file_output{ path };

            file_output << dcel;
        }

        void load_from_file(std::string const & path, DCEL & dcel) noexcept
        {
            std::ifstream file_input{ path };

            file_input >> dcel;
        }

        size_t get_outside_face_index(frm::dcel::DCEL const & dcel) noexcept(!IS_DEBUG)
        {
            size_t left_vertex_index = 0;
            frm::Point left_vertex_point = dcel.vertices[0].coordinate;

            for (size_t i = 0; i < dcel.vertices.size(); ++i)
            {
                frm::Point current_point = dcel.vertices[i].coordinate;

                if (abs(current_point.x - left_vertex_point.x) < frm::epsilon)
                {
                    if (current_point.y - left_vertex_point.y < frm::epsilon)
                    {
                        left_vertex_point = current_point;
                        left_vertex_index = i;
                    }
                }
                else if (current_point.x - left_vertex_point.x < frm::epsilon)
                {
                    left_vertex_point = current_point;
                    left_vertex_index = i;
                }
            }

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

        std::vector<size_t> get_adjacent_vertices(DCEL const & dcel, size_t vertex_index) noexcept
        {
            std::vector<std::pair<size_t, size_t>> adjacent_vertices_and_edges = get_adjacent_vertices_and_edges(dcel, vertex_index);

            std::vector<size_t> adjacents(adjacent_vertices_and_edges.size());

            for (size_t i = 0; i < adjacents.size(); ++i)
            {
                adjacents[i] = adjacent_vertices_and_edges[i].first;
            }

            return adjacents;
        }

        std::vector<std::pair<size_t, size_t>> get_adjacent_vertices_and_edges(DCEL const & dcel, size_t vertex_index) noexcept
        {
            std::vector<std::pair<size_t, size_t>> adjacents{};

            size_t const begin_edge = dcel.vertices[vertex_index].incident_edge;
            assert(dcel.edges[begin_edge].origin_vertex == vertex_index);

            size_t current_edge = begin_edge;

            do
            {
                size_t const twin = dcel.edges[current_edge].twin_edge;
                adjacents.push_back({ dcel.edges[twin].origin_vertex, current_edge });
                current_edge = dcel.edges[twin].next_edge;
            } while (current_edge != begin_edge);

            return adjacents;
        }

        template<typename T_1, typename T_2>
        size_t get_free_index(std::vector<T_1> & free_indices, std::vector<T_2> & data) noexcept
        {
            if (free_indices.empty())
            {
                size_t const index = data.size();
                data.push_back({});
                return index;
            }

            size_t const index = free_indices.back();
            free_indices.pop_back();
            data[index] = {};
            return index;
        }

        size_t get_free_vertex_index(DCEL & dcel) noexcept
        {
            return get_free_index(dcel.free_vertices, dcel.vertices);
        }

        size_t get_free_face_index(DCEL & dcel) noexcept
        {
            return get_free_index(dcel.free_faces, dcel.faces);
        }

        size_t get_free_edge_index(DCEL & dcel) noexcept
        {
            return get_free_index(dcel.free_edges, dcel.edges);
        }

        void add_vertex(DCEL & dcel, Point coordinate) noexcept
        {
            size_t const vertex_index = get_free_vertex_index(dcel);
            dcel.vertices[vertex_index] = { coordinate, std::numeric_limits<size_t>::max() };
        }

        void add_vertex_and_split_edge(DCEL & dcel, Point coordinate, size_t edge_index) noexcept
        {
            size_t const current_edge_index = edge_index;
            size_t const next_after_current_edge_index = dcel.edges[current_edge_index].next_edge;

            size_t const twin_edge_index = dcel.edges[edge_index].twin_edge;
            size_t const next_after_twin_edge_index = dcel.edges[twin_edge_index].next_edge;

            size_t const current_vertex_index = dcel.edges[current_edge_index].origin_vertex;

            size_t const new_vertex_index = get_free_vertex_index(dcel);
            dcel.vertices[new_vertex_index] = { coordinate, std::numeric_limits<size_t>::max() };

            DCEL::Vertex & new_vertex = dcel.vertices[new_vertex_index];

            size_t const edge_from_new_to_next_index = get_free_edge_index(dcel);

            size_t const edge_from_new_to_current_index = get_free_edge_index(dcel);

            DCEL::Edge & current_edge = dcel.edges[current_edge_index];
            DCEL::Edge & next_after_current_edge = dcel.edges[next_after_current_edge_index];
            DCEL::Edge & twin_edge = dcel.edges[twin_edge_index];
            DCEL::Edge & next_after_twin_edge = dcel.edges[next_after_twin_edge_index];
            DCEL::Edge & edge_from_new_to_next = dcel.edges[edge_from_new_to_next_index];
            DCEL::Edge & edge_from_new_to_current = dcel.edges[edge_from_new_to_current_index];

            new_vertex.incident_edge = edge_from_new_to_next_index;

            edge_from_new_to_next.origin_vertex = new_vertex_index;
            edge_from_new_to_next.twin_edge = twin_edge_index;
            edge_from_new_to_next.incident_face = current_edge.incident_face;
            edge_from_new_to_next.next_edge = next_after_current_edge_index;
            edge_from_new_to_next.previous_edge = current_edge_index;

            edge_from_new_to_current.origin_vertex = new_vertex_index;
            edge_from_new_to_current.twin_edge = current_edge_index;
            edge_from_new_to_current.incident_face = twin_edge.incident_face;
            edge_from_new_to_current.next_edge = next_after_twin_edge_index;
            edge_from_new_to_current.previous_edge = twin_edge_index;

            current_edge.next_edge = edge_from_new_to_next_index;
            current_edge.twin_edge = edge_from_new_to_current_index;

            next_after_current_edge.previous_edge = edge_from_new_to_next_index;

            twin_edge.next_edge = edge_from_new_to_current_index;
            twin_edge.twin_edge = edge_from_new_to_next_index;

            next_after_twin_edge.previous_edge = edge_from_new_to_current_index;
        }

        void add_vertex_and_connect_to_edge_origin(DCEL & dcel, Point coordinate, size_t edge_index) noexcept
        {
            size_t const current_edge_index = edge_index;
            size_t const previous_to_current_edge_index = dcel.edges[edge_index].previous_edge;

            size_t const current_vertex_index = dcel.edges[current_edge_index].origin_vertex;
            size_t const new_vertex_index = get_free_vertex_index(dcel);
            dcel.vertices[new_vertex_index] = { coordinate, std::numeric_limits<size_t>::max() };
            DCEL::Vertex & new_vertex = dcel.vertices[new_vertex_index];

            size_t const edge_to_new_index = get_free_edge_index(dcel);
            size_t const edge_from_new_index = get_free_edge_index(dcel);

            DCEL::Edge & current_edge = dcel.edges[current_edge_index];
            DCEL::Edge & previous_to_current_edge = dcel.edges[previous_to_current_edge_index];
            DCEL::Edge & edge_to_new = dcel.edges[edge_to_new_index];
            DCEL::Edge & edge_from_new = dcel.edges[edge_from_new_index];

            new_vertex.incident_edge = edge_from_new_index;

            edge_to_new.origin_vertex = current_vertex_index;
            edge_to_new.twin_edge = edge_from_new_index;
            edge_to_new.incident_face = current_edge.incident_face;
            edge_to_new.next_edge = edge_from_new_index;
            edge_to_new.previous_edge = previous_to_current_edge_index;

            edge_from_new.origin_vertex = new_vertex_index;
            edge_from_new.twin_edge = edge_to_new_index;
            edge_from_new.incident_face = current_edge.incident_face;
            edge_from_new.next_edge = current_edge_index;
            edge_from_new.previous_edge = edge_to_new_index;

            current_edge.previous_edge = edge_from_new_index;

            previous_to_current_edge.next_edge = edge_to_new_index;
        }

        std::pair<size_t, size_t> add_edge_between_two_edges(DCEL & dcel, size_t begin_edge_index, size_t end_edge_index) noexcept
        {
            size_t const previous_to_begin_edge_index = dcel.edges[begin_edge_index].previous_edge;
            size_t const previous_to_end_edge_index = dcel.edges[end_edge_index].previous_edge;

            size_t const begin_vertex_index = dcel.edges[begin_edge_index].origin_vertex;
            size_t const end_vertex_index = dcel.edges[end_edge_index].origin_vertex;

            size_t const current_face_index = dcel.edges[begin_edge_index].incident_face;
            size_t const new_face_index = get_free_face_index(dcel);

            size_t const edge_from_begin_to_end_index = get_free_edge_index(dcel);
            size_t const edge_from_end_to_begin_index = get_free_edge_index(dcel);

            DCEL::Edge & begin_edge = dcel.edges[begin_edge_index];
            DCEL::Edge & previous_to_begin_edge = dcel.edges[previous_to_begin_edge_index];
            DCEL::Edge & end_edge = dcel.edges[end_edge_index];
            DCEL::Edge & previous_to_end_edge = dcel.edges[previous_to_end_edge_index];
            DCEL::Edge & edge_from_begin_to_end = dcel.edges[edge_from_begin_to_end_index];
            DCEL::Edge & edge_from_end_to_begin = dcel.edges[edge_from_end_to_begin_index];

            DCEL::Face & current_face = dcel.faces[current_face_index];
            DCEL::Face & new_face = dcel.faces[new_face_index];

            current_face.edge = edge_from_end_to_begin_index;
            new_face.edge = edge_from_begin_to_end_index;

            edge_from_begin_to_end.origin_vertex = begin_vertex_index;
            edge_from_begin_to_end.twin_edge = edge_from_end_to_begin_index;
            edge_from_begin_to_end.incident_face = new_face_index;
            edge_from_begin_to_end.next_edge = end_edge_index;
            edge_from_begin_to_end.previous_edge = previous_to_begin_edge_index;

            edge_from_end_to_begin.origin_vertex = end_vertex_index;
            edge_from_end_to_begin.twin_edge = edge_from_begin_to_end_index;
            edge_from_end_to_begin.incident_face = current_face_index;
            edge_from_end_to_begin.next_edge = begin_edge_index;
            edge_from_end_to_begin.previous_edge = previous_to_end_edge_index;

            begin_edge.previous_edge = edge_from_end_to_begin_index;

            previous_to_begin_edge.next_edge = edge_from_begin_to_end_index;

            end_edge.previous_edge = edge_from_begin_to_end_index;

            previous_to_end_edge.next_edge = edge_from_end_to_begin_index;

            size_t const begin = edge_from_begin_to_end_index;
            size_t current_index = begin;

            do
            {
                dcel.edges[current_index].incident_face = new_face_index;
                current_index = dcel.edges[current_index].next_edge;
            } while (current_index != begin);

            if (dcel.edges[dcel.faces[current_face_index].edge].incident_face != current_face_index)
            {
                dcel.free_faces.push_back(current_face_index);
                dcel.faces[current_face_index].is_exist = false;

                // TODO: make more productive

                for (size_t i = 0; i < dcel.edges.size(); ++i)
                {
                    if (dcel.edges[i].incident_face == current_face_index)
                    {
                        dcel.edges[i].incident_face = new_face_index;
                    }
                }
            }

            return { edge_from_begin_to_end_index, edge_from_end_to_begin_index };
        }

        void add_face_from_three_points(DCEL & dcel, size_t first_vertex_index, size_t second_vertex_index, size_t third_vertex_index, size_t face_index) noexcept
        {
            size_t const new_face_index = get_free_face_index(dcel);

            size_t const from_first_to_second_index = get_free_edge_index(dcel);
            size_t const from_second_to_first_index = get_free_edge_index(dcel);
            size_t const from_third_to_second_index = get_free_edge_index(dcel);
            size_t const from_second_to_third_index = get_free_edge_index(dcel);
            size_t const from_third_to_first_index = get_free_edge_index(dcel);
            size_t const from_first_to_third_index = get_free_edge_index(dcel);

            DCEL::Face & new_face = dcel.faces[new_face_index];
            new_face.edge = from_first_to_second_index;

            DCEL::Vertex & first_vertex = dcel.vertices[first_vertex_index];
            DCEL::Vertex & second_vertex = dcel.vertices[second_vertex_index];
            DCEL::Vertex & third_vertex = dcel.vertices[third_vertex_index];

            first_vertex.incident_edge = from_first_to_second_index;
            second_vertex.incident_edge = from_second_to_third_index;
            third_vertex.incident_edge = from_third_to_first_index;

            DCEL::Edge & from_first_to_second = dcel.edges[from_first_to_second_index];
            DCEL::Edge & from_second_to_first = dcel.edges[from_second_to_first_index];
            DCEL::Edge & from_third_to_second = dcel.edges[from_third_to_second_index];
            DCEL::Edge & from_second_to_third = dcel.edges[from_second_to_third_index];
            DCEL::Edge & from_third_to_first = dcel.edges[from_third_to_first_index];
            DCEL::Edge & from_first_to_third = dcel.edges[from_first_to_third_index];

            from_first_to_second.origin_vertex = first_vertex_index;
            from_first_to_second.twin_edge = from_second_to_first_index;
            from_first_to_second.incident_face = new_face_index;
            from_first_to_second.next_edge = from_second_to_third_index;
            from_first_to_second.previous_edge = from_third_to_first_index;

            from_second_to_first.origin_vertex = second_vertex_index;
            from_second_to_first.twin_edge = from_first_to_second_index;
            from_second_to_first.incident_face = face_index;
            from_second_to_first.next_edge = from_first_to_third_index;
            from_second_to_first.previous_edge = from_third_to_second_index;

            from_second_to_third.origin_vertex = second_vertex_index;
            from_second_to_third.twin_edge = from_third_to_second_index;
            from_second_to_third.incident_face = new_face_index;
            from_second_to_third.next_edge = from_third_to_first_index;
            from_second_to_third.previous_edge = from_first_to_second_index;

            from_third_to_second.origin_vertex = third_vertex_index;
            from_third_to_second.twin_edge = from_second_to_third_index;
            from_third_to_second.incident_face = face_index;
            from_third_to_second.next_edge = from_second_to_first_index;
            from_third_to_second.previous_edge = from_first_to_third_index;

            from_third_to_first.origin_vertex = third_vertex_index;
            from_third_to_first.twin_edge = from_first_to_third_index;
            from_third_to_first.incident_face = new_face_index;
            from_third_to_first.next_edge = from_first_to_second_index;
            from_third_to_first.previous_edge = from_second_to_third_index;

            from_first_to_third.origin_vertex = first_vertex_index;
            from_first_to_third.twin_edge = from_third_to_first_index;
            from_first_to_third.incident_face = face_index;
            from_first_to_third.next_edge = from_third_to_second_index;
            from_first_to_third.previous_edge = from_second_to_first_index;
        }

        void remove_vertex_with_single_edge(DCEL & dcel, size_t vertex_index) noexcept
        {
            dcel.free_vertices.push_back(vertex_index);
            dcel.vertices[vertex_index].is_exist = false;

            size_t const from_vertex_index = dcel.vertices[vertex_index].incident_edge;
            size_t const to_vertex_index = dcel.edges[from_vertex_index].twin_edge;

            size_t const face_index = dcel.edges[from_vertex_index].incident_face;

            dcel.free_edges.push_back(from_vertex_index);
            dcel.edges[from_vertex_index].is_exist = false;
            dcel.free_edges.push_back(to_vertex_index);
            dcel.edges[to_vertex_index].is_exist = false;

            size_t const previous_to_to_vertex_index = dcel.edges[to_vertex_index].previous_edge;
            size_t const next_after_from_vertex_index = dcel.edges[from_vertex_index].next_edge;

            DCEL::Edge & previous_to_to_vertex = dcel.edges[previous_to_to_vertex_index];
            DCEL::Edge & next_after_from_vertex = dcel.edges[next_after_from_vertex_index];

            previous_to_to_vertex.next_edge = next_after_from_vertex_index;
            next_after_from_vertex.previous_edge = previous_to_to_vertex_index;

            if (dcel.faces[face_index].edge == from_vertex_index || dcel.faces[face_index].edge == to_vertex_index)
            {
                dcel.faces[face_index].edge = previous_to_to_vertex_index;
            }
        }
    }
}