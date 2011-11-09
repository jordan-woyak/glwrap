#pragma once

#include "buffer.hpp"

#include <boost/multi_array.hpp>

// this whole file is lame

namespace gl
{

// TODO: lame
enum class pixel_format : GLenum
{
	red = GL_RED,
	rg = GL_RG,
	rgb = GL_RGB,
	bgr = GL_BGR,
	rgba = GL_RGBA,
	bgra = GL_BGRA
};

template <int D>
class texture;

template <typename T, int D>
class unpack_buffer
{
	friend class texture<D>;

public:
	unpack_buffer(T const* _data, pixel_format _pfmt, basic_vec<int_t, 2> const& _dims)
		: m_data(_data)
		, m_pfmt(_pfmt)
		, m_dims(_dims)
	{}

private:
	T const* const m_data;
	pixel_format const m_pfmt;
	basic_vec<int_t, 2> const m_dims;
};

template <typename T>
unpack_buffer<T, 2> unpack(T const* _data, pixel_format _pfmt, basic_vec<int_t, 2> const& _dims)
{
	return {_data, _pfmt, _dims};
}

namespace detail
{

template <typename T>
GLenum data_type_enum();

template <>
GLenum data_type_enum<ubyte_t>()
{
	return GL_UNSIGNED_BYTE;
}

template <>
GLenum data_type_enum<byte_t>()
{
	return GL_BYTE;
}

template <>
GLenum data_type_enum<ushort_t>()
{
	return GL_UNSIGNED_SHORT;
}

template <>
GLenum data_type_enum<short_t>()
{
	return GL_SHORT;
}

template <>
GLenum data_type_enum<uint_t>()
{
	return GL_UNSIGNED_INT;
}

template <>
GLenum data_type_enum<int_t>()
{
	return GL_INT;
}

template <>
GLenum data_type_enum<float_t>()
{
	return GL_FLOAT;
}

}

}
