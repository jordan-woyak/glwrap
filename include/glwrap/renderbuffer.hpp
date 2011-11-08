#pragma once

namespace gl
{

class context;

class renderbuffer : public native_handle_base<GLuint>
{
public:
	renderbuffer(context& _context)
		: native_handle_base<GLuint>(gen_return(glGenRenderbuffers))
	{}

	~renderbuffer()
	{
		auto const nh = native_handle();
		glDeleteRenderbuffers(1, &nh);
	}
};

}
