#include "Application.h"
#include "dcel.h"


int main()
{
    frm::dcel::DCEL dcel{};

    dcel.vertices.push_back({ {0.f, 400.f}, 0 });
    dcel.vertices.push_back({ {200.f, 400.f}, 7 });
    dcel.vertices.push_back({ {200.f, 200.f}, 2 });
    dcel.vertices.push_back({ {100.f, 100.f}, 3 });

    dcel.faces.push_back({ 3 });
    dcel.faces.push_back({ 1 });

    dcel.edges.push_back({ 0, 1, 0, 7, 4 });
    dcel.edges.push_back({ 1, 0, 1, 5, 6 });
    dcel.edges.push_back({ 2, 3, 0, 3, 7 });
    dcel.edges.push_back({ 3, 2, 0, 4, 2 });
    dcel.edges.push_back({ 2, 5, 0, 0, 3 });
    dcel.edges.push_back({ 0, 4, 1, 6, 1 });
    dcel.edges.push_back({ 2, 7, 1, 1, 5 });
    dcel.edges.push_back({ 1, 6, 0, 2, 0 });

    frm::Application application{};

    application.set_on_update([&dcel](float dt, sf::RenderWindow & window) noexcept
        {
            frm::dcel::draw(dcel, window);

            frm::dcel::spawn_ui(dcel, window, "Dcel_1.dat");
        });

    application.run();
}