#pragma once

namespace gl
{

class context;

class renderbuffer : public globject
{
public:
	renderbuffer(context& _context)
		: globject(gen_return(glGenRenderbuffers))
	{}

	~renderbuffer()
	{
		auto const nh = native_handle();
		glDeleteRenderbuffers(1, &nh);
	}

	// TODO: internal format
	void storage(ivec2 const& _dims)
	{
		bind();
		glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, _dims.x, _dims.y);
	}

private:
	void bind() const
	{
		glBindRenderbuffer(GL_RENDERBUFFER, native_handle());
	}
};

}
