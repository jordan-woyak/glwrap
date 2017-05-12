#pragma once

#include "matrix.hpp"

#include "detail/variable.hpp"

namespace gl
{

class program;

namespace detail
{

class variable_base
{
	friend class gl::program;

public:
	variable_base(const std::string& _name)
		: m_name(_name)
	{}

	virtual std::string get_glsl_definition() const = 0;

	std::string const& get_name() const
	{
		return m_name;
	}

private:
	std::string m_name;
};

template <typename T>
class variable : public variable_base
{
public:
	variable(const std::string& _name)
		: variable_base(_name)
	{}

	std::string get_glsl_definition() const
	{
		return
			detail::glslvar::get_type_name<T>()
			+ " " + get_name()
			+ detail::glslvar::glsl_var_suffix<T>::suffix();
	}
};

}

}

