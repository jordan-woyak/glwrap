#pragma once

namespace gl
{

class context;

namespace detail
{

struct renderbuffer_obj
{
	static void create_objs(sizei_t _n, uint_t* _objs)
	{
		if (GL_ARB_direct_state_access)
		{
			GLWRAP_EC_CALL(glCreateRenderbuffers)(_n, _objs);
		}
		else
		{
			GLWRAP_EC_CALL(glGenRenderbuffers)(_n, _objs);
		}
	}

	static void delete_objs(sizei_t _n, uint_t* _objs)
	{
		GLWRAP_EC_CALL(glDeleteRenderbuffers)(_n, _objs);
	}
};

}

// TODO: depth and stencil renderbuffers

class renderbuffer : public detail::globject<detail::renderbuffer_obj>
{
public:
	explicit renderbuffer(context&)
	{}

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
