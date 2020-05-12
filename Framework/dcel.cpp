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

        void add_edge_between_two_edges(DCEL & dcel, size_t begin_edge_index, size_t end_edge_index) noexcept
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