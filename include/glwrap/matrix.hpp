#pragma once

#include "vector.hpp"
#include "detail/traits.hpp"

namespace gl
{

using glm::mat2;
using glm::mat2x2;
using glm::mat2x3;
using glm::mat2x4;

using glm::mat3;
using glm::mat3x2;
using glm::mat3x3;
using glm::mat3x4;

using glm::mat4;
using glm::mat4x2;
using glm::mat4x3;
using glm::mat4x4;

using glm::dmat2;
using glm::dmat2x2;
using glm::dmat2x3;
using glm::dmat2x4;

using glm::dmat3;
using glm::dmat3x2;
using glm::dmat3x3;
using glm::dmat3x4;

using glm::dmat4;
using glm::dmat4x2;
using glm::dmat4x3;
using glm::dmat4x4;

namespace detail
{

static_assert(sizeof(mat4x2) == sizeof(float) * 8, "Mat4x2 size is not sane.");

// TODO: better name?
template <typename T, int R, int C>
struct mat
{};

// TODO: all the rest
template <typename T>
struct mat<T, 2, 2>
{
	typedef glm::tmat2x2<T, glm::defaultp> type;
};

template <typename T>
struct mat<T, 3, 3>
{
	typedef glm::tmat3x3<T, glm::defaultp> type;
};

template <typename T>
struct mat<T, 4, 4>
{
	typedef glm::tmat4x4<T, glm::defaultp> type;
};

template <typename T, typename Enable = void>
struct mat_traits
{};

template <typename T>
struct mat_traits<T, typename std::enable_if<is_mat<T>::value>::type>
{
	// TODO: This is hacks that relies on glm impl details!
	static const int rows = vec_traits<typename T::col_type>::dimensions;
	static const int cols = vec_traits<typename T::row_type>::dimensions;
	
	typedef typename T::value_type value_type;
};

}

template <typename T, int R, int C>
using basic_mat = typename detail::mat<T, R, C>::type;

// TODO: non-float versions of these functions?
// TODO: only take vec3 instead of xyz?

template <typename T>
inline typename detail::mat<T, 4, 4>::type rotate(T _angle, T _x, T _y, T _z)
{
	return rotate(_angle, basic_vec<T, 3>{_x, _y, _z});
}

template <typename T>
inline typename detail::mat<T, 4, 4>::type rotate(T _angle, basic_vec<T, 3> const& _axis)
{
	return glm::rotate(_angle, _axis);
}

template <typename T>
inline typename detail::mat<T, 4, 4>::type scale(T _x, T _y, T _z)
{
	return scale(basic_vec<T, 3>{_x, _y, _z});
}

template <typename T>
inline typename detail::mat<T, 4, 4>::type scale(basic_vec<T, 3> const& _dims)
{
	return glm::scale(_dims);
}

template <typename T>
inline typename detail::mat<T, 4, 4>::type translate(T _x, T _y, T _z)
{
	return translate(basic_vec<T, 3>{_x, _y, _z});
}

template <typename T>
inline typename detail::mat<T, 4, 4>::type translate(basic_vec<T, 3> const& _dims)
{
	return glm::translate(_dims);
}

template <typename T>
inline typename detail::mat<T, 4, 4>::type ortho(T _left, T _right, T _bottom, T _top, T _near, T _far)
{
	return glm::ortho(_left, _right, _bottom, _top, _near, _far);
}

template <typename T>
inline typename detail::mat<T, 4, 4>::type ortho(T _left, T _right, T _bottom, T _top)
{
	return glm::ortho(_left, _right, _bottom, _top);
}

template <typename T>
inline typename detail::mat<T, 4, 4>::type frustum(T _left, T _right, T _bottom, T _top, T _near, T _far)
{
	return glm::frustum(_left, _right, _bottom, _top, _near, _far);
}

template <typename T>
inline typename detail::mat<T, 4, 4>::type perspective(T fovy, T aspect, T znear, T zfar)
{
	return glm::perspective(fovy, aspect, znear, zfar);
}

}
