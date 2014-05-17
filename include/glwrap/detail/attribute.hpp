#pragma once

#include "../util.hpp"

namespace gl
{
namespace detail
{

// TODO: these need to know the target generic vertex attrib type!

template <typename T, typename Enable = void>
struct vertex_attrib_pointer;

template <typename T>
struct vertex_attrib_pointer<T, typename std::enable_if<detail::is_gl_floating_point<T>::value>::type>
{
	static void bind(uint_t _index, sizei_t _stride, const GLvoid* _offset)
	{
		// TODO: enable use of normalize parameter
		glVertexAttribPointer(_index, 1, detail::data_type_enum<T>(), GL_FALSE, _stride, _offset);
	}
};

template <typename T, int D>
struct vertex_attrib_pointer<basic_vec<T, D>, typename std::enable_if<detail::is_gl_floating_point<T>::value>::type>
{
	static void bind(uint_t _index, sizei_t _stride, const GLvoid* _offset)
	{
		// TODO: enable use of normalize parameter
		glVertexAttribPointer(_index, D, detail::data_type_enum<T>(), GL_FALSE, _stride, _offset);
	}
};

template <typename T>
struct vertex_attrib_pointer<T, typename std::enable_if<detail::is_gl_integral<T>::value>::type>
{
	static void bind(uint_t _index, sizei_t _stride, const GLvoid* _offset)
	{
		glVertexAttribIPointer(_index, 1, detail::data_type_enum<T>(), _stride, _offset);
	}
};

template <typename T, int D>
struct vertex_attrib_pointer<basic_vec<T, D>, typename std::enable_if<detail::is_gl_integral<T>::value>::type>
{
	static void bind(uint_t _index, sizei_t _stride, const GLvoid* _offset)
	{
		glVertexAttribIPointer(_index, D, detail::data_type_enum<T>(), _stride, _offset);
	}
};

}
}
