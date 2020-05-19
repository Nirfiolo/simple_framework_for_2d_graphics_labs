#include "common_ui_part.h"

#include "imgui/imgui.h"


namespace frm
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

}