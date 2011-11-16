
#pragma once

#include "variable.hpp"
#include "texture.hpp"
#include "renderbuffer.hpp"

namespace gl
{

class program;

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

class pixel_block
{
	friend class context;

public:
	pixel_block(framebuffer& _fb, basic_vec<int_t, 2> _lower, basic_vec<int_t, 2> _upper)
		: m_fb(_fb.native_handle()), m_lower(_lower), m_upper(_upper)
	{}

private:
	void bind(GLenum _target) const
	{
		glBindFramebuffer(_target, m_fb);
	}

	framebuffer::native_handle_type m_fb;
	basic_vec<int_t, 2> m_lower, m_upper;
};

}
