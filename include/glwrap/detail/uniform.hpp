#pragma once

#include "../texture.hpp"

namespace GLWRAP_NAMESPACE
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
	GLWRAP_GL_CALL(glUniform##u_name##v)(_loc, _count, value_ptr(*_value)); \
} \
\
GLWRAP_FUNC_DECL \
void \
gl_program_uniform_v(uint_t _program, int_t _loc, sizei_t _count, const v_type* _value) \
{ \
	GLWRAP_GL_CALL(glProgramUniform##u_name##v)(_program, _loc, _count, value_ptr(*_value)); \
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
	GLWRAP_GL_CALL(glUniformMatrix##u_name##v)(_loc, _count, IS_UNIFORM_MATRIX_TRANSPOSE_NEEDED, value_ptr(*_value)); \
} \
\
GLWRAP_FUNC_DECL \
void \
gl_program_uniform_v(uint_t _program, int_t _loc, sizei_t _count, const v_type* _value) \
{ \
	GLWRAP_GL_CALL(glProgramUniformMatrix##u_name##v)(_program, _loc, _count, IS_UNIFORM_MATRIX_TRANSPOSE_NEEDED, value_ptr(*_value)); \
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

// To be used as a short-lived object to set a bunch of uniforms of a single program
class bindless_uniform_setter
{
public:
	bindless_uniform_setter(uint_t _program)
		: m_program(_program)
	{}

	template <typename T>
	void set(int_t _loc, sizei_t _count, const T* _value) const
	{
		gl_program_uniform_v(m_program, _loc, _count, _value);
	}

private:
	const uint_t m_program;
};

// To be used as a short-lived object to set a bunch of uniforms of a single program
class binding_uniform_setter
{
public:
	binding_uniform_setter(uint_t _program)
		: m_binding(_program)
	{}

	template <typename T>
	void set(int_t _loc, sizei_t _count, const T* _value) const
	{
		gl_uniform_v(_loc, _count, _value);
	}

private:
	const scoped_value<parameter::program> m_binding;
};

// TODO: rename this and use it to do all the conversion:
// TODO: rename convert_and_set

template <typename T, typename Enable = void>
struct uniform_value;

// Iterates a range of values, attempting to convert to the proper uploadable type
template <typename T, typename S, typename I>
static void set_uniforms_from_iterator(S&& setter, int_t _loc, sizei_t _count, I _iter)
{
	while (_count)
	{
		const typename uniform_value<T>::type converted_value(*_iter);

		uniform_value<T>::convert_and_set(setter, _loc, 1, &converted_value);

		++_loc;
		++_iter;
		--_count;
	}
}

template <typename T>
struct uniform_value<T, typename std::enable_if<
	(std::is_same<int_t, typename underlying_type<T>::type>::value
	|| std::is_same<uint_t, typename underlying_type<T>::type>::value
	|| std::is_same<float_t, typename underlying_type<T>::type>::value)
	&& (is_mat<T>::value || is_vec<T>::value || std::is_scalar<T>::value)
	>::type>
{
	typedef T type;

	// int,uint,float types can be uploaded directly when already the proper type:
	template <typename S>
	static void convert_and_set(S&& setter, int_t _loc, sizei_t _count, const type* _val)
	{
		setter.set(_loc, _count, _val);
	}

	// Attempt conversion if needed:
	template <typename S, typename V>
	static void convert_and_set(S&& setter, int_t _loc, sizei_t _count, const V* _val)
	{
		set_uniforms_from_iterator<T>(setter, _loc, _count, _val);
	}
};

// bool,bvec must be converted to int,uint,float types
template <typename T>
struct uniform_value<T, typename std::enable_if<
	std::is_same<bool_t, typename underlying_type<T>::type>::value
	&& (is_vec<T>::value || std::is_scalar<T>::value)>::type>
{
	// "Either the i, ui or f variants may be used to provide values
	// for uniform variables of type bool, bvec2, bvec3, bvec4, or arrays of these."

	// Kinda ugly, the "preferred" type is a uint version of T
	//typedef T type;
	typedef typename underlying_type<T>::template modify<uint_t> type;

	template <typename S, typename V>
	static typename std::enable_if<
			std::is_same<V, type>::value
			|| std::is_same<V, typename underlying_type<T>::template modify<int_t>>::value
			|| std::is_same<V, typename underlying_type<T>::template modify<float_t>>::value
		>::type
	convert_and_set(S&& setter, int_t _loc, sizei_t _count, const V* _val)
	{
		setter.set(_loc, _count, _val);
	}

	template <typename S, typename V>
	static typename std::enable_if<
			!(std::is_same<V, type>::value
			|| std::is_same<V, typename underlying_type<T>::template modify<int_t>>::value
			|| std::is_same<V, typename underlying_type<T>::template modify<float_t>>::value)
		>::type
	convert_and_set(S&& setter, int_t _loc, sizei_t _count, const V* _val)
	{
		set_uniforms_from_iterator<T>(setter, _loc, _count, _val);
	}
};

// TODO: basic_image
template <texture_type T, typename D>
struct uniform_value<shader::basic_sampler<T, D>>
{
	// "glUniform1i and glUniform1iv are the only two functions
	// that may be used to load uniform variables defined as sampler types."
	
	typedef texture_unit<shader::basic_sampler<T, D>> type;

	// texture_uint value can be interpreted as an int and uploaded:
	template <typename S>
	static void convert_and_set(S&& setter, int_t _loc, sizei_t _count, const type* _val)
	{
		static_assert(sizeof(type) == sizeof(int_t),
			"sanity check. texture_unit should be binary compatible with int_t.");
		
		setter.set(_loc, _count, reinterpret_cast<const int_t*>(_val));
	}
};

// Arrays
template <typename T>
struct uniform_value<T, typename std::enable_if<std::is_array<T>::value>::type>
{
	typedef typename std::remove_extent<T>::type variable_value_type;
	typedef typename uniform_value<variable_value_type>::type value_type;
	
	typedef std::array<value_type, std::extent<T>::value> type;
	static const int array_length = std::extent<T>::value;

	// Pass contiguous arrays to the underlying type handler:
	template <typename S, typename V>
	static
		//typename std::enable_if<is_contiguous<V>::value>::type
		void
		convert_and_set(S&& setter, int_t _loc, sizei_t _count, const V* _val)
	{
		static_assert(is_contiguous<V>::value, "Uniform arrays currently must be set by contiguous range.");
		
		uniform_value<variable_value_type>::
			convert_and_set(setter, _loc, _count * array_length, &*std::begin(*_val));
	}
};

template <typename S, typename T>
static void set_uniform(S&& setter, int_t _loc, sizei_t _count, const T* _val)
{
	setter.set(_loc, _count, _val);
}

// TODO: allow setting an entire array or a range of elements in an array
// with what syntax for the range?

template <typename T, typename V>
void set_program_uniform(uint_t _program, int_t _loc, V const& _value)
{	
	// TODO: is the proper check?
	if (is_extension_present(GL_ARB_separate_shader_objects))
	{
		detail::uniform_value<T>::convert_and_set(bindless_uniform_setter(_program), _loc, 1, &_value);
	}
	else
	{
		detail::uniform_value<T>::convert_and_set(binding_uniform_setter(_program), _loc, 1, &_value);
	}
}

}

}
