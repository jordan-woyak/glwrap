#pragma once

#include "../util.hpp"

namespace gl
{
namespace detail
{

// TODO: these need to know the target generic vertex attrib type!

template <typename T>
struct vertex_attrib_pointer;

template <>
struct vertex_attrib_pointer<float_t>
{
	static void bind(uint_t _index, sizei_t _stride, const GLvoid* _offset)
	{
		// TODO: enable use of normalize parameter
		glVertexAttribPointer(_index, 1, detail::data_type_enum<float_t>(), GL_FALSE, _stride, _offset);
	}
};

template <>
struct vertex_attrib_pointer<int_t>
{
	static void bind(uint_t _index, sizei_t _stride, const GLvoid* _offset)
	{
		glVertexAttribIPointer(_index, 1, detail::data_type_enum<int_t>(), _stride, _offset);
	}
};

template <>
template <int D>
struct vertex_attrib_pointer<basic_vec<float_t, D>>
{
	static void bind(uint_t _index, sizei_t _stride, const GLvoid* _offset)
	{
		// TODO: enable use of normalize parameter
		glVertexAttribPointer(_index, D, detail::data_type_enum<float_t>(), GL_FALSE, _stride, _offset);
	}
};

}
}
