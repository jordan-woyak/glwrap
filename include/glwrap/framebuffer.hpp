
#pragma once

#include "variable.hpp"
#include "texture.hpp"
#include "renderbuffer.hpp"

namespace gl
{

class program;

struct attach_point
{
	friend class context;

public:
	GLenum get_value() const
	{
		return m_attachment;
	}

protected:
	attach_point(GLenum _attachment)
		: m_attachment(_attachment)
	{}

	GLenum m_attachment;
};

struct color_attach_point : attach_point
{
	friend class context;

private:
	color_attach_point(GLenum _attachment)
		: attach_point(_attachment)
	{}
};

// TODO: rename?
class color_number
{
	friend class context;

public:
	uint_t get_index() const
	{
		return m_index;
	}

private:
	color_number(uint_t _index)
		: m_index(_index)
	{}

	uint_t m_index;
};

template <typename T>
class fragdata
{
	friend class shader<shader_type::fragment>;

public:
	std::string const& get_name() const
	{
		return (*m_iter)->get_name();
	}

private:
	typedef std::list<std::unique_ptr<detail::variable_base>>::iterator iter_t;

	fragdata(iter_t _iter)
		: m_iter(_iter)
	{}

	iter_t m_iter;
};

class framebuffer;

class attachment
{
	friend class framebuffer;

public:
	// TODO: lame, change
	explicit attachment(std::function<void(GLenum, GLenum)> const& _func)
		: m_func(_func)
	{}

private:
	std::function<void(GLenum, GLenum)> m_func;
};

template <texture_type T>
typename std::enable_if<2 == texture<T>::dimensions, attachment>::type
texture_attachment(texture<T>& _tex, int _level)
{
	// TODO: don't capture globject reference
	return attachment([&_tex, _level](GLenum _target, GLenum _attach_point)
	{
		glFramebufferTexture2D(_target, _attach_point, detail::get_texture_target<T>(), _tex.native_handle(), _level);
	});
};

inline attachment renderbuffer_attachment(renderbuffer& _rendbuf)
{
	// TODO: don't capture globject reference
	return attachment([&_rendbuf](GLenum _target, GLenum _attach_point)
	{
		glFramebufferRenderbuffer(_target, _attach_point, GL_RENDERBUFFER, _rendbuf.native_handle());
	});
};

class framebuffer : public globject
{
	friend class context;

	framebuffer(void*)
		: globject()
	{}

public:
	framebuffer(context& _context)
		: globject(gen_return(glGenFramebuffers))
	{}

	~framebuffer()
	{
		auto const nh = native_handle();
		glDeleteFramebuffers(1, &nh);
	}

	void bind_attachment(attach_point const& _point, attachment const& _attach)
	{
		// ugly..
		detail::scoped_value<detail::parameter::read_framebuffer> binding(native_handle());
		
		_attach.m_func(GL_READ_FRAMEBUFFER, _point.get_value());
	}

/*
	void unbind_attachment(attach_point const& _point)
	{
		bind_read();
		// TODO: is this ok?
		//glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, _point.get_value(), GL_RENDERBUFFER, 0);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, _point.get_value(), GL_TEXTURE_2D, 0, 0);
	}
*/
	void bind_draw_buffer(color_number const& _number, color_attach_point const& _attach_point)
	{
		auto const index = _number.get_index();

		if (index >= m_draw_buffers.size())
			m_draw_buffers.resize(index + 1, GL_NONE);

		m_draw_buffers[index] = _attach_point.get_value();

		// TODO: don't do always
		update_draw_buffers();
	}

	void bind_read_buffer(color_attach_point const& _attach_point)
	{
		// ugly..
		detail::scoped_value<detail::parameter::read_framebuffer> binding(native_handle());
		
		glReadBuffer(_attach_point.get_value());
	}

	void unbind_draw_buffer(color_number const& _number)
	{
		auto const index = _number.get_index();

		if (index < m_draw_buffers.size())
			m_draw_buffers[index] = GL_NONE;

		// TODO: don't do always
		update_draw_buffers();
	}

private:
	void update_draw_buffers()
	{
		// ugly..
		detail::scoped_value<detail::parameter::draw_framebuffer> binding(native_handle());
		
		glDrawBuffers(m_draw_buffers.size(), m_draw_buffers.data());
	}

	// TODO: can this be killed?
	std::vector<GLenum> m_draw_buffers;
};

class framebuffer_reference
{
	friend class context;

public:
	framebuffer_reference(framebuffer const& _fb)
		: m_fb(_fb.native_handle())
	{}

	framebuffer_reference(std::nullptr_t)
		: m_fb(0)
	{}

	GLuint native_handle() const
	{
		return m_fb;
	}

private:
	void bind_draw()
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fb);
	}

	GLuint m_fb;
};

}
