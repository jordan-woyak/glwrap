#pragma once

#include "matrix.hpp"

#include "detail/variable.hpp"

namespace gl
{

template <typename T, typename L>
class variable_description
{
public:
	typedef T value_type;
	typedef L location_type;

	variable_description(std::string _name, const location_type& _loc)
		: m_name(std::move(_name))
		, m_location(_loc)
	{}

	std::string const& get_name() const
	{
		return m_name;
	}

	location_type const& get_location() const
	{
		return m_location;
	}

private:
	std::string m_name;
	location_type m_location;
};

template <typename T, typename L>
auto variable(std::string _name, L& _enumerator) -> variable_description<T, typename L::template location_type<T>>
{
	return {std::move(_name), _enumerator.template get<T>()};
}

/*
// TODO: templatify all the location types so this can assume the variable type from the location
template <typename T, typename L>
auto variable(std::string _name, L& _loc) -> variable_description<T, L>
{
	return {std::move(_name), _loc};
}
*/

}

