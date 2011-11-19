
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
	friend class program;

public:
	std::string const& get_name() const
	{
		return (*m_iter)->get_name();
	}

private:
	typedef std::list<std::unique_ptr<variable_base>>::iterator iter_t;

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

attachment texture_attachment(texture_2d& _tex, int _level)
{
	return attachment([&_tex, _level](GLenum _target, GLenum _attach_point)
	{
		glFramebufferTexture2D(_target, _attach_point, _tex.get_target(), _tex.native_handle(), _level);
	});
};

attachment renderbuffer_attachment(renderbuffer& _rendbuf)
{
	return attachment([&_rendbuf](GLenum _target, GLenum _attach_point)
	{
		glFramebufferRenderbuffer(_target, _attach_point, GL_RENDERBUFFER, _rendbuf.native_handle());
	});
};

struct read_color_buffer
{
	friend class framebuffer;
	friend class context;

private:
	read_color_buffer(GLuint _fbo, GLenum _color_buffer)
		: m_fbo(_fbo), m_color_buffer(_color_buffer)
	{}

	void bind() const
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
		glReadBuffer(m_color_buffer);
	}

	GLuint m_fbo;
	GLenum m_color_buffer;
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
		bind_read();
		_attach.m_func(GL_READ_FRAMEBUFFER, _point.get_value());
	}

	read_color_buffer read_buffer(color_attach_point const& _point)
	{
		return {native_handle(), _point.get_value()};
	}

	template <typename T>
	void set_draw_buffers(T&& _attach_points)
	{
		std::vector<GLenum> bufs;

		for (color_attach_point const& pt : _attach_points)
			bufs.push_back(pt.get_value());

		bind_draw();
		glDrawBuffers(bufs.size(), bufs.data());
	}

	// TODO: make private
	void bind_draw() const
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, native_handle());
	}

private:
	void bind_read() const
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, native_handle());
	}

/*
	void bind_both() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, native_handle());
	}
*/
};

}
