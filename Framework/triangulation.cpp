#include "triangulation.h"

#include "imgui/imgui.h"


namespace frm
{
    void triangulation_y_monotone(dcel::DCEL & dcel) noexcept
    {

    }


    void triangulation(dcel::DCEL & dcel) noexcept
    {
        triangulation_y_monotone(dcel);
    }

    void spawn_triangulation_button(dcel::DCEL & dcel) noexcept
    {
        static bool is_active = false;
        if (ImGui::Begin("Triangulation", &is_active))
        {
            if (ImGui::Button("Triangulate"))
            {
                triangulation(dcel);
            }
        }
        ImGui::End();
    }
}