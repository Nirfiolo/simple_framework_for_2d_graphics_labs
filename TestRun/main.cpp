#include "Application.h"

#include "vvve.h"
#include "data_convert.h"

#include "triangulation.h"
#include "trapezoidal_decomposition.h"
#include "quick_hull.h"
#include "common_ui_part.h"
#include "nearest_point.h"
#include "nearest_line.h"
#include "apply_hull.h"


#include "imgui/imgui.h"


int main()
{
    frm::dcel::DCEL dcel{};

    frm::dcel::load_from_file("Dcel_1.dat", dcel);

    frm::trapezoid_data_and_graph_root_t trapezoid_data_and_graph_root = frm::generate_trapezoid_data_and_graph_root(dcel);

    size_t current_vertex = 0;
    size_t current_edge = 0;
    size_t current_face = trapezoid_data_and_graph_root.first;

    frm::Application application{};

    bool need_trapezoid_data = false;
    bool is_dirty = true;

    application.set_on_event([&dcel,
        &current_vertex,
        &current_edge,
        &current_face,
        &trapezoid_data_and_graph_root,
        &need_trapezoid_data,
        &is_dirty
    ](sf::Event current_event) noexcept
        {
            if (current_event.type == sf::Event::MouseButtonPressed &&
                current_event.mouseButton.button == sf::Mouse::Right)
            {
                int x = current_event.mouseButton.x;
                int y = current_event.mouseButton.y;

                frm::Point point{ static_cast<float>(x), static_cast<float>(y) };

                if (need_trapezoid_data)
                {
                    current_face = frm::get_face_index(trapezoid_data_and_graph_root, point);
                }

                current_vertex = frm::nearest_point(dcel, point);
                current_edge = frm::nearest_line(dcel, point);

                is_dirty = true;
            }
        });

    application.set_on_update([&dcel,
        &trapezoid_data_and_graph_root,
        &current_vertex,
        &current_edge,
        &current_face,
        &need_trapezoid_data,
        &is_dirty
    ](float dt, sf::RenderWindow & window) noexcept
        {
            frm::dcel::draw(dcel, window);

            bool is_dirty_trapezoid = false;

            static bool is_active = false;
            if (ImGui::Begin("Triangulation", &is_active))
            {
                if (ImGui::Button("Triangulate"))
                {
                    frm::vvve::VVVE vvve = frm::dcel_to_vvve(dcel);
                    frm::quick_hull(vvve);

                    frm::apply_hull(dcel, vvve);

                    frm::triangulation(dcel);
                    is_dirty_trapezoid = true;
                }
            }
            ImGui::End();

            is_dirty_trapezoid |= frm::dcel::spawn_ui(dcel, current_vertex, current_edge, current_face,  window, "Dcel_1.dat", is_dirty);

            if (ImGui::Begin("Need trapezoid data"))
            {
                ImGui::Checkbox("need_trapezoid_data", &need_trapezoid_data);
            }
            ImGui::End();

            if (need_trapezoid_data)
            {
                if (is_dirty_trapezoid)
                {
                    trapezoid_data_and_graph_root = frm::generate_trapezoid_data_and_graph_root(dcel);
                    current_face = frm::dcel::get_outside_face_index(dcel);
                }

                float color[4] = { 0.f, 0.f, 1.f, 0.5f };
                float radius = 10.f;
                if (frm::dcel::is_vertices_mode())
                {
                    frm::draw_vertex_highlighted(dcel.vertices[current_vertex].coordinate, color, radius, window);
                }
                if (frm::dcel::is_edges_mode())
                {
                    frm::Point begin_point = dcel.vertices[dcel.edges[current_edge].origin_vertex].coordinate;
                    frm::Point end_point = dcel.vertices[dcel.edges[dcel.edges[current_edge].twin_edge].origin_vertex].coordinate;

                    frm::draw_edge_highlighted(begin_point, end_point, color, radius, window);
                }
                if (frm::dcel::is_faces_mode())
                {
                    if (current_face != frm::dcel::get_outside_face_index(dcel))
                    {
                        frm::dcel::draw_face_highlighted(current_face, dcel, color, window);
                    }
                }
            }
        });

    application.run();
}