#pragma once

#include "../util.hpp"

namespace gl
{
namespace detail
{

template <typename T>
struct vertex_attrib_pointer;

template <>
struct vertex_attrib_pointer<float_t>
{
	static void bind(uint_t _index, sizei_t _stride, const GLvoid* _offset)
	{
		glVertexAttribPointer(_index, 1, detail::data_type_enum<float_t>(), GL_FALSE, _stride, _offset);
	}
};

template <>
template <typename T, int D>
struct vertex_attrib_pointer<basic_vec<T, D>>
{
	static void bind(uint_t _index, sizei_t _stride, const GLvoid* _offset)
	{
		// TODO: enable use normalize parameter
		glVertexAttribPointer(_index, D, detail::data_type_enum<T>(), GL_FALSE, _stride, _offset);
	}
};

}
}
