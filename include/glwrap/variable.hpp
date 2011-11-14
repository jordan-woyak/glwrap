#pragma once

#include "matrix.hpp"

#include "detail/variable.hpp"

namespace gl
{

class program;

class variable_base
{
	friend class program;

public:
	variable_base(const std::string& _name)
		: m_name(_name)
		, m_location()
	{}

	virtual std::string get_type_name() const = 0;
	virtual std::size_t get_index_count() const = 0;

	std::string const& get_name() const
	{
		return m_name;
	}

	GLuint get_location() const
	{
		return m_location;
	}

private:
	void set_location(GLuint _location)
	{
		m_location = _location;
	}

	std::string m_name;
	GLuint m_location;
};

template <typename T>
class variable : public variable_base
{
public:
	variable(const std::string& _name)
		: variable_base(_name)
	{}

	std::string get_type_name() const
	{
		return detail::variable::get_type_name<T>();
	}

	std::size_t get_index_count() const
	{
		return detail::variable::get_index_count<T>();
	}
};

}

