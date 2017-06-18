#pragma once

namespace GLWRAP_NAMESPACE
{

class context;

namespace detail
{

struct renderbuffer_obj
{
	static void create_objs(sizei_t _n, uint_t* _objs)
	{
		if (is_extension_present(GL_ARB_direct_state_access))
		{
			GLWRAP_GL_CALL(glCreateRenderbuffers)(_n, _objs);
		}
		else
		{
			GLWRAP_GL_CALL(glGenRenderbuffers)(_n, _objs);
		}
	}

	static void delete_objs(sizei_t _n, uint_t* _objs)
	{
		GLWRAP_GL_CALL(glDeleteRenderbuffers)(_n, _objs);
	}
};

}

// TODO: depth and stencil renderbuffers
template <typename DataType>
class basic_renderbuffer : public detail::globject<detail::renderbuffer_obj>
{
public:
	typedef typename image_format<DataType>::enum_type image_format_type;

	explicit basic_renderbuffer(context&)
	{}

	void storage_multisample(uint_t _samples, image_format_type _ifmt, ivec2 const& _dims)
	{
		if (is_extension_present(GL_ARB_direct_state_access))
		{
			GLWRAP_GL_CALL(glNamedRenderbufferStorageMultisample)(native_handle(), _samples, static_cast<enum_t>(_ifmt), _dims.x, _dims.y);
		}
		else
		{
			detail::scoped_value<detail::parameter::renderbuffer> binding(native_handle());

			GLWRAP_GL_CALL(glRenderbufferStorageMultisample)(GL_RENDERBUFFER, _samples, static_cast<enum_t>(_ifmt), _dims.x, _dims.y);
		}
	}

	void storage(image_format_type _ifmt, ivec2 const& _dims)
	{
		// "glRenderbufferStorage is equivalent to calling glRenderbufferStorageMultisample with the samples set to zero"
		storage_multisample(0, _ifmt, _dims);
	}

	void resize(ivec2 const& _dims)
	{

		GLint samples{}, format{};

		if (is_extension_present(GL_ARB_direct_state_access))
		{
			GLWRAP_GL_CALL(glGetNamedRenderbufferParameteriv)(native_handle(), GL_RENDERBUFFER_SAMPLES, &samples);
			GLWRAP_GL_CALL(glGetNamedRenderbufferParameteriv)(native_handle(), GL_RENDERBUFFER_INTERNAL_FORMAT, &format);

			GLWRAP_GL_CALL(glNamedRenderbufferStorageMultisample)(native_handle(), samples, format, _dims.x, _dims.y);
		}
		else
		{
			detail::scoped_value<detail::parameter::renderbuffer> binding(native_handle());

			GLWRAP_GL_CALL(glGetRenderbufferParameteriv)(GL_RENDERBUFFER, GL_RENDERBUFFER_SAMPLES, &samples);
			GLWRAP_GL_CALL(glGetRenderbufferParameteriv)(GL_RENDERBUFFER, GL_RENDERBUFFER_INTERNAL_FORMAT, &format);

			GLWRAP_GL_CALL(glRenderbufferStorageMultisample)(GL_RENDERBUFFER, samples, format, _dims.x, _dims.y);
		}
	}
};

typedef basic_renderbuffer<int_t> irenderbuffer;
typedef basic_renderbuffer<uint_t> urenderbuffer;
typedef basic_renderbuffer<float_t> renderbuffer;

}
