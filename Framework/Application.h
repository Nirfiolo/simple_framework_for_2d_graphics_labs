#pragma once


#include "SFML\Graphics.hpp"

#include <functional>


// Framework
namespace frm
{
    class Application
    {
    public:
        static constexpr char const * default_title{ "Application" };
        static constexpr size_t default_width{ 1280 };
        static constexpr size_t default_height{ 720 };

    public:
        Application(
            std::string title = default_title,
            size_t width = default_width,
            size_t height = default_height
        ) noexcept;

        Application(Application const &) = delete;
        Application(Application &&) = delete;
        Application & operator=(Application const &) = delete;
        Application & operator=(Application &&) = delete;

        ~Application();

        sf::RenderWindow & get_window() noexcept;

        void set_on_update(std::function<void(float, sf::RenderWindow &)> const & on_update) noexcept;
        void set_on_event(std::function<void(sf::Event)> const & on_event) noexcept;

        void run() noexcept;
        
    private:
        std::function<void(float, sf::RenderWindow &)> m_on_update{ [](float, sf::RenderWindow &) {} };
        std::function<void(sf::Event)> m_on_event{ [](sf::Event) {} };

        sf::RenderWindow m_window;
        sf::Clock m_clock{};

        std::string m_title;

        size_t m_width;
        size_t m_height;

        float m_speed_factor{ 1.f };

    private:
        sf::Color const background_color{ sf::Color::Black };
    };
}