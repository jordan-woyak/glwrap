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
	void storage(ivec2 const& _dims, int_t _samples = 0)
	{
		// ugly..
		detail::scoped_value<detail::parameter::renderbuffer> binding(native_handle());
		
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, _samples, GL_RGBA, _dims.x, _dims.y);
	}

	void resize(ivec2 const& _dims)
	{
		// ugly..
		detail::scoped_value<detail::parameter::renderbuffer> binding(native_handle());

		GLint samples{}, format{};

		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_SAMPLES, &samples);
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_INTERNAL_FORMAT, &format);

		glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, format, _dims.x, _dims.y);
	}
};

}
