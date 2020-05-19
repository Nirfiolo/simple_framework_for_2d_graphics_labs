#pragma once

#include "common.h"

#include "SFML\Graphics.hpp"

#include <vector>


namespace frm
{
    //vector of vertices and vector of edges 
    namespace vvve
    {
        //vector of vertices and vector of edges 
        struct VVVE
        {
            struct Vertex
            {
                Point coordinate;
            };

            using edge_t = std::pair<size_t, size_t>;

            std::vector<Vertex> vertices{};
            std::vector<edge_t> edges{};
        };

        std::ostream & operator<<(std::ostream & os, VVVE const & vvve) noexcept;
        std::istream & operator>>(std::istream & is, VVVE & vvve) noexcept;

        void safe_to_file(std::string const & path, VVVE const & vvve) noexcept;
        void load_from_file(std::string const & path, VVVE & vvve) noexcept;

        void add_vertex(VVVE & vvve, Point coordinate) noexcept;

        void add_edge_between_two_vertices(VVVE & vvve, size_t begin_vertex_index, size_t end_vertex_index) noexcept(!IS_DEBUG);

        bool spawn_ui(VVVE & vvve, sf::RenderWindow & window, std::string const & path) noexcept;

        void draw(VVVE const & vvve, sf::RenderWindow & window, sf::Color const & color = sf::Color::White) noexcept;
    }
}
