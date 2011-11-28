
#pragma once

#include <vector>

#include <GL/glew.h>

#include "vector.hpp"

namespace gl
{

typedef GLbyte byte_t;
typedef GLubyte ubyte_t;
typedef GLshort short_t;
typedef GLushort ushort_t;
typedef GLint int_t;
typedef GLuint uint_t;
typedef GLfloat float_t;
typedef GLdouble double_t;
typedef GLboolean bool_t;
typedef GLsizei sizei_t;
typedef GLbitfield bitfield_t;
typedef double_t depth_t;

// TODO: move this
typedef void(*glgenfunc)(GLsizei, GLuint*);
GLuint gen_return(glgenfunc f)
{
	GLuint name;
	f(1, &name);
	return name;
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

template <typename T>
struct remove_reference_cv : std::remove_cv<typename std::remove_reference<T>::type>
{};

template <typename T, typename U>
struct is_same_ignore_reference_cv : std::is_same<
	typename remove_reference_cv<T>::type,
	typename remove_reference_cv<U>::type
	>
{};

template <typename T>
struct is_std_vector : std::false_type
{};

template <>
template <typename U>
struct is_std_vector<std::vector<U>> : std::true_type
{};

template <typename T>
struct is_std_array : std::false_type
{};

template <>
template <typename U, std::size_t V>
struct is_std_array<std::array<U, V>> : std::true_type
{};

// TODO: initializer list?

template <typename T>
struct is_contiguous : std::integral_constant<bool,
	(is_std_vector<typename remove_reference_cv<T>::type>::value ||
	is_std_array<typename remove_reference_cv<T>::type>::value ||
	std::is_array<typename remove_reference_cv<T>::type>::value) &&
	!std::is_same<std::vector<bool>, typename remove_reference_cv<T>::type>::value
	>
{};

static_assert(is_contiguous<std::vector<int>&>::value, "fail");
static_assert(is_contiguous<const std::array<float, 4>&>::value, "fail");
static_assert(is_contiguous<int[5]>::value, "fail");
static_assert(is_contiguous<float(&)[1]>::value, "fail");
static_assert(!is_contiguous<std::list<char>>::value, "fail");

// TODO: boost provide something for this?
template <typename T>
struct range_traits
{
	// am I being stupid?
	template <typename U>
	static U return_t();

	typedef decltype(*std::begin(return_t<T>())) value_type;
};

template <typename T, typename U>
typename std::enable_if<is_contiguous<U>::value &&
	is_same_ignore_reference_cv<T, typename range_traits<U>::value_type>::value, U>::type
get_contiguous_range(U&& _range)
{
	return std::forward<U>(_range);
}

// TODO: should I really just copy to a vector if the data is of the wrong type?
template <typename T, typename U>
typename std::enable_if<!is_contiguous<U>::value ||
	!is_same_ignore_reference_cv<T, typename range_traits<U>::value_type>::value, std::vector<T>>::type
get_contiguous_range(U&& _range)
{
	return {std::begin(_range), std::end(_range)};

	static_assert(sizeof(T) == 0, "using this guy");
}

}

/*
// TODO: do something when not contiguous?
template <typename IterIn, typename IterOut>
void copy(IterIn _begin, IterIn _end, IterOut _dest)
{

}
*/

template <typename T, typename Min, typename Max>
auto clamp(T&& _val, Min&& _min, Max&& _max) -> typename std::remove_reference<T>::type
{
	return _val < _min ? _min : (_val < _max ? _val : _max);
}

}
