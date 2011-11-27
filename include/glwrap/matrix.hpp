#pragma once

#include <algorithm>

#include <Eigen/Dense>

#include "vector.hpp"

namespace gl
{

template <typename T, int Row, int Col>
class basic_mat
{
public:
	basic_mat()
	{
		m_matrix = m_matrix.Identity();
	}

	// TODO: I think this array is backwards, it is only working for NxN matrices
	// will need to fix set_uniform functions when fixing this
	basic_mat(const basic_vec<basic_vec<T, Col>, Row>& _arr)
	{
		std::copy_n(_arr.data()->data(), Row * Col, m_matrix.data());
	}

	// TODO: assignment from another T?

	// multiplication
	// TODO: template the type?
	basic_mat& operator*=(const basic_mat& _rhs)
	{
		m_matrix *= _rhs.m_matrix;
		return *this;
	}

	template <typename TL, typename TR, int RowL, int ColLRowR, int ColR>
	friend basic_mat<typename std::common_type<TL, TR>::type, RowL, ColR>
	operator*(basic_mat<TL, RowL, ColLRowR> const& _lhs, basic_mat<TR, ColLRowR, ColR> const& _rhs);

	// scalar
	// TODO: template the type?
	basic_mat& operator*=(T const& _rhs)
	{
		m_matrix *= _rhs;
		return *this;
	}

	/*
	template <typename TL, typename TR, int RowL, int ColL>
	friend basic_mat<typename std::common_type<TL, TR>::type, RowL, ColL>
	operator*(basic_mat<TL, RowL, ColL> const& _lhs, TR const& _rhs)
	{
		return {_lhs.m_matrix * _rhs};
	}

	template <typename TL, typename TR, int RowR, int ColR>
	friend basic_mat<typename std::common_type<TL, TR>::type, RowR, ColR>
	operator*(TR const& _lhs, basic_mat<TL, RowR, ColR> const& _rhs)
	{
		return _rhs * _lhs;
	}
	*/

	// addition
	// TODO: template the type?
	basic_mat& operator+=(basic_mat const& _rhs)
	{
		m_matrix += _rhs.m_matrix;
		return *this;
	}

	template <typename TL, typename TR, int RowLR, int ColLR>
	friend basic_mat<typename std::common_type<TL, TR>::type, RowLR, ColLR>
	operator+(basic_mat<TL, RowLR, ColLR> const& _lhs, basic_mat<TR, RowLR, ColLR> const& _rhs);

	const T* data() const
	{
		return m_matrix.data();
	}

private:
	basic_mat(Eigen::Matrix<T, Row, Col> const& _mat)
		: m_matrix(_mat)
	{}

	Eigen::Matrix<T, Row, Col> m_matrix;
};

template <typename TL, typename TR, int RowL, int ColLRowR, int ColR>
basic_mat<typename std::common_type<TL, TR>::type, RowL, ColR>
operator*(basic_mat<TL, RowL, ColLRowR> const& _lhs, basic_mat<TR, ColLRowR, ColR> const& _rhs)
{
	return {_lhs.m_matrix * _rhs.m_matrix};
}

template <typename TL, typename TR, int RowLR, int ColLR>
basic_mat<typename std::common_type<TL, TR>::type, RowLR, ColLR>
operator+(basic_mat<TL, RowLR, ColLR> const& _lhs, basic_mat<TR, RowLR, ColLR> const& _rhs)
{
	return {_lhs.m_matrix + _rhs.m_matrix};
}

typedef basic_mat<float_t, 2, 2> mat2x2, mat2;
typedef basic_mat<float_t, 2, 3> mat2x3;
typedef basic_mat<float_t, 2, 4> mat2x4;
typedef basic_mat<float_t, 3, 3> mat3x3, mat3;
typedef basic_mat<float_t, 3, 2> mat3x2;
typedef basic_mat<float_t, 3, 4> mat3x4;
typedef basic_mat<float_t, 4, 2> mat4x2;
typedef basic_mat<float_t, 4, 3> mat4x3;
typedef basic_mat<float_t, 4, 4> mat4x4, mat4;

typedef basic_mat<double_t, 2, 2> dmat2x2, dmat2;
typedef basic_mat<double_t, 2, 3> dmat2x3;
typedef basic_mat<double_t, 2, 4> dmat2x4;
typedef basic_mat<double_t, 3, 3> dmat3x3, dmat3;
typedef basic_mat<double_t, 3, 2> dmat3x2;
typedef basic_mat<double_t, 3, 4> dmat3x4;
typedef basic_mat<double_t, 4, 2> dmat4x2;
typedef basic_mat<double_t, 4, 3> dmat4x3;
typedef basic_mat<double_t, 4, 4> dmat4x4, dmat4;

mat4 rotate(float_t angle, float_t x, float_t y, float_t z)
{
	auto const c = std::cos(angle);
	auto const s = std::sin(angle);

	return
	{{
		{x * x * (1 - c) + c, x * y * (1 - c) - z * s, x * z * (1 - c) + y * s, 0},
		{x * y * (1 - c) + z * s, y * y * (1 - c) + c, y * z * (1 - c) - x * s, 0},
		{x * z * (1 - c) - y * s, z * y * (1 - c) + x * s, z * z * (1 - c) + c, 0},
		{0, 0, 0, 1}
	}};
}

mat4 scale(float_t x, float_t y, float_t z)
{
	return
	{{
		{x, 0, 0, 0},
		{0, y, 0, 0},
		{0, 0, z, 0},
		{0, 0, 0, 1}
	}};
}

mat4 translate(float_t x, float_t y, float_t z)
{
	return
	{{
		{1, 0, 0, x},
		{0, 1, 0, y},
		{0, 0, 1, z},
		{0, 0, 0, 1}
	}};
}

mat4 ortho(float_t left, float_t right, float_t top, float_t bottom, float_t near, float_t far)
{
	return
	{{
		{2 / (right - left), 0, 0, (right + left) / (left - right)},
		{0, 2 / (top - bottom), 0, (top + bottom) / (bottom - top)},
		{0, 0, 2 / (near - far), (far + near) / (near - far)},
		{0, 0, 0, 1}
	}};
}

mat4 frustum(float_t left, float_t right, float_t top, float_t bottom, float_t near, float_t far)
{
	auto const a = (right + left) / (right - left);
	auto const b = (top + bottom) / (top - bottom);
	auto const c = (far + near) / (far - near);
	auto const d = 2 * far * near / (far - near);

	return
	{{
		{2 * near / (right - left), 0, a, 0},
		{0, 2 * near / (top - bottom), b, 0},
		{0, 0, c, d},
		{0, 0, -1, 0}
	}};
}

// TODO: should this be here?
// TODO: I don't like fov being in degrees
mat4 perspective(float_t fovy, float_t aspect, float_t znear, float_t zfar)
{
	auto const f = 1 / std::tan(fovy * std::atan(1.0f) / 90);
	auto const nf = znear - zfar;

	return
	{{
		{f / aspect, 0, 0, 0},
		{0, f, 0, 0},
		{0, 0, (zfar + znear) / nf, 2 * zfar * znear / nf},
		{0, 0, -1, 0}
	}};
}

}
