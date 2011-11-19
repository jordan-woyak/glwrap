
#pragma once

#include <functional>
#include <memory>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

//#include <GL/glut.h>
#include <GL/glew.h>
#include <GL/gl.h>

#include "vector.hpp"

namespace gl
{

class display
{
public:
	display(context& _context, ivec2 const& _size)
		: m_sf_window(new sf::RenderWindow(sf::VideoMode(_size.x, _size.y, 32), ""))
	{
		glewInit();
	}

	void set_caption(std::string const& _caption)
	{
		m_sf_window->SetTitle(_caption);
	}

	void set_display_func(const std::function<void()>& _func)
	{
		m_display_func = _func;
	}

	void set_resize_func(const std::function<void(ivec2)>& _func)
	{
		m_resize_func = _func;
	}

	void run_loop()
	{
		m_sf_window->Show(true);
		while (m_sf_window->IsOpened())
		{
			sf::Event ev;
			while (m_sf_window->PollEvent(ev))
			{
				if (ev.Resized == ev.Type)
					m_resize_func({ev.Size.Width, ev.Size.Height});
				else if (ev.Closed == ev.Type)
					m_sf_window->Close();
			}

			m_sf_window->SetActive();
			m_display_func();
			swap();

			sf::Sleep(1000 / 60);
		}
	}

private:
	void swap()
	{
		m_sf_window->Display();
	}

	std::function<void()> m_display_func;
	std::function<void(ivec2)> m_resize_func;

	std::unique_ptr<sf::RenderWindow> m_sf_window;
};

}

