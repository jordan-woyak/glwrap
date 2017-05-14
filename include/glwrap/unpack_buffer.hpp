#pragma once

// this whole file is lame

namespace gl
{

// TODO: lame
enum class pixel_format : GLenum
{
	r = GL_RED,
	rg = GL_RG,
	rgb = GL_RGB,
	bgr = GL_BGR,
	rgba = GL_RGBA,
	bgra = GL_BGRA
};

template <typename T, int D>
class unpack_buffer
{
public:
	unpack_buffer(T const* _data, pixel_format _pfmt, basic_vec<int_t, 2> const& _dims)
		: m_data(_data)
		, m_pfmt(_pfmt)
		, m_dims(_dims)
	{}

	// TODO: GL_UNPACK_SWAP_BYTES, GL_UNPACK_LSB_FIRST, GL_UNPACK_ROW_LENGTH, GL_UNPACK_ALIGNMENT

// TODO: fix privacy
//private:
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
