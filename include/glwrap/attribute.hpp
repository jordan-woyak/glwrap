#pragma once

#include "variable.hpp"
#include "detail/attribute.hpp"

#include <list>

namespace gl
{

class program;

class attribute_location_enumerator;

template <typename T>
class attribute_location
{
	friend class attribute_location_enumerator;

	static_assert(detail::is_valid_attrib_type<T>::value, "Invalid Attrib Type");

public:
	int_t get_index() const
	{
		return m_index;
	}

private:
	attribute_location(int_t _index)
		: m_index(_index)
	{}

	int_t m_index;
};

// TODO: name?
class attribute_location_enumerator
{
public:
	template <typename T>
	using location_type = attribute_location<T>;

	// TODO: really need context?
	attribute_location_enumerator(context& _context)
		: m_current_index()
		, m_max_vertex_attribs()
	{
		detail::gl_get(GL_MAX_VERTEX_ATTRIBS, &m_max_vertex_attribs);
	}

	template <typename T>
	location_type<T> get()
	{
		location_type<T> ind(m_current_index);
		m_current_index += detail::attrib_index_count<T>::value;

		if (m_current_index > m_max_vertex_attribs)
			throw exception(0);

		return ind;
	}

private:
	int_t m_current_index;
	int_t m_max_vertex_attribs;
};

}

