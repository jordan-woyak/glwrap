#pragma once

#include <list>

#include "variable.hpp"
#include "texture.hpp"
#include "detail/uniform.hpp"

namespace gl
{

class program;

template <typename T>
class uniform;

namespace detail
{

class uniform_variable
{
	friend class gl::program;

	template <typename T>
	friend class gl::uniform;

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

}

template <typename T>
class uniform
{
	friend class program;

private:
	typedef std::list<detail::uniform_variable>::iterator iter_t;

	uniform(iter_t _iter)
		: m_iter(_iter)
	{}

	uint_t get_location() const
	{
		return m_iter->get_location();
	}

	void set_value(typename detail::uniform_value<T>::type const& _value)
	{
		detail::set_uniform_value(get_location(), _value);
	}

	iter_t m_iter;
};

}

