#pragma once

#include "array_buffer.hpp"
#include "variable.hpp"

namespace gl
{

class program;

template <typename T>
class uniform
{
	friend class program;

private:
	typedef std::list<std::unique_ptr<variable_base>>::iterator iter_t;

	uniform(iter_t _iter)
		: m_iter(_iter)
	{}

	iter_t m_iter;

	void set_value(const T&);
};

// TODO: ugly
template <>
class uniform<sampler_2d>
{
	friend class program;

private:
	typedef std::list<std::unique_ptr<variable_base>>::iterator iter_t;

	uniform(iter_t _iter)
		: m_iter(_iter)
	{}

	iter_t m_iter;

	void set_value(const bound_texture<2>& _value)
	{
		glUniform1i((*m_iter)->get_location(), _value.get_unit());
	}
};

// float
template <>
void uniform<float_t>::set_value(const float_t& _value)
{
	glUniform1f((*m_iter)->get_location(), _value);
}

// int
template <>
void uniform<int_t>::set_value(const int_t& _value)
{
	glUniform1i((*m_iter)->get_location(), _value);
}

// vec2
template <>
void uniform<fvec2>::set_value(const fvec2& _value)
{
	glUniform2fv((*m_iter)->get_location(), 1, _value.data());
}

// vec3
template <>
void uniform<fvec3>::set_value(const fvec3& _value)
{
	glUniform3fv((*m_iter)->get_location(), 1, _value.data());
}

// vec4
template <>
void uniform<fvec4>::set_value(const fvec4& _value)
{
	glUniform4fv((*m_iter)->get_location(), 1, _value.data());
}

// matrix4
template <>
void uniform<matrix4>::set_value(const matrix4& _value)
{
	glUniformMatrix4fv((*m_iter)->get_location(), 1, GL_TRUE, _value.data());
}

// TODO: rest of matrix types
// TODO: arrays

}

