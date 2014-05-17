
#pragma once

#include <functional>
#include <memory>

//#define USE_SFML

#ifdef USE_SFML
#include <SFML/Graphics.hpp>
#endif

#include "context.hpp"
#include "vector.hpp"

namespace gl
{

class display
{
public:
#ifdef USE_SFML
	display(context& _context, const ivec2& _size)
		: m_sf_window(_context.get_window())
	{
		m_sf_window.Create({_size.x, _size.y}, "");
	}
#else
	display(context& _context, const ivec2& _size)
	{
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
		glutInitWindowSize(_size.x, _size.y);
		glutCreateWindow("");

		glewInit();

		// gross
		glutReshapeFunc(&display::Reshape);
		glutDisplayFunc(&display::Display);
	}
#endif

	display(display const&) = delete;
	display& operator=(display const&) = delete;

	void set_size(const ivec2& _size)
	{
#ifdef USE_SFML
		m_sf_window.SetSize(_size.x, _size.y);
#else
		glutReshapeWindow(_size.x, _size.y);
#endif
	}

	void set_caption(std::string const& _caption)
	{
#ifdef USE_SFML
		m_sf_window.SetTitle(_caption);
#else
		glutSetWindowTitle(_caption.c_str());
#endif
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
#ifdef USE_SFML
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
#else
		glutMainLoop();
#endif
	}

private:
	void swap()
	{
#ifdef USE_SFML
		m_sf_window.Display();
#else
		glutSwapBuffers();
#endif
	}
#if USE_SFML
	std::function<void()> m_display_func;
	std::function<void(ivec2)> m_resize_func;

	sf::RenderWindow& m_sf_window;
#else
	// gross
	static std::function<void()> m_display_func;
	static std::function<void(ivec2)> m_resize_func;

	static void Display()
	{
		m_display_func();
		glutSwapBuffers();
		glutPostRedisplay();
	}

	static void Reshape(GLint _w, GLint _h)
	{
		m_resize_func({_w, _h});
	}
#endif
};

// gross
std::function<void()> display::m_display_func;
std::function<void(ivec2)> display::m_resize_func;

}

