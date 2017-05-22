
#pragma once

namespace GLWRAP_NAMESPACE
{

namespace detail
{



//
// gl_get_tex_parameter
//
// provides a single name for glGetTexParameter* functions
// the first template parameter specifies internal data format is int
// (i.e. glGetTexParameterI*)
// Relevant only for ints (e.g. GL_TEXTURE_BORDER_COLOR)
// the second template parameter is the input type
//

template <bool Internal, typename T>
GLWRAP_FUNC_DECL
void gl_get_tex_parameter(GLenum _target, GLenum _pname, T* _params);

template <>
GLWRAP_FUNC_DECL
void gl_get_tex_parameter<false, GLfloat>(GLenum _target, GLenum _pname, GLfloat* _params)
{
	GLWRAP_EC_CALL(glGetTexParameterfv)(_target, _pname, _params);
}

template <>
GLWRAP_FUNC_DECL
void gl_get_tex_parameter<false, GLint>(GLenum _target, GLenum _pname, GLint* _params)
{
	GLWRAP_EC_CALL(glGetTexParameteriv)(_target, _pname, _params);
}

template <>
GLWRAP_FUNC_DECL
void gl_get_tex_parameter<true, GLint>(GLenum _target, GLenum _pname, GLint* _params)
{
	GLWRAP_EC_CALL(glGetTexParameterIiv)(_target, _pname, _params);
}

template <>
GLWRAP_FUNC_DECL
void gl_get_tex_parameter<true, GLuint>(GLenum _target, GLenum _pname, GLuint* _params)
{
	GLWRAP_EC_CALL(glGetTexParameterIuiv)(_target, _pname, _params);
}

//
// get_tex_parameter
//
// provides a friendly way to glGetTexParameter directly into vecNs
//

template <bool I, typename T>
GLWRAP_FUNC_DECL
typename std::enable_if<std::is_scalar<T>::value, T>::type
get_tex_parameter(GLenum _target, GLenum _pname)
{
	T ret;

	gl_get_tex_parameter<I>(_target, _pname, &ret);

	return ret;
}

template <bool I, typename T>
GLWRAP_FUNC_DECL
typename std::enable_if<detail::is_vec<T>::value, T>::type
get_tex_parameter(GLenum _target, GLenum _pname)
{
	return get_tex_parameter<I>(_target, _pname, value_ptr(ret));
}

//
// gl_tex_parameter
//
// provides a single name for glTexParameter* functions
// the first template parameter specifies internal data format is int
// (i.e. glSetTexParameterI*)
// Relevant only for ints (e.g. GL_TEXTURE_BORDER_COLOR)
// the second template parameter is the input type
//

template <bool Internal, typename T>
GLWRAP_FUNC_DECL
void gl_tex_parameter(GLenum _target, GLenum _pname, T _param);

template <>
GLWRAP_FUNC_DECL
void gl_tex_parameter<false, GLfloat>(GLenum _target, GLenum _pname, GLfloat _param)
{
	GLWRAP_EC_CALL(glTexParameterf)(_target, _pname, _param);
}

template <>
GLWRAP_FUNC_DECL
void gl_tex_parameter<false, const GLfloat*>(GLenum _target, GLenum _pname, const GLfloat* _params)
{
	GLWRAP_EC_CALL(glTexParameterfv)(_target, _pname, _params);
}

template <>
GLWRAP_FUNC_DECL
void gl_tex_parameter<false, GLint>(GLenum _target, GLenum _pname, GLint _param)
{
	GLWRAP_EC_CALL(glTexParameteri)(_target, _pname, _param);
}

template <>
GLWRAP_FUNC_DECL
void gl_tex_parameter<false, const GLint*>(GLenum _target, GLenum _pname, const GLint* _params)
{
	GLWRAP_EC_CALL(glTexParameteriv)(_target, _pname, _params);
}

template <>
GLWRAP_FUNC_DECL
void gl_tex_parameter<true, const GLint*>(GLenum _target, GLenum _pname, const GLint* _params)
{
	GLWRAP_EC_CALL(glTexParameterIiv)(_target, _pname, _params);
}

template <>
GLWRAP_FUNC_DECL
void gl_tex_parameter<true, const GLuint*>(GLenum _target, GLenum _pname, const GLuint* _params)
{
	GLWRAP_EC_CALL(glTexParameterIuiv)(_target, _pname, _params);
}

//
// set_tex_parameter
//
// provides a friendly way to glTexParameter directly from vecNs
//

template <bool I, typename T>
GLWRAP_FUNC_DECL
typename std::enable_if<std::is_scalar<T>::value>::type
set_tex_parameter(GLenum _target, GLenum _pname, T _param)
{
	gl_tex_parameter<I>(_target, _pname, _param);
}

template <bool I, typename T>
GLWRAP_FUNC_DECL
typename std::enable_if<detail::is_vec<T>::value>::type
set_tex_parameter(GLenum _target, GLenum _pname, const T& _param)
{
	set_tex_parameter<I>(_target, _pname, value_ptr(_param));
}

//
// tex_parameter
//
// provides typed get and set functions for a particular parameter
//



}

}
