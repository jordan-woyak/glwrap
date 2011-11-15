
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
struct is_contiguous : std::false_type
{};

template <>
template <typename U>
struct is_contiguous<std::vector<U>> : std::true_type
{};

template <>
template <typename U, std::size_t V>
struct is_contiguous<std::array<U, V>> : std::true_type
{};

}

}
