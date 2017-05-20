#pragma once

#include "matrix.hpp"

#include "detail/variable.hpp"

#include "attribute.hpp"
//#include "transform_feedback.hpp"
#include "uniform.hpp"

namespace gl
{

template <typename T, typename L>
class variable_description
{
public:
	typedef T value_type;
	typedef L layout_type;

	variable_description(std::string _name, const layout_type& _layout)
		: m_name(std::move(_name))
		, m_layout(_layout)
	{}

	std::string const& get_name() const
	{
		return m_name;
	}

	layout_type const& get_layout() const
	{
		return m_layout;
	}

private:
	std::string m_name;
	layout_type m_layout;
};

template <typename T>
auto variable(std::string _name, attribute_location_enumerator& _enum) -> variable_description<T, attribute_layout<T>>
{
	return {std::move(_name), _enum.template get<T>()};
}

template <typename T>
auto variable(std::string _name, uniform_location_enumerator& _enum) -> variable_description<T, uniform_layout<T>>
{
	return {std::move(_name), _enum.template get<T>()};
}

/*
template <typename T>
auto variable(std::string _name, uniform_location_enumerator& _enum) -> variable_description<T, uniform_layout<T>>
{
	return {std::move(_name), _enum.template get<T>()};
}
*/

/*
// TODO: templatify all the location types so this can assume the variable type from the location
template <typename T, typename L>
auto variable(std::string _name, L& _loc) -> variable_description<T, L>
{
	return {std::move(_name), _loc};
}
*/

}

