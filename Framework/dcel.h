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
                bool is_exist{ true };
            };

            struct Face
            {
                size_t edge;
                bool is_exist{ true };
            };

            struct Edge
            {
                size_t origin_vertex;
                size_t twin_edge;
                size_t incident_face;
                size_t next_edge;
                size_t previous_edge;
                bool is_exist{ true };
            };

            std::vector<Vertex> vertices;
            std::vector<size_t> free_vertices{};
            std::vector<Face> faces;
            std::vector<size_t> free_faces{};
            std::vector<Edge> edges;
            std::vector<size_t> free_edges{};
        };

        std::ostream & operator<<(std::ostream & os, DCEL const & dcel) noexcept;
        std::istream & operator>>(std::istream & is, DCEL & dcel) noexcept;

        void safe_to_file(std::string const & path, DCEL const & dcel) noexcept;
        void load_from_file(std::string const & path, DCEL & dcel) noexcept;

        size_t get_outside_face_index(frm::dcel::DCEL const & dcel) noexcept(!IS_DEBUG);

        std::vector<size_t> get_adjacent_vertices(DCEL const & dcel, size_t vertex_index) noexcept;

        // pair.first vertex index
        // pair.second edge index
        std::vector<std::pair<size_t, size_t>> get_adjacent_vertices_and_edges(DCEL const & dcel, size_t vertex_index) noexcept;

        bool is_points_connected(DCEL const & dcel, size_t begin_vertex_index, size_t end_vertex_index) noexcept;

        void add_vertex(DCEL & dcel, Point coordinate) noexcept;
        void add_vertex_and_split_edge(DCEL & dcel, Point coordinate, size_t edge_index) noexcept;
        void add_vertex_and_connect_to_edge_origin(DCEL & dcel, Point coordinate, size_t edge_index) noexcept;
        
        std::pair<size_t, size_t> add_edge_between_two_edges(DCEL & dcel, size_t begin_edge_index, size_t end_edge_index) noexcept;

        std::pair<size_t, size_t> add_edge_between_two_points(DCEL & dcel, size_t begin_vertex_index, size_t end_vertex_index) noexcept;

        void add_face_from_three_points(DCEL & dcel, size_t first_vertex_index, size_t second_vertex_index, size_t third_vertex_index, size_t face_index) noexcept;

        void remove_vertex_with_single_edge(DCEL & dcel, size_t vertex_index) noexcept;

        // TODO: add remove

        bool spawn_ui(DCEL & dcel,
            size_t current_vertex,
            size_t current_edge,
            size_t current_face,
            sf::RenderWindow & window,
            std::string const & path,
            bool & is_dirty) noexcept;

        void draw_face_highlighted(size_t face_index, DCEL const & dcel, float color[4], sf::RenderWindow & window) noexcept;

        void draw(DCEL & dcel, sf::RenderWindow & window, sf::Color const & color = sf::Color::White) noexcept;

        bool is_vertices_mode() noexcept;
        bool is_faces_mode() noexcept;
        bool is_edges_mode() noexcept;
    }
}