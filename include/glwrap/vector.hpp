#pragma once

#include <GL/glew.h>

#include <array>

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
typedef GLsizei size_t;

template <typename T, int D>
class basic_vec;

template <>
class basic_vec<float_t, 2>
{
public:
	float_t x, y;

	// TODO: silly
	float_t* data()
	{ return &x; }

	const float_t* data() const
	{ return &x; }

	basic_vec()
		: x()
		, y()
	{}

	basic_vec(float_t _x, float_t _y)
		: x(_x)
		, y(_y)
	{}
};

template <>
class basic_vec<float_t, 3> : private basic_vec<float_t, 2>
{
public:
	using basic_vec<float_t, 2>::x;
	using basic_vec<float_t, 2>::y;

	basic_vec()
		: basic_vec<float_t, 2>()
		, z()
	{}

	basic_vec(float_t _x, float_t _y, float_t _z)
		: basic_vec<float_t, 2>(_x, _y)
		, z(_z)
	{}

	using basic_vec<float_t, 2>::data;

	float_t z;
};

template <>
class basic_vec<float_t, 4> : private basic_vec<float_t, 3>
{
public:
	using basic_vec<float_t, 3>::x;
	using basic_vec<float_t, 3>::y;
	using basic_vec<float_t, 3>::z;

	basic_vec()
		: basic_vec<float_t, 3>()
		, w()
	{}

	basic_vec(float_t _x, float_t _y, float_t _z, float_t _w)
		: basic_vec<float_t, 3>(_x, _y, _z)
		, w(_w)
	{}

	using basic_vec<float_t, 3>::data;

	float_t w;
};

// template aliases
//template <int D>
//using vec = basic_vec<float_t, D>;
//using ivec = basic_vec<int_t, D>;
//using bvec = basic_vec<bool_t, D>;
//
//typedef vec<2> vec2;
//typedef vec<3> vec3;
//typedef vec<4> vec4;
//
//typedef ivec<2> ivec2;
//typedef ivec<3> ivec3;
//typedef ivec<4> ivec4;
//
//typedef bvec<2> bvec2;
//typedef bvec<3> bvec3;
//typedef bvec<4> bvec4;

typedef basic_vec<float_t, 2> fvec2;
typedef basic_vec<float_t, 3> fvec3;
typedef basic_vec<float_t, 4> fvec4;

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

