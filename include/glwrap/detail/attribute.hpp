#pragma once

#include "../util.hpp"
#include "variable.hpp"

namespace gl
{

namespace detail
{

// TODO: arrays:

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

// TODO: index resource count == *2 for doubles
// implement this against comparison to GL_MAX_VERTEX_ATTRIBS

template <typename T, typename Enable = void>
struct attrib_index_count;

template <typename T>
struct attrib_index_count<T, typename std::enable_if<
	std::is_same<T, int_t>::value ||
	std::is_same<T, uint_t>::value ||
	std::is_same<T, float_t>::value ||
	std::is_same<T, double_t>::value
	, void>::type>
{
	static const uint_t value = 1;
};

template <typename T>
struct attrib_index_count<T, typename std::enable_if<is_vec<T>::value>::type>
{
	static const uint_t value =
		attrib_index_count<typename vec_traits<T>::value_type>::value;
};

template <typename T>
struct attrib_index_count<T, typename std::enable_if<is_mat<T>::value>::type>
{
	static const uint_t value =
		attrib_index_count<typename mat_traits<T>::value_type>::value * mat_traits<T>::cols;
};

template <typename T>
struct attrib_index_count<T, typename std::enable_if<std::is_array<T>::value>::type>
{
	static const uint_t value =
		attrib_index_count<typename std::remove_extent<T>::type>::value * std::extent<T>::value;
};

// TODO: normalize parameter
// TODO: doubles
// TODO: matrices
// TODO: arrays

// TODO: move this elsewhere?
template <typename T, typename Enable = void>
struct attrib_traits;

// TODO: is_scalar is a bad check, will fail when gl_half and stuff are implemented
template <typename T>
struct attrib_traits<T, typename std::enable_if<std::is_scalar<T>::value>::type>
{
	static const enum_t size = 1;
	static const enum_t type = detail::data_type_enum<T>();
	static const bool is_integral = std::is_integral<T>::value;
};

template <typename T>
struct attrib_traits<T, typename std::enable_if<is_vec<T>::value>::type>
{
private:
	typedef detail::vec_traits<T> traits;
	typedef typename traits::value_type value_type;

public:
	static const enum_t size = traits::dimensions;
	static const enum_t type = attrib_traits<value_type>::type;
	static const bool is_integral = attrib_traits<value_type>::is_integral;
};

template <typename ShaderT, typename InputT, bool Normalize>
typename std::enable_if<!attrib_traits<ShaderT>::is_integral>::type
inline gl_vertex_attrib_format(uint_t _index, uint_t _offset)
{
	typedef attrib_traits<InputT> traits;
	
	GLWRAP_EC_CALL(glVertexAttribFormat)(_index, traits::size, traits::type, Normalize, _offset);
}

template <typename ShaderT, typename InputT, bool Normalize>
typename std::enable_if<attrib_traits<ShaderT>::is_integral>::type
inline gl_vertex_attrib_format(uint_t _index, uint_t _offset)
{
	typedef attrib_traits<InputT> traits;
	
	GLWRAP_EC_CALL(glVertexAttribIFormat)(_index, traits::size, traits::type, _offset);
}

template <typename ShaderT, typename InputT, bool Normalize>
typename std::enable_if<std::is_array<ShaderT>::value>::type
inline gl_vertex_attrib_format(uint_t _index, uint_t _offset)
{
	typedef typename std::remove_extent<ShaderT>::type shader_type;
	typedef typename std::remove_extent<InputT>::type input_type;
	
	const uint_t length = std::extent<ShaderT>::value;
	const uint_t index_count = detail::attrib_index_count<shader_type>::value;

	for (uint_t i = 0; i != length; ++i)
		gl_vertex_attrib_format<shader_type, input_type, Normalize>(_index + index_count * i, _offset + sizeof(input_type) * i);
}

template <typename ShaderT, typename InputT, bool Normalize>
typename std::enable_if<!attrib_traits<ShaderT>::is_integral>::type
inline gl_vertex_array_attrib_format(GLuint _vao, uint_t _index, uint_t _offset)
{
	typedef attrib_traits<InputT> traits;
	
	GLWRAP_EC_CALL(glVertexArrayAttribFormat)(_vao, _index, traits::size, traits::type, Normalize, _offset);
}

template <typename ShaderT, typename InputT, bool Normalize>
typename std::enable_if<attrib_traits<ShaderT>::is_integral>::type
inline gl_vertex_array_attrib_format(GLuint _vao, uint_t _index, uint_t _offset)
{
	typedef attrib_traits<InputT> traits;
	
	GLWRAP_EC_CALL(glVertexArrayAttribIFormat)(_vao, _index, traits::size, traits::type, _offset);
}

template <typename ShaderT, typename InputT, bool Normalize>
typename std::enable_if<std::is_array<ShaderT>::value>::type
inline gl_vertex_array_attrib_format(GLuint _vao, uint_t _index, uint_t _offset)
{
	typedef typename std::remove_extent<ShaderT>::type shader_type;
	typedef typename std::remove_extent<InputT>::type input_type;
	
	const uint_t length = std::extent<ShaderT>::value;
	const uint_t index_count = detail::attrib_index_count<shader_type>::value;

	for (uint_t i = 0; i != length; ++i)
		gl_vertex_array_attrib_format<shader_type, input_type, Normalize>(_vao, _index + index_count * i, _offset + sizeof(input_type) * i);
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
