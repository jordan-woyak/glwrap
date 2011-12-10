#pragma once

#include "../declarations.hpp"

namespace gl
{

namespace detail
{

template <texture_type T, typename Enable = void>
struct texture_dims;

template <texture_type T>
struct texture_dims<T, typename std::enable_if<
	(T == texture_type::texture_1d) ||
	(T == texture_type::texture_buffer)
	>::type>
{
	static const int value = 1;
};

template <texture_type T>
struct texture_dims<T, typename std::enable_if<
	(T == texture_type::texture_2d) ||
	(T == texture_type::texture_rectangle)
	>::type>
{
	static const int value = 2;
};

template <>
struct texture_dims<texture_type::texture_3d>
{
	static const int value = 3;
};

template <texture_type>
GLenum get_texture_target();

template <>
inline GLenum get_texture_target<texture_type::texture_1d>()
{
	return GL_TEXTURE_1D;
}

template <>
inline GLenum get_texture_target<texture_type::texture_2d>()
{
	return GL_TEXTURE_2D;
}

template <>
inline GLenum get_texture_target<texture_type::texture_rectangle>()
{
	return GL_TEXTURE_RECTANGLE;
}

template <>
inline GLenum get_texture_target<texture_type::texture_buffer>()
{
	return GL_TEXTURE_BUFFER;
}

template <>
inline GLenum get_texture_target<texture_type::texture_3d>()
{
	return GL_TEXTURE_3D;
}

}

}
