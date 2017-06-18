#pragma once

#include "detail/attribute.hpp"
#include "detail/variable.hpp"
#include "detail/context.hpp"

#include <list>

namespace GLWRAP_NAMESPACE
{

namespace detail
{

struct attribute_index
{
	static int_t get_index_count()
	{
		int_t val = 0;

		detail::gl_get(GL_MAX_VERTEX_ATTRIBS, &val);

		return val;
	}

	template <typename T>
	static int_t get_index_usage()
	{
		// TODO: put this elsewhere?
		static_assert(detail::is_valid_attrib_type<T>::value, "Invalid Attrib Type");

		return detail::variable_traits<T>::attrib_index_count;

		// TODO: keep track of the attrib_resource_count also.
	}
};

}

template <typename T>
class attribute_location
{
public:
	int_t get_index() const
	{
		return m_index;
	}

// TODO: make more explicit perhaps..
//private:
	attribute_location(int_t _index)
		: m_index(_index)
	{}

	int_t m_index;
};

// TODO: name?
typedef detail::typed_index_enumerator<detail::attribute_index, attribute_location>
	attribute_location_enumerator;

template <typename T>
class attribute_layout
{
public:
	typedef attribute_location<T> location_type;

	attribute_layout(const location_type& _loc)
		: m_location(_loc)
	{}

	location_type get_location() const
	{
		return m_location;
	}

	std::string get_string() const
	{
		// TODO: optional if -1
		return "location = " + std::to_string(m_location.get_index());
	}

private:
	location_type m_location;
};

}

