#pragma once

#include "array_buffer.hpp"

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

}
