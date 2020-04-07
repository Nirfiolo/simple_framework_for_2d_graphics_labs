#pragma once


#include "common.h"

#include "SFML\Graphics.hpp"

#include <vector>
#include <iostream>


namespace frm
{
    // Doubly Connected Edge List
    namespace dcel
    {
        struct DCEL
        {
            struct Vertex
            {
                Point coordinate;
                size_t incident_edge;
            };

            struct Face
            {
                size_t edge;
            };

            struct Edge
            {
                size_t origin_vertex;
                size_t twin_edge;
                size_t incident_face;
                size_t next_edge;
                size_t privous_edge;
            };

            std::vector<Vertex> vertices;
            std::vector<Face> faces;
            std::vector<Edge> edges;
        };

        std::ostream & operator<<(std::ostream & os, DCEL const & dcel) noexcept;
        std::istream & operator>>(std::istream & is, DCEL & dcel) noexcept;

        void safe_to_file(std::string const & path, DCEL const & dcel) noexcept;
        void load_from_file(std::string const & path, DCEL & dcel) noexcept;

        void spawn_ui(DCEL & dcel, sf::RenderWindow & window, std::string const & path) noexcept;

        void draw(DCEL & dcel, sf::RenderWindow & window, sf::Color const & color = sf::Color::White) noexcept;
    }
}