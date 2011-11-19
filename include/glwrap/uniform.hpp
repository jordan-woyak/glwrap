#pragma once

#include <list>

#include "array_buffer.hpp"
#include "variable.hpp"
#include "texture.hpp"

namespace gl
{

class program;

namespace detail
{

template <typename T>
struct uniform_value
{
	typedef T type;
};

// TODO: try to eliminate this
template <>
struct uniform_value<texture_2d>
{
	typedef texture_unit<texture_2d> type;
};

}

template <typename T>
class uniform;

class uniform_variable
{
	friend class program;

	template <typename T>
	friend class uniform;

private:
	template <typename T>
	uniform_variable(T&& _var)
		: m_var(new T(_var))
	{}

	GLuint get_location() const
	{
		return m_location;
	}

	void set_location(GLuint _location)
	{
		m_location = _location;
	}

	variable_base& get_variable()
	{
		return *m_var;
	}

	std::unique_ptr<variable_base> m_var;
	uint_t m_location;
};

template <typename T>
class uniform
{
	friend class program;

private:
	typedef std::list<uniform_variable>::iterator iter_t;

	uniform(iter_t _iter)
		: m_iter(_iter)
	{}

	uint_t get_location() const
	{
		return m_iter->get_location();
	}

	void set_value(typename detail::uniform_value<T>::type const&);

	iter_t m_iter;
};

// float
template <>
void uniform<float_t>::set_value(const float_t& _value)
{
	glUniform1f(get_location(), _value);
}

// int
template <>
void uniform<int_t>::set_value(const int_t& _value)
{
	glUniform1i(get_location(), _value);
}

// vec2
template <>
void uniform<fvec2>::set_value(const fvec2& _value)
{
	glUniform2fv(get_location(), 1, _value.data());
}

// vec3
template <>
void uniform<fvec3>::set_value(const fvec3& _value)
{
	glUniform3fv(get_location(), 1, _value.data());
}

// vec4
template <>
void uniform<fvec4>::set_value(const fvec4& _value)
{
	glUniform4fv(get_location(), 1, _value.data());
}

// matrix4
template <>
void uniform<matrix4>::set_value(const matrix4& _value)
{
	glUniformMatrix4fv(get_location(), 1, GL_TRUE, _value.data());
}

template <>
void uniform<texture_2d>::set_value(const texture_unit<texture_2d>& _value)
{
	glUniform1i(get_location(), _value.get_index());
}

// TODO: rest of matrix types
// TODO: arrays

}

