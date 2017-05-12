#pragma once

#include "../declarations.hpp"

namespace gl
{

namespace detail
{

void gl_tex_parameter(enum_t _target, enum_t _pname, float_t _value)
{
	glTexParameterf(_target, _pname, _value);
}

void gl_tex_parameter(enum_t _target, enum_t _pname, int_t _value)
{
	glTexParameteri(_target, _pname, _value);
}

void gl_texture_parameter(uint_t _texture, enum_t _pname, float_t _value)
{
	glTextureParameterf(_texture, _pname, _value);
}

void gl_texture_parameter(uint_t _texture, enum_t _pname, int_t _value)
{
	glTextureParameteri(_texture, _pname, _value);
}

// TODO: work dims into the rest of the traits
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

template <texture_type T>
struct texture_traits
{};

template <>
struct texture_traits<texture_type::texture_1d>
{
	static const enum_t target = GL_TEXTURE_1D;
	static const enum_t binding = GL_TEXTURE_BINDING_1D;
};

template <>
struct texture_traits<texture_type::texture_2d>
{
	static const enum_t target = GL_TEXTURE_2D;
	static const enum_t binding = GL_TEXTURE_BINDING_2D;
};

template <>
struct texture_traits<texture_type::texture_3d>
{
	static const enum_t target = GL_TEXTURE_3D;
	static const enum_t binding = GL_TEXTURE_BINDING_3D;
};

template <>
struct texture_traits<texture_type::texture_rectangle>
{
	static const enum_t target = GL_TEXTURE_RECTANGLE;
	static const enum_t binding = GL_TEXTURE_BINDING_RECTANGLE;
};

template <>
struct texture_traits<texture_type::texture_buffer>
{
	static const enum_t target = GL_TEXTURE_BUFFER;
	static const enum_t binding = GL_TEXTURE_BINDING_BUFFER;
};

namespace parameter
{

template <texture_type Type>
struct texture : regular_parameter_getter<int_t, texture_traits<Type>::binding>
{
	typedef uint_t value_type;
	
	static GLWRAP_MEMBER_FUNC_DECL
	void set(value_type _value)
	{
		glBindTexture(texture_traits<Type>::target, _value);
	}
};

}

template <texture_type Type, typename T>
void set_texture_parameter(uint_t _texture, enum_t _pname, T _value)
{
	if (GL_ARB_direct_state_access)
	{
		gl_texture_parameter(_texture, _pname, _value);
	}
	else
	{
		scoped_value<parameter::texture<Type>> binding(_texture);
		
		gl_tex_parameter(texture_traits<Type>::target, _pname, _value);
	}
}

}

}
