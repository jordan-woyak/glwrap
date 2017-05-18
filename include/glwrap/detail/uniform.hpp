#pragma once

#include "variable.hpp"

namespace gl
{

namespace detail
{

// TODO: arrays:

template <typename T, typename Enable = void>
struct is_valid_uniform_type : std::false_type
{};

template <typename T>
struct is_valid_uniform_type<T, typename std::enable_if<
	std::is_same<T, bool_t>::value ||
	std::is_same<T, int_t>::value ||
	std::is_same<T, uint_t>::value ||
	std::is_same<T, float_t>::value ||
	std::is_same<T, double_t>::value
	>::type> : std::true_type
{};

template <typename T>
struct is_valid_uniform_type<T, typename std::enable_if<
	is_vec<T>::value &&
	is_valid_vec_size<T>::value &&
	is_valid_vec_value_type<T>::value
	>::type> : std::true_type
{};

template <typename T>
struct is_valid_uniform_type<T, typename std::enable_if<
	is_mat<T>::value &&
	is_valid_mat_size<T>::value &&
	is_valid_mat_value_type<T>::value
	>::type> : std::true_type
{};

template <texture_type T, typename D>
struct is_valid_uniform_type<shader::basic_sampler<T, D>> : std::true_type
{};

template <texture_type T, typename D>
struct is_valid_uniform_type<shader::basic_image<T, D>> : std::true_type
{};

template <typename T, typename Enable = void>
struct uniform_location_count
{
	static const uint_t value = 1;
};

template <typename T>
struct uniform_location_count<T, typename std::enable_if<std::is_array<T>::value>::type>
{
	static const uint_t value =
		uniform_location_count<typename std::remove_extent<T>::type>::value * std::extent<T>::value;
};

//
// gl_uniform_v / gl_program_uniform_v
//
// provides a single name for glUniform*v / glProgramUniform*v functions
// which accepts typed arguments like vec3 and mat4
//

#define GLWRAP_UNIFORM_V_DEF(u_name, v_type) \
GLWRAP_FUNC_DECL \
void \
gl_uniform_v(int_t _loc, sizei_t _count, const v_type* _value) \
{ \
	GLWRAP_EC_CALL(glUniform##u_name##v)(_loc, _count, value_ptr(*_value)); \
} \
\
GLWRAP_FUNC_DECL \
void \
gl_program_uniform_v(uint_t _program, int_t _loc, sizei_t _count, const v_type* _value) \
{ \
	GLWRAP_EC_CALL(glProgramUniform##u_name##v)(_program, _loc, _count, value_ptr(*_value)); \
}

GLWRAP_UNIFORM_V_DEF(1f, float_t)
GLWRAP_UNIFORM_V_DEF(2f, vec2)
GLWRAP_UNIFORM_V_DEF(3f, vec3)
GLWRAP_UNIFORM_V_DEF(4f, vec4)

GLWRAP_UNIFORM_V_DEF(1i, int_t)
GLWRAP_UNIFORM_V_DEF(2i, ivec2)
GLWRAP_UNIFORM_V_DEF(3i, ivec3)
GLWRAP_UNIFORM_V_DEF(4i, ivec4)

GLWRAP_UNIFORM_V_DEF(1ui, uint_t)
GLWRAP_UNIFORM_V_DEF(2ui, uvec2)
GLWRAP_UNIFORM_V_DEF(3ui, uvec3)
GLWRAP_UNIFORM_V_DEF(4ui, uvec4)

#undef GLWRAP_UNIFORM_V_DEF

#define GLWRAP_UNIFORM_MAT_V_DEF(u_name, v_type) \
GLWRAP_FUNC_DECL \
void \
gl_uniform_v(int_t _loc, sizei_t _count, const v_type* _value) \
{ \
	GLWRAP_EC_CALL(glUniformMatrix##u_name##v)(_loc, _count, IS_UNIFORM_MATRIX_TRANSPOSE_NEEDED, value_ptr(*_value)); \
} \
\
GLWRAP_FUNC_DECL \
void \
gl_program_uniform_v(uint_t _program, int_t _loc, sizei_t _count, const v_type* _value) \
{ \
	GLWRAP_EC_CALL(glProgramUniformMatrix##u_name##v)(_program, _loc, _count, IS_UNIFORM_MATRIX_TRANSPOSE_NEEDED, value_ptr(*_value)); \
}

GLWRAP_UNIFORM_MAT_V_DEF(2f, mat2)
GLWRAP_UNIFORM_MAT_V_DEF(2x3f, mat2x3)
GLWRAP_UNIFORM_MAT_V_DEF(2x4f, mat2x4)

GLWRAP_UNIFORM_MAT_V_DEF(3x2f, mat3x2)
GLWRAP_UNIFORM_MAT_V_DEF(3f, mat3)
GLWRAP_UNIFORM_MAT_V_DEF(3x4f, mat3x4)

GLWRAP_UNIFORM_MAT_V_DEF(4x2f, mat4x2)
GLWRAP_UNIFORM_MAT_V_DEF(4x3f, mat4x3)
GLWRAP_UNIFORM_MAT_V_DEF(4f, mat4)

#undef GLWRAP_UNIFORM_MAT_V_DEF

// TODO: implement not in terms of gl_uniform_v ?
template <typename T>
void gl_uniform(int_t _loc, const T& _value)
{
	gl_uniform_v(_loc, 1, &_value);
}

template <typename T>
void gl_program_uniform(uint_t _program, int_t _loc, const T& _value)
{
	gl_program_uniform_v(_program, _loc, 1, &_value);
}

// TODO: bool and bvec

// To be used as a short-lived object to set a bunch of uniforms of a single program
// Uses glProgramUniform if available or falls back to glUniform
/*
class program_uniform_setter
{
	program_uniform_setter(uint_t _program)
		: m_program(_program)
	{
		
	}

	template <typename T>
	void set(int_t _loc, T _value)
	{
		
	}

private:
	const uint_t m_program;
};
*/

// TODO: rename this:
template <typename T, typename Enable = void>
struct uniform_value
{
	// TODO: enable_if for: float,int,uint, vec,ivec,uvec, and float matrices
	
	typedef T type;
	typedef T gl_type;

	static gl_type convert_to_gl_type(const type& _val)
	{
		return _val;
	}
};

template <>
struct uniform_value<bool_t>
{
	// TODO: handle bvec too
	
	typedef bool_t type;

	// TODO: gl_type can be any of int,uint,float
	typedef bool_t gl_type;	

	template <typename T>
	static
	typename std::enable_if<std::is_same<T, int_t>::value>::type
	convert_to_gl_type(const T& _val)
	{
		return _val;
	}
};

// TODO: basic_image
template <texture_type T, typename D>
struct uniform_value<shader::basic_sampler<T, D>>
{
	// "glUniform1i and glUniform1iv are the only two functions
	// that may be used to load uniform variables defined as sampler types."
	
	typedef texture_unit<shader::basic_sampler<T, D>> type;
	typedef int_t gl_type;

	static_assert(sizeof(type) == sizeof(gl_type),
		"sanity check. setting arrays will require this.");

	static gl_type convert_to_gl_type(const type& _val)
	{
		return _val.get_index();
	}
};

template <typename T>
struct uniform_value<T, typename std::enable_if<std::extent<T>::value>::type>
{
	typedef std::array<typename std::remove_extent<T>::type, std::extent<T>::value> type;
};

// TODO: allow setting an entire array or a range of elements in an array
// with what syntax for the range?

template <typename T, typename V>
void set_program_uniform(uint_t _program, int_t _loc, V const& _value)
{
	// TODO: is the proper check?
	if (GL_ARB_separate_shader_objects)
	{
		gl_program_uniform(_program, _loc, detail::uniform_value<T>::convert_to_gl_type(_value));
	}
	else
	{
		// TODO: ugly
		scoped_value<parameter::program> binding(_program);
		
		gl_uniform(_loc, detail::uniform_value<T>::convert_to_gl_type(_value));
	}
}

}

}
