#pragma once

#include "../util.hpp"

namespace gl
{

namespace detail
{

// TODO: normalize parameter
// TODO: doubles

// TODO: move this elsewhere
template <typename T, typename Enable = void>
struct attrib_traits
{};

template <typename T>
struct attrib_traits<T, typename std::enable_if<is_vec<T>::value>::type>
{
private:
	typedef detail::vec_traits<T> traits;

public:
	static const enum_t size = traits::dimensions();
	static const enum_t type = detail::data_type_enum<typename traits::value_type>();
};

template <typename T>
struct attrib_traits<T, typename std::enable_if<!is_vec<T>::value>::type>
{
	static const enum_t size = 1;
	static const enum_t type = detail::data_type_enum<T>();
};

template <typename ShaderT, typename InputT, bool Normalize>
typename std::enable_if<!std::is_integral<ShaderT>::value>::type
inline gl_vertex_attrib_format(uint_t _index, uint_t _offset)
{
	typedef attrib_traits<InputT> traits;
	
	glVertexAttribFormat(_index, traits::size, traits::type, GL_FALSE, _offset);
}

template <typename ShaderT, typename InputT, bool Normalize>
typename std::enable_if<std::is_integral<ShaderT>::value>::type
inline gl_vertex_attrib_format(uint_t _index, uint_t _offset)
{
	typedef attrib_traits<InputT> traits;
	
	glVertexAttribIFormat(_index, traits::size, traits::type, _offset);
}

template <typename ShaderT, typename InputT, bool Normalize>
typename std::enable_if<!std::is_integral<ShaderT>::value>::type
inline gl_vertex_array_attrib_format(GLuint _vao, uint_t _index, uint_t _offset)
{
	typedef attrib_traits<InputT> traits;
	
	glVertexArrayAttribFormat(_vao, _index, traits::size, traits::type, GL_FALSE, _offset);
}

// TODO: fully implement
template <typename ShaderT, typename InputT, bool Normalize>
typename std::enable_if<std::is_integral<ShaderT>::value>::type
inline gl_vertex_array_attrib_format(GLuint _vao, uint_t _index, uint_t _offset)
{
	typedef attrib_traits<InputT> traits;
	
	glVertexArrayAttribIFormat(_vao, _index, traits::size, traits::type, _offset);
}

}

}
