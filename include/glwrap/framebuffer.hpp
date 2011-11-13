
#pragma once

#include "variable.hpp"
#include "texture.hpp"
#include "renderbuffer.hpp"

namespace gl
{

class program;

template <typename T>
class fragdata
{
	friend class program;

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
	// TODO: kinda lame
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

class framebuffer : public native_handle_base<GLuint>
{
public:
	framebuffer(context& _context)
		: native_handle_base<GLuint>(gen_return(glGenFramebuffers))
	{}

	~framebuffer()
	{
		auto const nh = native_handle();
		glDeleteFramebuffers(1, &nh);
	}

	void attach_draw(attachment const& _attach)
	{
		bind_draw();
		// TODO: attachment point
		_attach.m_func(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0);
	}

	void attach_read(attachment const& _attach)
	{
		bind_read();
		// TODO: attachment point
		_attach.m_func(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0);
	}

private:
	void bind_read() const
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, native_handle());
	}

	void bind_draw() const
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, native_handle());
	}

	void bind_both() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, native_handle());
	}
};

}
