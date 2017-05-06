#pragma once

#include <array>

#include "util.hpp"
#include "detail/traits.hpp"

namespace gl
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

static_assert(sizeof(vec3) == sizeof(float) * 3, "Vec3 size is not sane.");

namespace detail
{

// TODO: better name?
template <typename T, int N>
struct vec
{};

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

template <typename T>
struct vec_traits
{
	vec_traits()
	{
		static_assert(is_vec<T>::value, "invalid vector");
	}
	
	// TODO: This is pretty hacky..
	static const int dimensions = sizeof(T) / sizeof(T::x);
	
	typedef typename T::value_type value_type;
};

}

template <typename T, int N>
using basic_vec = typename detail::vec<T, N>::type;

}

