#include "Application.h"
#include "triangulation.h"
#include "trapezoidal_decomposition.h"
#include "vvse.h"

#include "imgui/imgui.h"


int main()
{
    frm::dcel::DCEL dcel{};

    frm::dcel::load_from_file("Dcel_1.dat", dcel);

    frm::trapezoid_data_and_graph_root_t trapezoid_data_and_graph_root = frm::generate_trapezoid_data_and_graph_root(dcel);

    size_t current_face = trapezoid_data_and_graph_root.first;

    frm::Application application{};

    bool need_trapezoid_data = true;


    frm::vvse::VVSE vvse{};

    frm::vvse::load_from_file("Vvse_1.dat", vvse);


    application.set_on_event([&dcel, &current_face, &trapezoid_data_and_graph_root, &need_trapezoid_data](sf::Event current_event) noexcept
        {
            if (current_event.type == sf::Event::MouseButtonPressed)
            {
                int x = current_event.mouseButton.x;
                int y = current_event.mouseButton.y;

                if (need_trapezoid_data)
                {
                    current_face = frm::get_face_index(trapezoid_data_and_graph_root, { static_cast<float>(x), static_cast<float>(y) });
                }
            }
        });

    application.set_on_update([&dcel, &trapezoid_data_and_graph_root, &current_face, &need_trapezoid_data, &vvse](float dt, sf::RenderWindow & window) noexcept
        {
            frm::dcel::draw(dcel, window);
            frm::vvse::draw(vvse, window);

            bool is_dirty_dcel = false;
            bool is_dirty_vvse = false;

            is_dirty_dcel |= frm::spawn_triangulation_button(dcel);

            is_dirty_dcel |= frm::dcel::spawn_ui(dcel, window, "Dcel_1.dat");

            is_dirty_vvse |= frm::vvse::spawn_ui(vvse, window, "Vvse_1.dat");

            if (ImGui::Begin("Need trapezoid data"))
            {
                ImGui::Checkbox("need_trapezoid_data", &need_trapezoid_data);
            }
            ImGui::End();

            if (need_trapezoid_data)
            {            
                if (is_dirty_dcel)
                {
                    trapezoid_data_and_graph_root = frm::generate_trapezoid_data_and_graph_root(dcel);
                    current_face = trapezoid_data_and_graph_root.first;
                }

                if (current_face != trapezoid_data_and_graph_root.first)
                {
                    float color[4] = { 0.f, 0.f, 1.f, 0.5f };
                    frm::dcel::draw_face_highlighted(current_face, dcel, color, window);
                }
            }
        });

    application.run();
}