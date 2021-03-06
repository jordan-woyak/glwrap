#pragma once

#include <array>

#include "util.hpp"
#include "detail/traits.hpp"

namespace GLWRAP_NAMESPACE
{

using glm::vec2;
using glm::vec3;
using glm::vec4;

using glm::ivec2;
using glm::ivec3;
using glm::ivec4;

using glm::uvec2;
using glm::uvec3;
using glm::uvec4;

using glm::bvec2;
using glm::bvec3;
using glm::bvec4;

static_assert(sizeof(vec3) == sizeof(float) * 3, "Vec3 size is not sane.");

namespace detail
{

// TODO: better name?
template <typename T, int N>
struct vec;

template <typename T>
struct vec<T, 2>
{
	typedef glm::tvec2<T, glm::defaultp> type;
};

template <typename T>
struct vec<T, 3>
{
	typedef glm::tvec3<T, glm::defaultp> type;
};

template <typename T>
struct vec<T, 4>
{
	typedef glm::tvec4<T, glm::defaultp> type;
};

template <typename T, typename Enable = void>
struct vec_traits;

template <typename T>
struct vec_traits<T, typename std::enable_if<is_vec<T>::value>::type>
{
	// TODO: This is pretty hacky..
	static const int dimensions = sizeof(T) / sizeof(T::x);
	
	typedef typename T::value_type value_type;
};

}

template <typename T, int N>
using basic_vec = typename detail::vec<T, N>::type;

namespace detail
{

template <typename T, int N>
GLWRAP_FUNC_DECL
T* value_ptr(const basic_vec<T, N>& _val)
{
	return glm::value_ptr(_val);
}

}

}

