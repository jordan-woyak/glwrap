#pragma once

#include "../util.hpp"

namespace gl
{

namespace detail
{

// TODO: normalize parameter
// TODO: doubles

// TODO: move this elsewhere?
template <typename T, typename Enable = void>
struct attrib_traits
{};

template <typename T>
struct attrib_traits<T, typename std::enable_if<!is_vec<T>::value>::type>
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
	static const enum_t size = traits::dimensions();
	static const enum_t type = attrib_traits<value_type>::type;
	static const bool is_integral = attrib_traits<value_type>::is_integral;
};

template <typename ShaderT, typename InputT, bool Normalize>
typename std::enable_if<!attrib_traits<ShaderT>::is_integral>::type
inline gl_vertex_attrib_format(uint_t _index, uint_t _offset)
{
	typedef attrib_traits<InputT> traits;
	
	glVertexAttribFormat(_index, traits::size, traits::type, Normalize, _offset);
}

template <typename ShaderT, typename InputT, bool Normalize>
typename std::enable_if<attrib_traits<ShaderT>::is_integral>::type
inline gl_vertex_attrib_format(uint_t _index, uint_t _offset)
{
	typedef attrib_traits<InputT> traits;
	
	glVertexAttribIFormat(_index, traits::size, traits::type, _offset);
}

template <typename ShaderT, typename InputT, bool Normalize>
typename std::enable_if<!attrib_traits<ShaderT>::is_integral>::type
inline gl_vertex_array_attrib_format(GLuint _vao, uint_t _index, uint_t _offset)
{
	typedef attrib_traits<InputT> traits;
	
	glVertexArrayAttribFormat(_vao, _index, traits::size, traits::type, Normalize, _offset);
}

template <typename ShaderT, typename InputT, bool Normalize>
typename std::enable_if<attrib_traits<ShaderT>::is_integral>::type
inline gl_vertex_array_attrib_format(GLuint _vao, uint_t _index, uint_t _offset)
{
	typedef attrib_traits<InputT> traits;
	
	glVertexArrayAttribIFormat(_vao, _index, traits::size, traits::type, _offset);
}

// TODO: kill this:
template <typename ShaderT, typename InputT, bool Normalize>
typename std::enable_if<!attrib_traits<ShaderT>::is_integral>::type
inline gl_vertex_attrib_pointer(uint_t _index, uint_t _stride, const void_t* _offset)
{
	typedef attrib_traits<InputT> traits;
	
	glVertexAttribPointer(_index, traits::size, traits::type, Normalize, _stride, _offset);
}

// TODO: kill this:
template <typename ShaderT, typename InputT, bool Normalize>
typename std::enable_if<attrib_traits<ShaderT>::is_integral>::type
inline gl_vertex_attrib_pointer(uint_t _index, uint_t _stride, const void_t* _offset)
{
	typedef attrib_traits<InputT> traits;
	
	glVertexAttribIPointer(_index, traits::size, traits::type, _stride, _offset);
}

}

}
