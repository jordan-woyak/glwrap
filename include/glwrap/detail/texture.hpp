#pragma once

#include "../declarations.hpp"

namespace GLWRAP_NAMESPACE
{

namespace detail
{

void gl_tex_parameter(enum_t _target, enum_t _pname, float_t _value)
{
	GLWRAP_GL_CALL(glTexParameterf)(_target, _pname, _value);
}

void gl_tex_parameter(enum_t _target, enum_t _pname, int_t _value)
{
	GLWRAP_GL_CALL(glTexParameteri)(_target, _pname, _value);
}

void gl_texture_parameter(uint_t _texture, enum_t _pname, float_t _value)
{
	GLWRAP_GL_CALL(glTextureParameterf)(_texture, _pname, _value);
}

void gl_texture_parameter(uint_t _texture, enum_t _pname, int_t _value)
{
	GLWRAP_GL_CALL(glTextureParameteri)(_texture, _pname, _value);
}


template <texture_type T, typename Enable = void>
struct texture_dims;

/*
template <texture_type T>
struct texture_dims<T, typename std::enable_if<
	(T == texture_type::texture_1d) ||
	(T == texture_type::texture_buffer)
	>::type>
{
	static const int value = 1;
	typedef float_t type;
};
*/

template <texture_type T>
struct texture_dims<T, typename std::enable_if<
	(T == texture_type::texture_2d)
	>::type>
{
	static const int value = 2;
	typedef ivec2 type;
};

template <texture_type T>
struct texture_dims<T, typename std::enable_if<
	(T == texture_type::texture_3d) ||
	(T == texture_type::texture_2d_array)
	>::type>
{
	static const int value = 3;
	typedef ivec3 type;
};

template <texture_type T>
using tex_dims = typename texture_dims<T>::type;

template <texture_type T>
struct texture_traits;

/*
template <>
struct texture_traits<texture_type::texture_1d>
{
	static const enum_t target = GL_TEXTURE_1D;
	static const enum_t binding = GL_TEXTURE_BINDING_1D;
};
* */

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
/*
template <>
struct texture_traits<texture_type::texture_buffer>
{
	static const enum_t target = GL_TEXTURE_BUFFER;
	static const enum_t binding = GL_TEXTURE_BINDING_BUFFER;
};
*/
namespace parameter
{

template <texture_type Type>
struct texture : regular_parameter_getter<int_t, texture_traits<Type>::binding>
{
	typedef uint_t value_type;
	
	static GLWRAP_MEMBER_FUNC_DECL
	void set(value_type _value)
	{
		GLWRAP_GL_CALL(glBindTexture)(texture_traits<Type>::target, _value);
	}
};

}

// TODO: kill this. move the direct_state check into the texture class
template <texture_type Type, typename T>
void set_texture_parameter(uint_t _texture, enum_t _pname, T _value)
{
	if (is_extension_present(GL_ARB_direct_state_access))
	{
		gl_texture_parameter(_texture, _pname, _value);
	}
	else
	{
		scoped_value<parameter::texture<Type>> binding(_texture);
		
		gl_tex_parameter(texture_traits<Type>::target, _pname, _value);
	}
}

// glTexStorage*

template <texture_type TexType>
typename std::enable_if<2 == texture_dims<TexType>::value>::type
gl_tex_storage(int_t _levels, int_t _internal_format, const tex_dims<TexType>& _dims)
{
	GLWRAP_GL_CALL(glTexStorage2D)(texture_traits<TexType>::target, _levels, _internal_format,
		_dims.x, _dims.y);
}

template <texture_type TexType>
typename std::enable_if<3 == texture_dims<TexType>::value>::type
gl_tex_storage(int_t _levels, int_t _internal_format, const tex_dims<TexType>& _dims)
{
	GLWRAP_GL_CALL(glTexStorage3D)(texture_traits<TexType>::target, _levels, _internal_format,
		_dims.x, _dims.y, _dims.z);
}

// glTextureStorage*

template <texture_type TexType>
typename std::enable_if<2 == texture_dims<TexType>::value>::type
gl_texture_storage(uint_t _texture, int_t _levels, int_t _internal_format, const tex_dims<TexType>& _dims)
{
	GLWRAP_GL_CALL(glTextureStorage2D)(_texture, _levels, _internal_format,
		_dims.x, _dims.y);
}

template <texture_type TexType>
typename std::enable_if<3 == texture_dims<TexType>::value>::type
gl_texture_storage(uint_t _texture, int_t _levels, int_t _internal_format, const tex_dims<TexType>& _dims)
{
	GLWRAP_GL_CALL(glTextureStorage3D)(_texture, _levels, _internal_format,
		_dims.x, _dims.y, _dims.z);
}

// glTexImage*

template <texture_type TexType, typename DataType>
typename std::enable_if<2 == texture_dims<TexType>::value>::type
gl_tex_image(int_t _level, int_t _internal_format,
	const tex_dims<TexType>& _dims, enum_t _format, const DataType* _data)
{
	GLWRAP_GL_CALL(glTexImage2D)(texture_traits<TexType>::target, _level, _internal_format,
		_dims.x, _dims.y,
		0, _format, detail::data_type_enum<DataType>(), _data);
}

template <texture_type TexType, typename DataType>
typename std::enable_if<3 == texture_dims<TexType>::value>::type
gl_tex_image(int_t _level, int_t _internal_format,
	const tex_dims<TexType>& _dims, enum_t _format, const DataType* _data)
{
	GLWRAP_GL_CALL(glTexImage3D)(texture_traits<TexType>::target, _level, _internal_format,
		_dims.x, _dims.y, _dims.z,
		0, _format, detail::data_type_enum<DataType>(), _data);
}

// glTexSubImage*

template <texture_type TexType, typename DataType>
typename std::enable_if<2 == texture_dims<TexType>::value>::type
gl_tex_sub_image(int_t _level, const tex_dims<TexType>& _offset,
	const tex_dims<TexType>& _dims, enum_t _format, const DataType* _data)
{
	GLWRAP_GL_CALL(glTexSubImage2D)(texture_traits<TexType>::target, _level,
		_offset.x, _offset.y,
		_dims.x, _dims.y,
		_format, detail::data_type_enum<DataType>(), _data);
}

template <texture_type TexType, typename DataType>
typename std::enable_if<3 == texture_dims<TexType>::value>::type
gl_tex_sub_image(int_t _level, const tex_dims<TexType>& _offset,
	const tex_dims<TexType>& _dims, enum_t _format, const DataType* _data)
{
	GLWRAP_GL_CALL(glTexSubImage3D)(texture_traits<TexType>::target, _level,
		_offset.x, _offset.y, _offset.z,
		_dims.x, _dims.y, _dims.z,
		_format, detail::data_type_enum<DataType>(), _data);
}

// glTextureSubImage*

// TODO: kinda ugly that these two need the texture_type just to get the dimension type
template <texture_type TexType, typename DataType>
typename std::enable_if<2 == texture_dims<TexType>::value>::type
gl_texture_sub_image(uint_t _texture, int_t _level, const tex_dims<TexType>& _offset,
	const tex_dims<TexType>& _dims, enum_t _format, const DataType* _data)
{
	GLWRAP_GL_CALL(glTextureSubImage2D)(_texture, _level,
		_offset.x, _offset.y,
		_dims.x, _dims.y,
		_format, detail::data_type_enum<DataType>(), _data);
}

template <texture_type TexType, typename DataType>
typename std::enable_if<3 == texture_dims<TexType>::value>::type
gl_texture_sub_image(uint_t _texture, int_t _level, const tex_dims<TexType>& _offset,
	const tex_dims<TexType>& _dims, enum_t _format, const DataType* _data)
{
	GLWRAP_GL_CALL(glTextureSubImage3D)(_texture, _level,
		_offset.x, _offset.y, _offset.z,
		_dims.x, _dims.y, _dims.z,
		_format, detail::data_type_enum<DataType>(), _data);
}

// glCopyTexSubImage*

template <texture_type TexType>
typename std::enable_if<2 == texture_dims<TexType>::value>::type
gl_copy_tex_sub_image(uint_t _target, int_t _level, const tex_dims<TexType>& _offset,
	const ivec2& _pos, const ivec2& _size)
{
	GLWRAP_GL_CALL(glCopyTexSubImage2D)(texture_traits<TexType>::target, _level,
		_offset.x, _offset.y,
		_pos.x, _pos.y,
		_size.x, _size.y);
}

template <texture_type TexType>
typename std::enable_if<3 == texture_dims<TexType>::value>::type
gl_copy_tex_sub_image(int_t _level, const tex_dims<TexType>& _offset,
	const ivec2& _pos, const ivec2& _size)
{
	GLWRAP_GL_CALL(glCopyTexSubImage3D)(texture_traits<TexType>::target, _level,
		_offset.x, _offset.y, _offset.z,
		_pos.x, _pos.y,
		_size.x, _size.y);
}

// glCopyTextureSubImage*

template <texture_type TexType>
typename std::enable_if<2 == texture_dims<TexType>::value>::type
gl_copy_texture_sub_image(uint_t _texture, int_t _level, const tex_dims<TexType>& _offset,
	const ivec2& _pos, const ivec2& _size)
{
	GLWRAP_GL_CALL(glCopyTextureSubImage2D)(_texture, _level,
		_offset.x, _offset.y,
		_pos.x, _pos.y,
		_size.x, _size.y);
}

template <texture_type TexType>
typename std::enable_if<3 == texture_dims<TexType>::value>::type
gl_copy_texture_sub_image(uint_t _texture, int_t _level, const tex_dims<TexType>& _offset,
	const ivec2& _pos, const ivec2& _size)
{
	GLWRAP_GL_CALL(glCopyTextureSubImage3D)(_texture, _level,
		_offset.x, _offset.y, _offset.z,
		_pos.x, _pos.y,
		_size.x, _size.y);
}

}

}
