
#pragma once

namespace __GLWRAP_NAMESPACE__
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
__GLWRAP_FUNC_DECL__
void gl_get_tex_parameter(GLenum _target, GLenum _pname, T* _params);

template <>
__GLWRAP_FUNC_DECL__
void gl_get_tex_parameter<false, GLfloat>(GLenum _target, GLenum _pname, GLfloat* _params)
{
	glGetTexParameterfv(_target, _pname, _params);
}

template <>
__GLWRAP_FUNC_DECL__
void gl_get_tex_parameter<false, GLint>(GLenum _target, GLenum _pname, GLint* _params)
{
	glGetTexParameteriv(_target, _pname, _params);
}

template <>
__GLWRAP_FUNC_DECL__
void gl_get_tex_parameter<true, GLint>(GLenum _target, GLenum _pname, GLint* _params)
{
	glGetTexParameterIiv(_target, _pname, _params);
}

template <>
__GLWRAP_FUNC_DECL__
void gl_get_tex_parameter<true, GLuint>(GLenum _target, GLenum _pname, GLuint* _params)
{
	glGetTexParameterIuiv(_target, _pname, _params);
}

//
// get_tex_parameter
//
// provides a friendly way to glGetTexParameter directly into vecNs
//

template <bool I, typename T>
__GLWRAP_FUNC_DECL__
typename std::enable_if<std::is_arithmetic<T>::value, T>::type
get_tex_parameter(GLenum _target, GLenum _pname)
{
		T ret;

		gl_get_tex_parameter<I>(_target, _pname, &ret);
		check_unlikely_error();

		return ret;
}

template <bool I, typename T>
__GLWRAP_FUNC_DECL__
typename std::enable_if<detail::is_vec<T>::value, T>::type
get_tex_parameter(GLenum _target, GLenum _pname)
{
		T ret;

		gl_get_tex_parameter<I>(_target, _pname, glm::value_ptr(ret));
		check_unlikely_error();

		return ret;
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
__GLWRAP_FUNC_DECL__
void gl_tex_parameter(GLenum _target, GLenum _pname, T _param);

template <>
__GLWRAP_FUNC_DECL__
void gl_tex_parameter<false, GLfloat>(GLenum _target, GLenum _pname, GLfloat _param)
{
	glTexParameterf(_target, _pname, _param);
}

template <>
__GLWRAP_FUNC_DECL__
void gl_tex_parameter<false, const GLfloat*>(GLenum _target, GLenum _pname, const GLfloat* _params)
{
	glTexParameterfv(_target, _pname, _params);
}

template <>
__GLWRAP_FUNC_DECL__
void gl_tex_parameter<false, GLint>(GLenum _target, GLenum _pname, GLint _param)
{
	glTexParameteri(_target, _pname, _param);
}

template <>
__GLWRAP_FUNC_DECL__
void gl_tex_parameter<false, const GLint*>(GLenum _target, GLenum _pname, const GLint* _params)
{
	glTexParameteriv(_target, _pname, _params);
}

template <>
__GLWRAP_FUNC_DECL__
void gl_tex_parameter<true, const GLint*>(GLenum _target, GLenum _pname, const GLint* _params)
{
	glTexParameterIiv(_target, _pname, _params);
}

template <>
__GLWRAP_FUNC_DECL__
void gl_tex_parameter<true, const GLuint*>(GLenum _target, GLenum _pname, const GLuint* _params)
{
	glTexParameterIuiv(_target, _pname, _params);
}

//
// tex_parameter
//
// provides a friendly way to glTexParameter directly from vecNs
//

template <bool I, typename T>
__GLWRAP_FUNC_DECL__
typename std::enable_if<std::is_arithmetic<T>::value>::type
tex_parameter(GLenum _target, GLenum _pname, T _param)
{
	gl_tex_parameter<I>(_target, _pname, _param);
	check_unlikely_error();
};

template <bool I, typename T>
__GLWRAP_FUNC_DECL__
typename std::enable_if<detail::is_vec<T>::value>::type
tex_parameter(GLenum _target, GLenum _pname, const T& _param)
{
	gl_tex_parameter<I>(_target, _pname, glm::value_ptr(_param));
	check_unlikely_error();
};

}

}
