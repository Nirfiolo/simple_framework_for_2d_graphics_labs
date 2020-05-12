#include "Application.h"
#include "triangulation.h"


int main()
{
    frm::dcel::DCEL dcel{};

    frm::dcel::load_from_file("Dcel_1.dat", dcel);

    frm::Application application{};

    application.set_on_update([&dcel](float dt, sf::RenderWindow & window) noexcept
        {
            frm::dcel::draw(dcel, window);

            frm::dcel::spawn_ui(dcel, window, "Dcel_1.dat");

            frm::spawn_triangulation_button(dcel);
        });

    application.run();
}