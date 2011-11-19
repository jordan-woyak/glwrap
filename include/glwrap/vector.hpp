#pragma once

#include <GL/glew.h>

#include <array>

#include "util.hpp"

namespace gl
{

template <typename T, int D>
class basic_vec;

template <typename T>
class basic_vec<T, 2>
{
public:
	T x, y;

	static const int dims = 2;

	// TODO: silly
	T* data()
	{ return &x; }

	const T* data() const
	{ return &x; }

	basic_vec()
		: x()
		, y()
	{}

	basic_vec(T _x, T _y)
		: x(_x)
		, y(_y)
	{}
};

template <typename T>
class basic_vec<T, 3> : private basic_vec<T, 2>
{
public:
	using basic_vec<T, 2>::x;
	using basic_vec<T, 2>::y;

	static const int dims = 3;

	basic_vec()
		: basic_vec<T, 2>()
		, z()
	{}

	basic_vec(T _x, T _y, T _z)
		: basic_vec<T, 2>(_x, _y)
		, z(_z)
	{}

	using basic_vec<T, 2>::data;

	T z;
};

template <typename T>
class basic_vec<T, 4> : private basic_vec<T, 3>
{
public:
	using basic_vec<T, 3>::x;
	using basic_vec<T, 3>::y;
	using basic_vec<T, 3>::z;

	static const int dims = 4;

	basic_vec()
		: basic_vec<T, 3>()
		, w()
	{}

	basic_vec(T _x, T _y, T _z, T _w)
		: basic_vec<T, 3>(_x, _y, _z)
		, w(_w)
	{}

	using basic_vec<T, 3>::data;

	T w;
};

// TODO: need template aliases :(
typedef basic_vec<float_t, 2> fvec2;
typedef basic_vec<float_t, 3> fvec3;
typedef basic_vec<float_t, 4> fvec4;

typedef basic_vec<int_t, 2> ivec2;
typedef basic_vec<int_t, 3> ivec3;
typedef basic_vec<int_t, 4> ivec4;

template <typename T, int D>
basic_vec<T, D>& operator*=(const basic_vec<T, D>& _vec1, const basic_vec<T, D>& _vec2)
{
	// TODO: recursive template magic
	for (int i = 0; i != D; ++i)
		_vec1[i] *= _vec2[i];

	return _vec1;
}

template <typename T, int D>
basic_vec<T, D> operator*(basic_vec<T, D> _vec1, const basic_vec<T, D>& _vec2)
{
	return _vec1 *= _vec2;
}

template <int D>
float_t dot(const basic_vec<float_t, D>& _vec1, const basic_vec<float_t, D> & _vec2)
{
	float_t result = 0;

	// TODO: recursive template magic
	for (int i = 0; i != D; ++i)
		result += _vec1.data()[i] * _vec2.data()[i];

	return result;
}

}

