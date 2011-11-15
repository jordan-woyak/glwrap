
#pragma once

#include <vector>

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

}

}
