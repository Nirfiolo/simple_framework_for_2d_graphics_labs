#include "Application.h"

#include "imgui/imgui-SFML.h"
#include "imgui/imgui.h"


namespace frm
{
    Application::Application(
        std::string title,
        size_t width,
        size_t height
    ) noexcept
        : m_window{ sf::VideoMode{ static_cast<unsigned int>(width), static_cast<unsigned int>(height) }, "" },
        m_title{ title },
        m_width{ width },
        m_height{ height }
    {
        ImGui::SFML::Init(m_window);

        m_window.setTitle(m_title);
    }

    Application::~Application()
    {
        ImGui::SFML::Shutdown();
    }

    sf::RenderWindow & Application::get_window() noexcept
    {
        return m_window;
    }

    void Application::set_on_update(std::function<void(float, sf::RenderWindow &)> const & on_update) noexcept
    {
        m_on_update = on_update;
    }

    void Application::set_on_event(std::function<void(sf::Event)> const & on_event) noexcept
    {
        m_on_event = on_event;
    }

    void Application::run() noexcept
    {
        m_window.resetGLStates();
        while (m_window.isOpen())
        {
            sf::Event event;
            while (m_window.pollEvent(event))
            {
                ImGui::SFML::ProcessEvent(event);

                m_on_event(event);

                if (event.type == sf::Event::Closed)
                {
                    m_window.close();
                }
            }

            sf::Time const time = m_clock.restart();

            float const dt = static_cast<float>(time.asMicroseconds()) / 1000.f;

            ImGui::SFML::Update(m_window, time);


            if (ImGui::Begin("Simulation speed"))
            {
                ImGui::SliderFloat("Speed Factor", &m_speed_factor, -2.f, 2.f, "%.3f", 4.f);
                ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            }
            ImGui::End();


            m_window.clear(background_color);


            m_on_update(dt, m_window);


            ImGui::SFML::Render(m_window);

            m_window.display();
        }
    }
}