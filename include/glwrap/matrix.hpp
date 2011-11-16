#pragma once

#include "vector.hpp"

#include <eigen2/Eigen/Dense>

namespace gl
{

template <typename T, int Row, int Col>
class basic_matrix
{
public:
	basic_matrix()
	{
		m_matrix = m_matrix.Identity();
	}

	// TODO: I think this array is backwards, it is only working for NxN matrices
	// will need to fix set_uniform functions when fixing this
	basic_matrix(const std::array<std::array<T, Col>, Row>& arr)
	{
		std::copy_n(arr.data()->data(), Row * Col, m_matrix.data());
	}

	// TODO: assignment from another T?

	// multiplication
	// TODO: template the type?
	basic_matrix& operator*=(const basic_matrix& _rhs)
	{
		m_matrix *= _rhs.m_matrix;
		return *this;
	}

	template <typename TL, typename TR, int RowL, int ColLRowR, int ColR>
	friend basic_matrix<typename std::common_type<TL, TR>::type, RowL, ColR>
	operator*(basic_matrix<TL, RowL, ColLRowR> const& _lhs, basic_matrix<TR, ColLRowR, ColR> const& _rhs)
	{
		return {_lhs.m_matrix * _rhs.m_matrix};
	}

	// scalar
	// TODO: template the type?
	basic_matrix& operator*=(T const& _rhs)
	{
		m_matrix *= _rhs;
		return *this;
	}

	/*
	template <typename TL, typename TR, int RowL, int ColL>
	friend basic_matrix<typename std::common_type<TL, TR>::type, RowL, ColL>
	operator*(basic_matrix<TL, RowL, ColL> const& _lhs, TR const& _rhs)
	{
		return {_lhs.m_matrix * _rhs};
	}

	template <typename TL, typename TR, int RowR, int ColR>
	friend basic_matrix<typename std::common_type<TL, TR>::type, RowR, ColR>
	operator*(TR const& _lhs, basic_matrix<TL, RowR, ColR> const& _rhs)
	{
		return _rhs * _lhs;
	}
	*/

	// addition
	// TODO: template the type?
	basic_matrix& operator+=(basic_matrix const& _rhs)
	{
		m_matrix += _rhs.m_matrix;
		return *this;
	}

	template <typename TL, typename TR, int RowLR, int ColLR>
	friend basic_matrix<typename std::common_type<TL, TR>::type, RowLR, ColLR>
	operator+(basic_matrix<TL, RowLR, ColLR> const& _lhs, basic_matrix<TR, RowLR, ColLR> const& _rhs)
	{
		return {_lhs.m_matrix + _rhs.m_matrix};
	}

	const float_t* data() const
	{
		return m_matrix.data();
	}

private:
	basic_matrix(Eigen::Matrix<T, Row, Col> const& _mat)
		: m_matrix(_mat)
	{}

	Eigen::Matrix<T, Row, Col> m_matrix;
};

typedef basic_matrix<float_t, 2, 2> matrix2;
typedef basic_matrix<float_t, 2, 3> matrix2x3;
typedef basic_matrix<float_t, 2, 4> matrix2x4;
typedef basic_matrix<float_t, 3, 3> matrix3;
typedef basic_matrix<float_t, 3, 2> matrix3x2;
typedef basic_matrix<float_t, 3, 4> matrix3x4;
typedef basic_matrix<float_t, 4, 2> matrix4x2;
typedef basic_matrix<float_t, 4, 3> matrix4x3;
typedef basic_matrix<float_t, 4, 4> matrix4;

matrix4 rotate(float_t angle, float_t x, float_t y, float_t z)
{
	auto const c = std::cos(angle);
	auto const s = std::sin(angle);

	return
	{{{
		{{x * x * (1 - c) + c, x * y * (1 - c) - z * s, x * z * (1 - c) + y * s, 0}},
		{{x * y * (1 - c) + z * s, y * y * (1 - c) + c, y * z * (1 - c) - x * s, 0}},
		{{x * z * (1 - c) - y * s, z * y * (1 - c) + x * s, z * z * (1 - c) + c, 0}},
		{{0, 0, 0, 1}}
	}}};
}

matrix4 scale(float_t x, float_t y, float_t z)
{
	return
	{{{
		{{x, 0, 0, 0}},
		{{0, y, 0, 0}},
		{{0, 0, z, 0}},
		{{0, 0, 0, 1}}
	}}};
}

matrix4 translate(float_t x, float_t y, float_t z)
{
	return
	{{{
		{{1, 0, 0, x}},
		{{0, 1, 0, y}},
		{{0, 0, 1, z}},
		{{0, 0, 0, 1}}
	}}};
}

matrix4 ortho(float_t left, float_t right, float_t top, float_t bottom, float_t near, float_t far)
{
	return
	{{{
		{{2 / (right - left), 0, 0, (right + left) / (left - right)}},
		{{0, 2 / (top - bottom), 0, (top + bottom) / (bottom - top)}},
		{{0, 0, 2 / (near - far), (far + near) / (near - far)}},
		{{0, 0, 0, 1}}
	}}};
}

matrix4 frustum(float_t left, float_t right, float_t top, float_t bottom, float_t near, float_t far)
{
	auto const a = (right + left) / (right - left);
	auto const b = (top + bottom) / (top - bottom);
	auto const c = (far + near) / (far - near);
	auto const d = 2 * far * near / (far - near);

	return
	{{{
		{{2 * near / (right - left), 0, a, 0}},
		{{0, 2 * near / (top - bottom), b, 0}},
		{{0, 0, c, d}},
		{{0, 0, -1, 0}}
	}}};
}

}
