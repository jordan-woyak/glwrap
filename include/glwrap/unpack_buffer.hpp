#pragma once

// TODO: rename/move this whole file is lame

#include "buffer.hpp"
#include "image_format.hpp"

namespace GLWRAP_NAMESPACE
{

template <typename T, int D>
class unpack_buffer
{
public:
	// TODO: this won't work with 1D formats:
	unpack_buffer(T const* _data, pixel_format _pfmt, basic_vec<sizei_t, D> const& _dims)
		: m_buffer()
		, m_data(_data)
		, m_pfmt(_pfmt)
		, m_dims(_dims)
	{}

	// TODO: return a binder object instead?
	void bind() const
	{	
		GLWRAP_GL_CALL(glBindBuffer)(GL_PIXEL_UNPACK_BUFFER, m_buffer);

		// SWAP_BYTES and LSB_FIRST not in GL ES.
		//GLWRAP_GL_CALL(glPixelStorei)(GL_UNPACK_SWAP_BYTES, 0);
		//GLWRAP_GL_CALL(glPixelStorei)(GL_UNPACK_LSB_FIRST, 0);
		
		GLWRAP_GL_CALL(glPixelStorei)(GL_UNPACK_ROW_LENGTH, m_ps_row_length);
		GLWRAP_GL_CALL(glPixelStorei)(GL_UNPACK_IMAGE_HEIGHT, m_ps_image_height);
		GLWRAP_GL_CALL(glPixelStorei)(GL_UNPACK_ALIGNMENT, m_ps_alignment);
	}

// TODO: fix privacy
//private:
	uint_t m_buffer;

	int_t m_ps_row_length = 0;
	int_t m_ps_image_height = 0;
	int_t m_ps_alignment = 4;
	
	T const* const m_data;
	pixel_format const m_pfmt;
	basic_vec<sizei_t, D> const m_dims;
};

// TODO: allow for {} syntax for dimensions

template <typename T, typename D>
auto unpack(T const* _data, pixel_format _pfmt, D const& _dims)
	-> unpack_buffer<T, detail::vec_traits<D>::dimensions> 
{
	return {_data, _pfmt, _dims};
}

template <typename T, typename D>
auto unpack(gl::tight_buffer_iterator<T> const& _data, pixel_format _pfmt, D const& _dims)
	-> unpack_buffer<T, detail::vec_traits<D>::dimensions> 
{
	unpack_buffer<T, detail::vec_traits<D>::dimensions>  ret{_data.get_offset(), _pfmt, _dims};
	ret.m_buffer = _data.get_buffer();
	return ret;
}

}
