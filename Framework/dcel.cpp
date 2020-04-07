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
                    dcel.vertices[i].incident_edge << " ] ";
            }
            os << '\n';

            os << dcel.faces.size() << '\n';
            for (size_t i = 0; i < dcel.faces.size(); ++i)
            {
                os << "[ " << dcel.faces[i].edge << " ] ";
            }
            os << '\n';

            os << dcel.edges.size() << '\n';
            for (size_t i = 0; i < dcel.edges.size(); ++i)
            {
                os << "[ " << dcel.edges[i].origin_vertex << " , " <<
                    dcel.edges[i].twin_edge << " , " <<
                    dcel.edges[i].incident_face << " , " << 
                    dcel.edges[i].next_edge << " , " << 
                    dcel.edges[i].privous_edge << " ] ";
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
                    additional_symbols >> dcel.vertices[i].incident_edge >> additional_symbols;
            }

            size_t faces_size;
            is >> faces_size;

            dcel.faces.resize(faces_size);
            for (size_t i = 0; i < dcel.faces.size(); ++i)
            {
                is >> additional_symbols >> dcel.faces[i].edge >> additional_symbols;
            }

            size_t edges_size;
            is >> edges_size;

            dcel.edges.resize(edges_size);
            for (size_t i = 0; i < dcel.edges.size(); ++i)
            {
                is >> additional_symbols >> dcel.edges[i].origin_vertex >> additional_symbols >>
                    dcel.edges[i].twin_edge >> additional_symbols >>
                    dcel.edges[i].incident_face >> additional_symbols >>
                    dcel.edges[i].next_edge >> additional_symbols >>
                    dcel.edges[i].privous_edge >> additional_symbols;
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

        void show_indexed_combo(size_t & current, size_t size, char const * lable) noexcept
        {
            if (ImGui::BeginCombo(lable, current == std::numeric_limits<size_t>::max() ?
                "" :
                std::to_string(current).c_str()))
            {
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
                Point point = dcel.vertices[dcel.edges[current_index].origin_vertex].coordinate;
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

        void draw_edge_highlighted(Point begin_point, Point end_point, float rectangle_color[4], float width, sf::RenderWindow & window) noexcept
        {
            sf::Vector2f begin{ begin_point.x, begin_point.y };
            sf::Vector2f end{ end_point.x, end_point.y };

            sf::Vector2f target{ end - begin };

            sf::RectangleShape line{ sf::Vector2f(sqrt(target.x * target.x + target.y * target.y), width) };
            line.setFillColor(float4_to_uint8_t4(rectangle_color));
            line.setOrigin(0.f, width / 2.f);
            line.setRotation(atan2(target.y, target.x) * 180.f / 3.14f);
            line.setPosition(begin);
            window.draw(line);
        }

        void show_vertices(DCEL & dcel, sf::RenderWindow & window) noexcept
        {
            ImGui::Columns(2);

            static size_t current = std::numeric_limits<size_t>::max();

            show_indexed_combo(current, dcel.vertices.size(), "Vertex");

            ImGui::NextColumn();

            if (current != std::numeric_limits<size_t>::max())
            {
                static float circle_color[4] = { 1.f, 0.0f, 0.f, 0.7f };
                static float radius = 10.f;

                Point & point = dcel.vertices[current].coordinate;
                draw_vertex_highlighted(point, circle_color, radius, window);

                ImGui::SliderFloat("X", &point.x, 0.f, 1000.f);
                ImGui::SliderFloat("Y", &point.y, 0.f, 1000.f);
                ImGui::Text("Edge &d", static_cast<int>(dcel.vertices[current].incident_edge));

                ImGui::Separator();
                ImGui::SliderFloat("Radius", &radius, 0.01f, 100.f);
                ImGui::ColorPicker4("Circle", circle_color);
            }
        }

        void show_faces(DCEL & dcel, sf::RenderWindow & window) noexcept
        {
            ImGui::Columns(2);

            static size_t current = std::numeric_limits<size_t>::max();

            show_indexed_combo(current, dcel.faces.size(), "Face");

            ImGui::NextColumn();

            if (current != std::numeric_limits<size_t>::max())
            {
                static float color[4] = { 1.f, 0.0f, 0.f, 0.7f };

                draw_face_highlighted(current, dcel, color, window);

                ImGui::Text("Edge %d", static_cast<int>(dcel.faces[current].edge));

                ImGui::Separator();
                ImGui::ColorPicker4("Face color", color);
            }
        }

        void show_edges(DCEL & dcel, sf::RenderWindow & window) noexcept
        {
            ImGui::Columns(2);

            static size_t current = std::numeric_limits<size_t>::max();

            show_indexed_combo(current, dcel.edges.size(), "Edge");

            ImGui::NextColumn();

            if (current != std::numeric_limits<size_t>::max())
            {
                static float color[4] = { 1.f, 0.0f, 0.f, 0.7f };
                static float width = 10.f;

                size_t const begin_origin = dcel.edges[current].origin_vertex;
                Point & begin_point = dcel.vertices[begin_origin].coordinate;
                size_t const end_origin = dcel.edges[dcel.edges[current].twin_edge].origin_vertex;
                Point & end_point = dcel.vertices[end_origin].coordinate;

                draw_vertex_highlighted(begin_point, color, width, window);
                draw_edge_highlighted(begin_point, end_point, color, width,  window);

                ImGui::SliderFloat("X begin", &begin_point.x, 0.f, 1000.f);
                ImGui::SliderFloat("Y begin", &begin_point.y, 0.f, 1000.f);

                ImGui::SliderFloat("X end", &end_point.x, 0.f, 1000.f);
                ImGui::SliderFloat("Y end", &end_point.y, 0.f, 1000.f);

                ImGui::Text("Face %d", static_cast<int>(dcel.edges[current].incident_face));
                ImGui::Text("Twin %d", static_cast<int>(dcel.edges[current].twin_edge));
                ImGui::Text("Next %d", static_cast<int>(dcel.edges[current].next_edge));
                ImGui::Text("Previous %d", static_cast<int>(dcel.edges[current].privous_edge));

                ImGui::Separator();
                ImGui::SliderFloat("Width", &width, 0.01f, 100.f);
                ImGui::ColorPicker4("Line", color);
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