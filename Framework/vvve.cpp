#include "vvve.h"

#include <string>
#include <fstream>
#include <filesystem>
#include <cassert>


namespace frm
{
    //vector of vertices and vector of edges 
    namespace vvve
    {
        std::ostream & operator<<(std::ostream & os, VVVE const & vvve) noexcept
        {
            os << "{ " << vvve.vertices.size() << '\n';
            for (size_t i = 0; i < vvve.vertices.size(); ++i)
            {
                os << "[ " << vvve.vertices[i].coordinate << " ] ";
            }
            os << '\n';

            os << vvve.edges.size() << '\n';
            for (size_t i = 0; i < vvve.edges.size(); ++i)
            {
                os << "[ " << vvve.edges[i].first << " , " <<
                    vvve.edges[i].second << " ] ";
            }
            os << '\n';

            os << "}\n";

            return os;
        }

        std::istream & operator>>(std::istream & is, VVVE & vvve) noexcept
        {
            std::string additional_symbols;

            is >> additional_symbols;
            size_t vertices_size;
            is >> vertices_size;

            vvve.vertices.resize(vertices_size);
            for (size_t i = 0; i < vvve.vertices.size(); ++i)
            {
                is >> additional_symbols >> vvve.vertices[i].coordinate >> additional_symbols;
            }

            size_t edges_size;
            is >> edges_size;

            vvve.edges.resize(vertices_size);
            for (size_t i = 0; i < edges_size; ++i)
            {
                is >> additional_symbols >> vvve.edges[i].first >>
                    additional_symbols >> vvve.edges[i].second >> additional_symbols;
            }

            is >> additional_symbols;

            return is;
        }

        void safe_to_file(std::string const & path, VVVE const & vvve) noexcept
        {
            std::ofstream file_output{ path };

            file_output << vvve;
        }

        void load_from_file(std::string const & path, VVVE & vvve) noexcept
        {
            std::ifstream file_input{ path };

            file_input >> vvve;
        }

        void add_vertex(VVVE & vvve, Point coordinate) noexcept
        {
            vvve.vertices.push_back({ coordinate });
        }

        void add_edge_between_two_vertices(VVVE & vvve, size_t begin_vertex_index, size_t end_vertex_index) noexcept(!IS_DEBUG)
        {
            assert(begin_vertex_index < vvve.vertices.size());
            assert(end_vertex_index < vvve.vertices.size());

            VVVE::edge_t edge{};
            edge.first = begin_vertex_index;
            edge.second = end_vertex_index;

            vvve.edges.push_back(edge);
        }
    }
}