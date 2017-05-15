#pragma once

namespace gl
{

class context;

// TODO: depth and stencil renderbuffers

class renderbuffer : public globject
{
public:
	renderbuffer(context& _context)
		: globject(detail::gen_return(glGenRenderbuffers))
	{
		// TODO: this is ugly, actually create the object
		detail::scoped_value<detail::parameter::renderbuffer> binding(native_handle());
	}

	~renderbuffer()
	{
		auto const nh = native_handle();
		GLWRAP_EC_CALL(glDeleteRenderbuffers)(1, &nh);
	}

	// TODO: internal format
	void storage(ivec2 const& _dims, sizei_t _samples = 0)
	{
		if (GL_ARB_direct_state_access)
		{
			GLWRAP_EC_CALL(glNamedRenderbufferStorageMultisample)(native_handle(), _samples, GL_RGBA, _dims.x, _dims.y);
		}
		else
		{
			detail::scoped_value<detail::parameter::renderbuffer> binding(native_handle());
			
			GLWRAP_EC_CALL(glRenderbufferStorageMultisample)(GL_RENDERBUFFER, _samples, GL_RGBA, _dims.x, _dims.y);
		}
	}

	void resize(ivec2 const& _dims)
	{

		GLint samples{}, format{};
		
		if (GL_ARB_direct_state_access)
		{
			GLWRAP_EC_CALL(glGetNamedRenderbufferParameteriv)(native_handle(), GL_RENDERBUFFER_SAMPLES, &samples);
			GLWRAP_EC_CALL(glGetNamedRenderbufferParameteriv)(native_handle(), GL_RENDERBUFFER_INTERNAL_FORMAT, &format);

			GLWRAP_EC_CALL(glNamedRenderbufferStorageMultisample)(native_handle(), samples, format, _dims.x, _dims.y);
		}
		else
		{
			detail::scoped_value<detail::parameter::renderbuffer> binding(native_handle());

			GLWRAP_EC_CALL(glGetRenderbufferParameteriv)(GL_RENDERBUFFER, GL_RENDERBUFFER_SAMPLES, &samples);
			GLWRAP_EC_CALL(glGetRenderbufferParameteriv)(GL_RENDERBUFFER, GL_RENDERBUFFER_INTERNAL_FORMAT, &format);

			GLWRAP_EC_CALL(glRenderbufferStorageMultisample)(GL_RENDERBUFFER, samples, format, _dims.x, _dims.y);
		}
	}
};

}
