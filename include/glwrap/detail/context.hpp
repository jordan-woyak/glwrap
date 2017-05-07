
#pragma once

namespace GLWRAP_NAMESPACE
{

namespace detail
{

//
// gl_get
//
// provides a single name for glGet* functions
//

void gl_get(GLenum _pname, GLboolean* _params)
{
	glGetBooleanv(_pname, _params);
}

void gl_get(GLenum _pname, GLdouble* _params)
{
	glGetDoublev(_pname, _params);
}

void gl_get(GLenum _pname, GLfloat* _params)
{
	glGetFloatv(_pname, _params);
}

void gl_get(GLenum _pname, GLint* _params)
{
	glGetIntegerv(_pname, _params);
}

void gl_get(GLenum _pname, GLint64* _params)
{
	glGetInteger64v(_pname, _params);
}

//
// get_parameter
//
// a slightly more friendly interface for gl_get
//

template <typename T>
GLWRAP_FUNC_DECL
T get_parameter(enum_t _pname)
{
	T ret;

	gl_get(_pname, value_ptr(ret));
	check_unlikely_error();

	return ret;
}

template <typename T, enum_t P>
struct regular_parameter_getter
{
	static GLWRAP_MEMBER_FUNC_DECL
	T get()
	{
		return get_parameter<T>(P);
	}
};

namespace parameter
{

struct clear_color {};
struct clear_depth {};
struct clear_stencil {};
struct blend_color {};

// TODO: named with _binding ?
struct draw_framebuffer : regular_parameter_getter<int_t, GL_DRAW_FRAMEBUFFER_BINDING>
{
	typedef uint_t value_type;
	
	static GLWRAP_MEMBER_FUNC_DECL
	void set(value_type _value)
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _value);
	}
};

struct read_framebuffer : regular_parameter_getter<int_t, GL_READ_FRAMEBUFFER_BINDING>
{
	typedef uint_t value_type;
	
	static GLWRAP_MEMBER_FUNC_DECL
	void set(value_type _value)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, _value);
	}
};

struct vertex_array : regular_parameter_getter<int_t, GL_VERTEX_ARRAY_BINDING>
{
	typedef uint_t value_type;
	
	static GLWRAP_MEMBER_FUNC_DECL
	void set(value_type _value)
	{
		glBindVertexArray(_value);
	}
};

struct program : regular_parameter_getter<int_t, GL_CURRENT_PROGRAM>
{
	typedef uint_t value_type;
	
	static GLWRAP_MEMBER_FUNC_DECL
	void set(value_type _value)
	{
		glUseProgram(_value);
	}
};

}

template <typename P>
class parameter_ref;

template <>
class parameter_ref<parameter::blend_color>
{
public:
	typedef glm::vec4 value_type;

	GLWRAP_MEMBER_FUNC_DECL
	void
	set(value_type _val)
	{
		glBlendColor(_val.r, _val.g, _val.b, _val.a);
		check_unlikely_error();
	}

	GLWRAP_MEMBER_FUNC_DECL
	value_type
	get() const
	{
		return get_parameter<value_type>(GL_BLEND_COLOR);
	}
};

template <>
class parameter_ref<parameter::clear_color>
{
public:
	typedef glm::vec4 value_type;

	GLWRAP_MEMBER_FUNC_DECL
	void
	set(const value_type& _val)
	{
		glClearColor(_val.r, _val.g, _val.b, _val.a);
		check_unlikely_error();
	}

	GLWRAP_MEMBER_FUNC_DECL
	value_type
	get() const
	{
		return get_parameter<value_type>(GL_COLOR_CLEAR_VALUE);
	}
};

template <>
class parameter_ref<parameter::clear_depth>
{
public:
	typedef glm::float_t value_type;

	GLWRAP_MEMBER_FUNC_DECL
	void
	set(value_type _val)
	{
		glClearDepthf(_val);
		check_unlikely_error();
	}

	GLWRAP_MEMBER_FUNC_DECL
	value_type
	get() const
	{
		return get_parameter<value_type>(GL_DEPTH_CLEAR_VALUE);
	}
};

template <>
class parameter_ref<parameter::clear_stencil>
{
public:
	typedef glm::int_t value_type;

	GLWRAP_MEMBER_FUNC_DECL
	void
	set(value_type _val)
	{
		glClearStencil(_val);
		check_unlikely_error();
	}

	GLWRAP_MEMBER_FUNC_DECL
	value_type
	get() const
	{
		return get_parameter<value_type>(GL_STENCIL_CLEAR_VALUE);
	}
};

}

/*
class context
{
public:
	GLWRAP_MEMBER_FUNC_DECL
	detail::parameter_ref<detail::parameter::blend_color>
	blend_color()
	{
		return detail::parameter_ref<detail::parameter::blend_color>();
	}

	GLWRAP_MEMBER_FUNC_DECL
	detail::parameter_ref<detail::parameter::clear_color>
	clear_color()
	{
		return detail::parameter_ref<detail::parameter::clear_color>();
	}

	GLWRAP_MEMBER_FUNC_DECL
	detail::parameter_ref<detail::parameter::clear_depth>
	clear_depth()
	{
		return detail::parameter_ref<detail::parameter::clear_depth>();
	}

	GLWRAP_MEMBER_FUNC_DECL
	detail::parameter_ref<detail::parameter::clear_stencil>
	clear_stencil()
	{
		return detail::parameter_ref<detail::parameter::clear_stencil>();
	}

	template <typename T>
	GLWRAP_MEMBER_FUNC_DECL
	detail::parameter_ref<T>
	parameter()
	{
		return detail::parameter_ref<T>();
	}

private:

};
*/
}
