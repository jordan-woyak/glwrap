#pragma once

#include "vector.hpp"

#include <eigen2/Eigen/Dense>

namespace gl
{

template <int Row, int Col>
class matrix
{
public:
	matrix()
	{
		m_matrix = m_matrix.Identity();
	}

	matrix(const std::array<std::array<float_t, Col>, Row>& arr)
	{
		std::copy_n(arr.data()->data(), Row * Col, m_matrix.data());
	}

	matrix& operator*=(const matrix& _rhs)
	{
		m_matrix *= _rhs.m_matrix;
		return *this;
	}

	const float_t* data() const
	{
		return m_matrix.data();
	}

private:
	Eigen::Matrix<float_t, Row, Col> m_matrix;
};

typedef matrix<2, 2> matrix2;
typedef matrix<2, 3> matrix2x3;
typedef matrix<2, 4> matrix2x4;
typedef matrix<3, 3> matrix3;
typedef matrix<3, 2> matrix3x2;
typedef matrix<3, 4> matrix3x4;
typedef matrix<4, 2> matrix4x2;
typedef matrix<4, 3> matrix4x3;
typedef matrix<4, 4> matrix4;

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
