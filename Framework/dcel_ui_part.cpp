#include "dcel.h"

#include "imgui/imgui.h"

#include <string>
#include <fstream>
#include <filesystem>


namespace frm
{
    namespace dcel
    {
        void show_indexed_combo(size_t & current, size_t size, char const * lable) noexcept
        {
            if (ImGui::BeginCombo(lable, current == std::numeric_limits<size_t>::max() ?
                "" :
                std::to_string(current).c_str()))
            {
                {
                    bool const is_selected = current == std::numeric_limits<size_t>::max();

                    if (ImGui::Selectable("", &is_selected))
                    {
                        current = std::numeric_limits<size_t>::max();
                    }

                    if (is_selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                for (size_t i = 0; i < size; ++i)
                {
                    bool const is_selected = current == i;

                    if (ImGui::Selectable(std::to_string(i).c_str(), &is_selected))
                    {
                        current = i;
                    }

                    if (is_selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
        }

        sf::Color float4_to_uint8_t4(float circle_color[4]) noexcept
        {
            return sf::Color(
                static_cast<sf::Uint8>(circle_color[0] * 255.f),
                static_cast<sf::Uint8>(circle_color[1] * 255.f),
                static_cast<sf::Uint8>(circle_color[2] * 255.f),
                static_cast<sf::Uint8>(circle_color[3] * 255.f)
            );
        }

        void draw_vertex_highlighted(Point point, float circle_color[4], float radius, sf::RenderWindow & window) noexcept
        {
            sf::CircleShape circle{ radius };
            circle.setFillColor(float4_to_uint8_t4(circle_color));
            circle.setOrigin(radius, radius);
            circle.setPosition(point.x, point.y);
            window.draw(circle);
        }

        void draw_face_highlighted(size_t face_index, DCEL const & dcel, float color[4], sf::RenderWindow & window) noexcept
        {
            sf::ConvexShape shape{};
            shape.setFillColor(float4_to_uint8_t4(color));

            size_t const begin = dcel.faces[face_index].edge;
            size_t current_index = begin;
            size_t current_number = 0;

            do
            {
                current_index = dcel.edges[current_index].next_edge;
                ++current_number;
            } while (current_index != begin);

            shape.setPointCount(current_number);

            current_index = begin;
            current_number = 0;

            do
            {
                Point point = dcel.vertices[dcel.edges[current_index].origin_vertex].coordinate;
                shape.setPoint(current_number, sf::Vector2f{ point.x, point.y });
                current_index = dcel.edges[current_index].next_edge;
                ++current_number;
            } while (current_index != begin);

            window.draw(shape);
        }

        void draw_edge_highlighted(Point begin_point, Point end_point, float color[4], float width, sf::RenderWindow & window) noexcept
        {
            sf::Vector2f begin{ begin_point.x, begin_point.y };
            sf::Vector2f end{ end_point.x, end_point.y };

            sf::Vector2f target{ end - begin };

            sf::RectangleShape line{ sf::Vector2f(sqrt(target.x * target.x + target.y * target.y), width) };
            line.setFillColor(float4_to_uint8_t4(color));
            line.setOrigin(0.f, width / 2.f);
            line.setRotation(atan2(target.y, target.x) * 180.f / 3.14f);
            line.setPosition(begin);
            window.draw(line);

            draw_vertex_highlighted(begin_point, color, width, window);
        }

        void show_vertices(DCEL & dcel, sf::RenderWindow & window) noexcept
        {
            ImGui::Columns(3);

            static size_t current = std::numeric_limits<size_t>::max();

            show_indexed_combo(current, dcel.vertices.size(), "Vertex");

            ImGui::NextColumn();

            if (current < dcel.vertices.size())
            {
                static float circle_color[4] = { 1.f, 0.0f, 0.f, 0.7f };
                static float radius = 10.f;

                Point & point = dcel.vertices[current].coordinate;
                draw_vertex_highlighted(point, circle_color, radius, window);

                ImGui::SliderFloat("X", &point.x, 0.f, 1000.f);
                ImGui::SliderFloat("Y", &point.y, 0.f, 1000.f);
                ImGui::Text("Edge &d", static_cast<int>(dcel.vertices[current].incident_edge));

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
                    add_vertex(dcel, new_vertex);
                }
            }
        }

        void show_faces(DCEL & dcel, sf::RenderWindow & window) noexcept
        {
            ImGui::Columns(2);

            static size_t current = std::numeric_limits<size_t>::max();

            show_indexed_combo(current, dcel.faces.size(), "Face");

            ImGui::NextColumn();

            if (current < dcel.faces.size())
            {
                static float color[4] = { 1.f, 0.0f, 0.f, 0.7f };

                draw_face_highlighted(current, dcel, color, window);

                ImGui::Text("Edge %d", static_cast<int>(dcel.faces[current].edge));

                ImGui::ColorEdit3("Face color", color);
            }
        }

        void show_edges(DCEL & dcel, sf::RenderWindow & window) noexcept
        {
            ImGui::Columns(3);

            static size_t current = std::numeric_limits<size_t>::max();

            show_indexed_combo(current, dcel.edges.size(), "Edge");

            ImGui::NextColumn();

            if (current < dcel.edges.size())
            {
                static float color[4] = { 1.f, 0.0f, 0.f, 0.7f };
                static float width = 10.f;

                size_t const begin_origin = dcel.edges[current].origin_vertex;
                Point & begin_point = dcel.vertices[begin_origin].coordinate;
                size_t const end_origin = dcel.edges[dcel.edges[current].twin_edge].origin_vertex;
                Point & end_point = dcel.vertices[end_origin].coordinate;

                draw_edge_highlighted(begin_point, end_point, color, width, window);

                ImGui::SliderFloat("X begin", &begin_point.x, 0.f, 1000.f);
                ImGui::SliderFloat("Y begin", &begin_point.y, 0.f, 1000.f);

                ImGui::SliderFloat("X end", &end_point.x, 0.f, 1000.f);
                ImGui::SliderFloat("Y end", &end_point.y, 0.f, 1000.f);

                ImGui::Text("Face %d", static_cast<int>(dcel.edges[current].incident_face));
                ImGui::Text("Twin %d", static_cast<int>(dcel.edges[current].twin_edge));
                ImGui::Text("Next %d", static_cast<int>(dcel.edges[current].next_edge));
                ImGui::Text("Previous %d", static_cast<int>(dcel.edges[current].privous_edge));

                ImGui::SliderFloat("Width", &width, 0.01f, 100.f);
                ImGui::ColorEdit3("Line", color);
            }

            ImGui::NextColumn();

            if (current < dcel.edges.size())
            {
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

                    if (ImGui::Button("Add vertex and connect to edge origin"))
                    {
                        add_vertex_and_connect_to_edge_origin(dcel, new_vertex, current);
                    }
                    if (ImGui::Button("Add vertex and split edge"))
                    {
                        add_vertex_and_split_edge(dcel, new_vertex, current);
                    }
                }
            }
            static bool need_add_edge = true;
            ImGui::Checkbox("Add Edge", &need_add_edge);
            if (need_add_edge)
            {
                static float color[4] = { 0.f, 1.0f, 0.f, 0.7f };
                static float width = 10.f;

                static size_t begin_edge = std::numeric_limits<size_t>::max();
                static size_t end_edge = std::numeric_limits<size_t>::max();

                show_indexed_combo(begin_edge, dcel.edges.size(), "Begin edge");
                show_indexed_combo(end_edge, dcel.edges.size(), "End edge");

                if (begin_edge < dcel.edges.size())
                {
                    size_t const begin_origin = dcel.edges[begin_edge].origin_vertex;
                    Point & begin_point = dcel.vertices[begin_origin].coordinate;
                    size_t const end_origin = dcel.edges[dcel.edges[begin_edge].twin_edge].origin_vertex;
                    Point & end_point = dcel.vertices[end_origin].coordinate;

                    draw_edge_highlighted(begin_point, end_point, color, width, window);
                }

                if (end_edge < dcel.edges.size())
                {
                    size_t const begin_origin = dcel.edges[end_edge].origin_vertex;
                    Point & begin_point = dcel.vertices[begin_origin].coordinate;
                    size_t const end_origin = dcel.edges[dcel.edges[end_edge].twin_edge].origin_vertex;
                    Point & end_point = dcel.vertices[end_origin].coordinate;

                    draw_edge_highlighted(begin_point, end_point, color, width, window);
                }

                if (begin_edge < dcel.edges.size() && begin_edge != end_edge)
                {
                    if (ImGui::Button("Add adge between two edges"))
                    {
                        add_edge_between_two_edges(dcel, begin_edge, end_edge);
                    }
                }
            }
        }

        void spawn_ui(DCEL & dcel, sf::RenderWindow & window, std::string const & path) noexcept
        {
            std::string const name = std::filesystem::path{ path }.stem().string();

            static bool is_active = false;
            if (ImGui::Begin(name.c_str(), &is_active, ImGuiWindowFlags_MenuBar))
            {
                if (ImGui::BeginMenuBar())
                {
                    if (ImGui::MenuItem("Save"))
                    {
                        safe_to_file(path, dcel);
                    }
                    if (ImGui::MenuItem("Load"))
                    {
                        load_from_file(path, dcel);
                    }
                    ImGui::EndMenuBar();
                }

                char const * items[] = { "Vertices", "Faces", "Edges" };

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
                    show_vertices(dcel, window);
                }
                if (current_item == items[1])
                {
                    show_faces(dcel, window);
                }
                if (current_item == items[2])
                {
                    show_edges(dcel, window);
                }
            }
            ImGui::End();
        }

        void draw(DCEL & dcel, sf::RenderWindow & window, sf::Color const & color) noexcept
        {
            sf::VertexArray vertices{ sf::Lines, 2 * dcel.edges.size() };

            for (size_t i = 0; i < dcel.edges.size(); ++i)
            {
                size_t const begin_origin = dcel.edges[i].origin_vertex;
                vertices[2 * i].position = { dcel.vertices[begin_origin].coordinate.x, dcel.vertices[begin_origin].coordinate.y };
                vertices[2 * i].color = color;

                size_t const end_origin = dcel.edges[dcel.edges[i].twin_edge].origin_vertex;
                vertices[2 * i + 1].position = { dcel.vertices[end_origin].coordinate.x, dcel.vertices[end_origin].coordinate.y };
                vertices[2 * i + 1].color = color;
            }

            window.draw(vertices);
        }
    }
}