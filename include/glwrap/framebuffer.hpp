
#pragma once

#include "variable.hpp"

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
