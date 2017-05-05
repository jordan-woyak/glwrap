#pragma once

#include "../util.hpp"

namespace gl
{
namespace detail
{

// TODO: these need to know the target generic vertex attrib type!

template <typename T, typename Enable = void>
struct vertex_attrib_pointer
{
	vertex_attrib_pointer()
	{
		static_assert(false && std::is_void<T>::value, "Unsupported vertex attrib pointer type.");
	}
};

template <typename T>
struct vertex_attrib_pointer<T, typename std::enable_if<detail::is_gl_floating_point<T>::value>::type>
{
	static void bind(uint_t _index, sizei_t _stride, const GLvoid* _offset)
	{
		// TODO: enable use of normalize parameter
		glVertexAttribPointer(_index, 1, detail::data_type_enum<T>(), GL_FALSE, _stride, _offset);
	}
};

template <typename T>
struct vertex_attrib_pointer<T, typename std::enable_if<detail::is_vec<T>::value && detail::is_gl_floating_point<typename detail::vec_traits<T>::value_type>::value>::type>
{
	static void bind(uint_t _index, sizei_t _stride, const GLvoid* _offset)
	{
		typedef typename detail::vec_traits<T>::value_type value_type;
		
		// TODO: enable use of normalize parameter
		glVertexAttribPointer(_index, detail::vec_traits<T>::dimensions, detail::data_type_enum<value_type>(), GL_FALSE, _stride, _offset);
	}
};

// TODO: ints are all broken

template <typename T>
struct vertex_attrib_pointer<T, typename std::enable_if<detail::is_gl_integral<T>::value>::type>
{
	static void bind(uint_t _index, sizei_t _stride, const GLvoid* _offset)
	{
		glVertexAttribIPointer(_index, 1, detail::data_type_enum<T>(), _stride, _offset);
	}
};

template <typename T>
struct vertex_attrib_pointer<T, typename std::enable_if<detail::is_vec<T>::value && detail::is_gl_integral<typename detail::vec_traits<T>::value_type>::value>::type>
{
	static void bind(uint_t _index, sizei_t _stride, const GLvoid* _offset)
	{
		typedef typename detail::vec_traits<T>::value_type value_type;
		
		glVertexAttribIPointer(_index, detail::vec_traits<T>::dimensions, detail::data_type_enum<value_type>(), _stride, _offset);
	}
};

}
}
