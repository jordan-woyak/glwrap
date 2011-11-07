
#pragma once

#include <functional>
#include <memory>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

//#include <GL/glut.h>
#include <GL/glew.h>
#include <GL/gl.h>

namespace gl
{

class display
{
public:
	display(context& _context)
		: m_sf_window(new sf::RenderWindow(sf::VideoMode(640, 480, 32), "Testing"))
	{
		m_sf_window->Show(true);
		glewInit();
	}

	void swap()
	{
		m_sf_window->Display();
	}

	void set_display_func(const std::function<void()>& _func)
	{
		m_display_func = _func;
	}

	void run_loop()
	{
		while (m_sf_window->IsOpened())
		{
			sf::Event ev;
			while (m_sf_window->PollEvent(ev))
			{
				if (ev.Closed == ev.Type)
					m_sf_window->Close();
			}

			m_sf_window->SetActive();
			m_display_func();
			swap();

			sf::Sleep(1000 / 60);
		}
	}

private:
	std::function<void()> m_display_func;

	std::unique_ptr<sf::RenderWindow> m_sf_window;
};

}

