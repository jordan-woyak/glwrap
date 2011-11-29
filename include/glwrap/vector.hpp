#pragma once

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
	static const int dims = 2;

	// TODO: silly
	T* data()
	{ return &x; }

	const T* data() const
	{ return &x; }

	basic_vec()
		: x(), y()
	{}

	basic_vec(T _x, T _y)
		: x(_x), y(_y)
	{}

	T x, y;
};

template <typename T>
class basic_vec<T, 3>
{
public:
	static const int dims = 3;

	// TODO: silly
	T* data()
	{ return &x; }

	const T* data() const
	{ return &x; }

	basic_vec()
		: x(), y(), z()
	{}

	basic_vec(T _x, T _y, T _z)
		: x(_x), y(_y), z(_z)
	{}

	T x, y, z;
};

template <typename T>
class basic_vec<T, 4>
{
public:
	static const int dims = 4;

	// TODO: silly
	T* data()
	{ return &x; }

	const T* data() const
	{ return &x; }

	basic_vec()
		: x(), y(), z(), w()
	{}

	basic_vec(T _x, T _y, T _z, T _w)
		: x(_x), y(_y), z(_z), w(_w)
	{}

	T x, y, z, w;
};

typedef basic_vec<float_t, 2> vec2;
typedef basic_vec<float_t, 3> vec3;
typedef basic_vec<float_t, 4> vec4;

typedef basic_vec<int_t, 2> ivec2;
typedef basic_vec<int_t, 3> ivec3;
typedef basic_vec<int_t, 4> ivec4;

typedef basic_vec<uint_t, 2> uvec2;
typedef basic_vec<uint_t, 3> uvec3;
typedef basic_vec<uint_t, 4> uvec4;

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

