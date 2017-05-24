#pragma once

#include "../util.hpp"

#include "variable.hpp"

namespace GLWRAP_NAMESPACE
{

namespace detail
{

// TODO: normalize parameter

template <typename T, typename Enable = void>
struct is_valid_attrib_type : std::false_type
{};

template <typename T>
struct is_valid_attrib_type<T, typename std::enable_if<
	// TODO: is bool valid?
	//std::is_same<T, bool_t>::value ||
	std::is_same<T, int_t>::value ||
	std::is_same<T, uint_t>::value ||
	std::is_same<T, float_t>::value ||
	std::is_same<T, double_t>::value
	>::type> : std::true_type
{};

template <typename T>
struct is_valid_attrib_type<T, typename std::enable_if<
	is_vec<T>::value &&
	is_valid_vec_size<T>::value &&
	is_valid_vec_value_type<T>::value
	>::type> : std::true_type
{};

template <typename T>
struct is_valid_attrib_type<T, typename std::enable_if<
	is_mat<T>::value &&
	is_valid_mat_size<T>::value &&
	is_valid_mat_value_type<T>::value
	>::type> : std::true_type
{};

template <typename T>
struct is_valid_attrib_type<T, typename std::enable_if<
	std::is_array<T>::value
	>::type> : is_valid_attrib_type<typename std::remove_extent<T>::type>
{};

// TODO: move this elsewhere?
template <typename T, typename Enable = void>
struct variable_traits;

// TODO: is_scalar is a bad check, will fail when gl_half and stuff are implemented
template <typename T>
struct variable_traits<T, typename std::enable_if<std::is_scalar<T>::value>::type>
{
	typedef T underlying_type;
	
	static const int component_count = 1;
	static const int attrib_index_count = 1;
	// Implementations can count doubles twice for GL_MAX_VERTEX_ATTRIBS
	static const int attrib_resource_count = 1 + std::is_same<double_t, T>::value;
	
	static const enum_t type_enum = detail::data_type_enum<T>();
	static const bool is_integral = std::is_integral<T>::value;
};

template <typename T>
struct variable_traits<T, typename std::enable_if<is_vec<T>::value>::type>
{	
private:
	typedef detail::vec_traits<T> traits;

public:
	typedef typename traits::value_type value_type;
	typedef value_type underlying_type;

	static const int component_count = traits::dimensions;
	static const int attrib_index_count = 1;
	static const int attrib_resource_count = variable_traits<value_type>::attrib_resource_count;
	
	static const enum_t type_enum = variable_traits<value_type>::type_enum;
	static const bool is_integral = variable_traits<value_type>::is_integral;

	static_assert(sizeof(T) == sizeof(value_type) * component_count,
		"Sanity check. Code assumes containers are exactly the size of their contents.");
};

template <typename T>
struct variable_traits<T, typename std::enable_if<std::is_array<T>::value>::type>
{
private:
	static const int length = std::extent<T>::value;

public:
	typedef typename std::remove_extent<T>::type value_type;
	typedef typename variable_traits<value_type>::underlying_type underlying_type;

	static const int component_count = variable_traits<value_type>::component_count * length;
	static const int attrib_index_count = variable_traits<value_type>::attrib_index_count * length;
	static const int attrib_resource_count =
		variable_traits<value_type>::attrib_resource_count * length;
	
	static const enum_t type_enum = variable_traits<value_type>::type_enum;
	static const bool is_integral = variable_traits<value_type>::is_integral;

	static_assert(sizeof(T) == sizeof(value_type) * length,
		"Sanity check. Code assumes containers are exactly the size of their contents.");
};

// Matrices are just an array of vecs
template <typename T>
struct variable_traits<T, typename std::enable_if<is_mat<T>::value>::type>
	: variable_traits<typename T::col_type[mat_traits<T>::cols]>
{};

template <typename ShaderT, typename InputT, bool Normalize>
typename std::enable_if<!variable_traits<ShaderT>::is_integral>::type
inline gl_vertex_attrib_format(uint_t _index, uint_t _offset)
{
	typedef variable_traits<InputT> traits;

	for (uint_t i = 0; i != traits::attrib_index_count; ++i)
	{
		GLWRAP_EC_CALL(glVertexAttribFormat)(
			_index + i, traits::component_count / traits::attrib_index_count,
			traits::type_enum, Normalize, _offset + i * sizeof(InputT) / traits::attrib_index_count);
	}
}

template <typename ShaderT, typename InputT, bool Normalize>
typename std::enable_if<variable_traits<ShaderT>::is_integral>::type
inline gl_vertex_attrib_format(uint_t _index, uint_t _offset)
{
	typedef variable_traits<InputT> traits;
	
	for (uint_t i = 0; i != traits::attrib_index_count; ++i)
	{
		GLWRAP_EC_CALL(glVertexAttribIFormat)(
			_index + i, traits::component_count / traits::attrib_index_count,
			traits::type_enum, _offset + i * sizeof(InputT) / traits::attrib_index_count);
	}
}

template <typename ShaderT>
inline void gl_vertex_attrib_binding(uint_t _index, uint_t _binding)
{
	const uint_t index_count = detail::variable_traits<ShaderT>::attrib_index_count;

	for (uint_t i = 0; i != index_count; ++i)
		GLWRAP_EC_CALL(glVertexAttribBinding)(_index + i, _binding);
}

template <typename ShaderT>
inline void gl_enable_vertex_attrib_array(uint_t _index)
{
	const uint_t index_count = detail::variable_traits<ShaderT>::attrib_index_count;

	for (uint_t i = 0; i != index_count; ++i)
		GLWRAP_EC_CALL(glEnableVertexAttribArray)(_index + i);
}

template <typename ShaderT>
inline void gl_disable_vertex_attrib_array(uint_t _index)
{
	const uint_t index_count = detail::variable_traits<ShaderT>::attrib_index_count;

	for (uint_t i = 0; i != index_count; ++i)
		GLWRAP_EC_CALL(glDisableVertexAttribArray)(_index + i);
}

template <typename ShaderT, typename InputT, bool Normalize>
typename std::enable_if<!variable_traits<ShaderT>::is_integral>::type
inline gl_vertex_array_attrib_format(GLuint _vao, uint_t _index, uint_t _offset)
{
	typedef variable_traits<InputT> traits;
	
	for (uint_t i = 0; i != traits::attrib_index_count; ++i)
	{
		GLWRAP_EC_CALL(glVertexArrayAttribFormat)(_vao,
			_index + i, traits::component_count / traits::attrib_index_count,
			traits::type_enum, Normalize, _offset + i * sizeof(InputT) / traits::attrib_index_count);
	}
}

template <typename ShaderT, typename InputT, bool Normalize>
typename std::enable_if<variable_traits<ShaderT>::is_integral>::type
inline gl_vertex_array_attrib_format(GLuint _vao, uint_t _index, uint_t _offset)
{
	typedef variable_traits<InputT> traits;
	
	for (uint_t i = 0; i != traits::attrib_index_count; ++i)
	{
		GLWRAP_EC_CALL(glVertexArrayAttribIFormat)(_vao,
			_index + i, traits::component_count / traits::attrib_index_count,
			traits::type_enum, _offset + i * sizeof(InputT) / traits::attrib_index_count);
	}
}

template <typename ShaderT>
inline void gl_vertex_array_attrib_binding(GLuint _vao, uint_t _index, uint_t _binding)
{
	const uint_t index_count = detail::variable_traits<ShaderT>::attrib_index_count;

	for (uint_t i = 0; i != index_count; ++i)
		GLWRAP_EC_CALL(glVertexArrayAttribBinding)(_vao, _index + i, _binding);
}

template <typename ShaderT>
inline void gl_enable_vertex_array_attrib(GLuint _vao, uint_t _index)
{
	const uint_t index_count = detail::variable_traits<ShaderT>::attrib_index_count;

	for (uint_t i = 0; i != index_count; ++i)
		GLWRAP_EC_CALL(glEnableVertexArrayAttrib)(_vao, _index + i);
}

template <typename ShaderT>
inline void gl_disable_vertex_array_attrib(GLuint _vao, uint_t _index)
{
	const uint_t index_count = detail::variable_traits<ShaderT>::attrib_index_count;

	for (uint_t i = 0; i != index_count; ++i)
		GLWRAP_EC_CALL(glDisableVertexArrayAttrib)(_vao, _index + i);
}

/*
// TODO: kill this:
template <typename ShaderT, typename InputT, bool Normalize>
typename std::enable_if<!attrib_traits<ShaderT>::is_integral>::type
inline gl_vertex_attrib_pointer(uint_t _index, uint_t _stride, const void_t* _offset)
{
	typedef attrib_traits<InputT> traits;
	
	GLWRAP_EC_CALL(glVertexAttribPointer)(_index, traits::size, traits::type, Normalize, _stride, _offset);
}

// TODO: kill this:
template <typename ShaderT, typename InputT, bool Normalize>
typename std::enable_if<attrib_traits<ShaderT>::is_integral>::type
inline gl_vertex_attrib_pointer(uint_t _index, uint_t _stride, const void_t* _offset)
{
	typedef attrib_traits<InputT> traits;
	
	GLWRAP_EC_CALL(glVertexAttribIPointer)(_index, traits::size, traits::type, _stride, _offset);
}
*/

}

}
