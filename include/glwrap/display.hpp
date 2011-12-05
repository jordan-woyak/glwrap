
#pragma once

#include <functional>
#include <memory>

#include <SFML/Graphics.hpp>

#include "context.hpp"
#include "vector.hpp"

namespace gl
{

class display
{
public:
	display(context& _context, const ivec2& _size)
		: m_sf_window(_context.get_window())
	{
		m_sf_window.Create({_size.x, _size.y}, "");
	}

	display(display const&) = delete;
	display& operator=(display const&) = delete;

	void set_size(const ivec2& _size)
	{
		m_sf_window.SetSize(_size.x, _size.y);
	}

	void set_caption(std::string const& _caption)
	{
		m_sf_window.SetTitle(_caption);
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
		//m_sf_window.Show(true);
		while (m_sf_window.IsOpened())
		{
			sf::Event ev;
			while (m_sf_window.PollEvent(ev))
			{
				if (ev.Resized == ev.Type)
					m_resize_func({ev.Size.Width, ev.Size.Height});
				else if (ev.Closed == ev.Type)
					m_sf_window.Close();
			}

			m_sf_window.SetActive();
			m_display_func();
			swap();

			sf::Sleep(1000 / 60);
		}
	}

private:
	void swap()
	{
		m_sf_window.Display();
	}

	std::function<void()> m_display_func;
	std::function<void(ivec2)> m_resize_func;

	sf::RenderWindow& m_sf_window;
};

}

