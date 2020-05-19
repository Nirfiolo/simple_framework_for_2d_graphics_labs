#pragma once

#include "common.h"

#include "SFML\Graphics.hpp"

#include <vector>
#include <unordered_set>


namespace frm
{
    //vector of vertices and set of edges 
    namespace vvse
    {
        //vector of vertices and set of edges 
        struct VVSE
        {
            struct Vertex
            {
                Point coordinate;
            };

            using edge_t = std::pair<size_t, size_t>;

            struct edge_hash
            {
                std::size_t operator() (edge_t const & edge) const noexcept
                {
                    std::size_t const hash_1 = std::hash<size_t>()(edge.first);
                    std::size_t const hash_2 = std::hash<size_t>()(edge.second);

                    return hash_1 ^ hash_2;
                }
            };

            std::vector<Vertex> vertices{};
            std::unordered_set<edge_t, edge_hash> edges{};

        };

        std::ostream & operator<<(std::ostream & os, VVSE const & vvse) noexcept;
        std::istream & operator>>(std::istream & is, VVSE & vvse) noexcept;

        void safe_to_file(std::string const & path, VVSE const & vvse) noexcept;
        void load_from_file(std::string const & path, VVSE & vvse) noexcept;

        void add_vertex(VVSE & vvse, Point coordinate) noexcept;

        void add_edge_between_two_vertices(VVSE & vvse, size_t begin_vertex_index, size_t end_vertex_index) noexcept(!IS_DEBUG);

        bool spawn_ui(VVSE & vvse, sf::RenderWindow & window, std::string const & path) noexcept;

        void draw(VVSE const & vvse, sf::RenderWindow & window, sf::Color const & color = sf::Color::White) noexcept;
    }
}
