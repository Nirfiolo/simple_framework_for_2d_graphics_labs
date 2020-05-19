#include "vvve.h"
#include "common_ui_part.h"

#include "imgui/imgui.h"

#include <string>
#include <fstream>
#include <filesystem>
#include <cassert>


namespace frm
{
    //vector of vertices and vector of edges 
    namespace vvve
    {
        VVVE::edge_t get_edge_by_index(VVVE & vvve, size_t index) noexcept(!IS_DEBUG)
        {
            size_t current_index = 0;

            for (VVVE::edge_t const & current : vvve.edges)
            {
                if (index == current_index)
                {
                    return current;
                }

                ++current_index;
            }
            assert(false && "Incorrect index");
            return {};
        }

        bool show_vertices(VVVE & vvve, sf::RenderWindow & window) noexcept
        {
            bool is_dirty = false;

            ImGui::Columns(3);

            static size_t current = std::numeric_limits<size_t>::max();

            show_indexed_combo(current, vvve.vertices.size(), "Vertex");

            ImGui::NextColumn();

            if (current < vvve.vertices.size())
            {
                static float circle_color[4] = { 1.f, 0.0f, 0.f, 0.7f };
                static float radius = 10.f;

                Point & point = vvve.vertices[current].coordinate;
                draw_vertex_highlighted(point, circle_color, radius, window);

                is_dirty |= ImGui::SliderFloat("X", &point.x, 0.f, 1000.f);
                is_dirty |= ImGui::SliderFloat("Y", &point.y, 0.f, 1000.f);

                ImGui::SliderFloat("Radius", &radius, 0.01f, 100.f);
                ImGui::ColorEdit3("Circle", circle_color);
            }

            ImGui::NextColumn();

            static bool need_add_vertex = true;
            ImGui::Checkbox("Add Vertex", &need_add_vertex);
            if (need_add_vertex)
            {
                static float circle_color[4] = { 0.f, 1.0f, 0.f, 0.7f };
                static float radius = 10.f;

                static Point new_vertex = { 100.f, 100.f };

                draw_vertex_highlighted(new_vertex, circle_color, radius, window);

                ImGui::SliderFloat("new X", &new_vertex.x, 0.f, 1000.f);
                ImGui::SliderFloat("new Y", &new_vertex.y, 0.f, 1000.f);

                if (ImGui::Button("Add vertex"))
                {
                    add_vertex(vvve, new_vertex);
                    is_dirty = true;
                }
            }
            return is_dirty;
        }

        bool show_edges(VVVE & vvve, sf::RenderWindow & window) noexcept
        {
            bool is_dirty = false;

            ImGui::Columns(3);

            static size_t current = std::numeric_limits<size_t>::max();

            show_indexed_combo(current, vvve.edges.size(), "Edge");

            ImGui::NextColumn();

            if (current < vvve.edges.size())
            {
                static float color[4] = { 1.f, 0.0f, 0.f, 0.7f };
                static float width = 10.f;

                VVVE::edge_t edge = get_edge_by_index(vvve, current);

                size_t const begin_origin = edge.first;
                Point & begin_point = vvve.vertices[begin_origin].coordinate;
                size_t const end_origin = edge.second;
                Point & end_point = vvve.vertices[end_origin].coordinate;

                draw_edge_highlighted(begin_point, end_point, color, width, window);

                is_dirty |= ImGui::SliderFloat("X begin", &begin_point.x, 0.f, 1000.f);
                is_dirty |= ImGui::SliderFloat("Y begin", &begin_point.y, 0.f, 1000.f);

                is_dirty |= ImGui::SliderFloat("X end", &end_point.x, 0.f, 1000.f);
                is_dirty |= ImGui::SliderFloat("Y end", &end_point.y, 0.f, 1000.f);

                ImGui::SliderFloat("Width", &width, 0.01f, 100.f);
                ImGui::ColorEdit3("Line", color);
            }

            ImGui::NextColumn();

            static bool need_add_edge = true;
            ImGui::Checkbox("Add Edge", &need_add_edge);
            if (need_add_edge)
            {
                static float color[4] = { 0.f, 1.0f, 0.f, 0.7f };
                static float radius = 10.f;

                static size_t begin_vertex = std::numeric_limits<size_t>::max();
                static size_t end_vertex = std::numeric_limits<size_t>::max();

                show_indexed_combo(begin_vertex, vvve.vertices.size(), "Begin edge");
                show_indexed_combo(end_vertex, vvve.vertices.size(), "End edge");

                if (begin_vertex < vvve.vertices.size())
                {
                    Point const & point = vvve.vertices[begin_vertex].coordinate;
                    draw_vertex_highlighted(point, color, radius, window);
                }

                if (end_vertex < vvve.vertices.size())
                {
                    Point const & point = vvve.vertices[end_vertex].coordinate;
                    draw_vertex_highlighted(point, color, radius, window);
                }

                if (begin_vertex < vvve.vertices.size() && begin_vertex != end_vertex)
                {
                    if (ImGui::Button("Add adge between two vetrices"))
                    {
                        add_edge_between_two_vertices(vvve, begin_vertex, end_vertex);
                        is_dirty = true;
                    }
                }
            }
            return is_dirty;
        }

        bool spawn_ui(VVVE & vvve, sf::RenderWindow & window, std::string const & path) noexcept
        {
            bool is_dirty = false;

            std::string const name = std::filesystem::path{ path }.stem().string();

            static bool is_active = false;
            if (ImGui::Begin(name.c_str(), &is_active, ImGuiWindowFlags_MenuBar))
            {
                if (ImGui::BeginMenuBar())
                {
                    if (ImGui::MenuItem("Save"))
                    {
                        safe_to_file(path, vvve);
                    }
                    if (ImGui::MenuItem("Load"))
                    {
                        load_from_file(path, vvve);
                        is_dirty = true;
                    }
                    ImGui::EndMenuBar();
                }

                char const * items[] = { "Vertices", "Edges" };

                static char const * current_item = items[0];
                if (ImGui::BeginCombo("Type", current_item))
                {
                    for (size_t i = 0; i < std::size(items); ++i)
                    {
                        bool const is_selected = current_item == items[i];
                        if (ImGui::Selectable(items[i], is_selected))
                        {
                            current_item = items[i];
                        }

                        if (is_selected)
                        {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                ImGui::Separator();

                if (current_item == items[0])
                {
                    is_dirty |= show_vertices(vvve, window);
                }
                if (current_item == items[1])
                {
                    is_dirty |= show_edges(vvve, window);
                }
            }
            ImGui::End();

            return is_dirty;
        }

        void draw(VVVE const & vvve, sf::RenderWindow & window, sf::Color const & color) noexcept
        {
            sf::VertexArray vertices{ sf::Lines, 2 * vvve.edges.size() };

            size_t index = 0;

            for (VVVE::edge_t const & current : vvve.edges)
            {
                size_t const begin_origin = current.first;
                vertices[2 * index].position = { vvve.vertices[begin_origin].coordinate.x, vvve.vertices[begin_origin].coordinate.y };
                vertices[2 * index].color = color;

                size_t const end_origin = current.second;
                vertices[2 * index + 1].position = { vvve.vertices[end_origin].coordinate.x, vvve.vertices[end_origin].coordinate.y };
                vertices[2 * index + 1].color = color;

                ++index;
            }

            window.draw(vertices);

            for (size_t i = 0; i < vvve.vertices.size(); ++i)
            {
                float const radius = 3.f;
                Point const point = vvve.vertices[i].coordinate;

                sf::CircleShape circle{ radius };
                circle.setFillColor(color);
                circle.setOrigin(radius, radius);
                circle.setPosition(point.x, point.y);
                window.draw(circle);
            }
        }
    }
}