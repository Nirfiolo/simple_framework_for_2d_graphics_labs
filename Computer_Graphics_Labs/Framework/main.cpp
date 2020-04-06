#include "SFML/Graphics.hpp"

int main(int argc, char ** argv)
{
    sf::RenderWindow render_window(sf::VideoMode(640, 480), "Hello World");

    while (render_window.isOpen())
    {
        sf::Event event;

        while (render_window.pollEvent(event))
        {
            if (event.type == sf::Event::EventType::Closed)
            {
                render_window.close();
            }
        }
        render_window.clear(sf::Color::White);
        render_window.display();
    }
}