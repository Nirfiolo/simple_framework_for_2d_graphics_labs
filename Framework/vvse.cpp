#include "vvse.h"

#include <string>
#include <fstream>
#include <filesystem>
#include <cassert>


namespace frm
{
    //vector of vertices and set of edges 
    namespace vvse
    {
        std::ostream & operator<<(std::ostream & os, VVSE const & vvse) noexcept
        {
            os << "{ " << vvse.vertices.size() << '\n';
            for (size_t i = 0; i < vvse.vertices.size(); ++i)
            {
                os << "[ " << vvse.vertices[i].coordinate << " ] ";
            }
            os << '\n';

            os << vvse.edges.size() << '\n';
            for (VVSE::edge_t const & current : vvse.edges)
            {
                os << "[ " << current.first << " , " <<
                    current.second << " ] ";
            }
            os << '\n';

            os << "}\n";

            return os;
        }

        std::istream & operator>>(std::istream & is, VVSE & vvse) noexcept
        {
            std::string additional_symbols;

            is >> additional_symbols;
            size_t vertices_size;
            is >> vertices_size;

            vvse.vertices.resize(vertices_size);
            for (size_t i = 0; i < vvse.vertices.size(); ++i)
            {
                is >> additional_symbols >> vvse.vertices[i].coordinate >> additional_symbols;
            }

            size_t edges_size;
            is >> edges_size;

            for (size_t i = 0; i < edges_size; ++i)
            {
                VVSE::edge_t edge{};

                is >> additional_symbols >> edge.first >>
                    additional_symbols >> edge.second >> additional_symbols;

                vvse.edges.insert(edge);
            }

            is >> additional_symbols;

            return is;
        }

        void safe_to_file(std::string const & path, VVSE const & vvse) noexcept
        {
            std::ofstream file_output{ path };

            file_output << vvse;
        }

        void load_from_file(std::string const & path, VVSE & vvse) noexcept
        {
            std::ifstream file_input{ path };

            file_input >> vvse;
        }

        void add_vertex(VVSE & vvse, Point coordinate) noexcept
        {
            vvse.vertices.push_back({ coordinate });
        }

        void add_edge_between_two_vertices(VVSE & vvse, size_t begin_vertex_index, size_t end_vertex_index) noexcept(!IS_DEBUG)
        {
            assert(begin_vertex_index < vvse.vertices.size());
            assert(end_vertex_index < vvse.vertices.size());

            VVSE::edge_t edge{};
            edge.first = begin_vertex_index;
            edge.second = end_vertex_index;

            vvse.edges.insert(edge);
        }
    }
}