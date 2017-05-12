#pragma once

#include "variable.hpp"

#include <list>

namespace gl
{

class program;

class attribute_location_enumerator;

// TODO: rename attribute_location?
template <typename T>
class attribute_location
{
	friend class attribute_location_enumerator;

public:
	int_t get_begin_index() const
	{
		return m_index;
	}

	int_t get_end_index() const
	{
		return m_index + detail::glslvar::index_count<T>::value;
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
	// TODO: really need context?
	attribute_location_enumerator(context& _context)
		: m_current_index()
		, m_max_vertex_attribs()
	{
		detail::gl_get(GL_MAX_VERTEX_ATTRIBS, &m_max_vertex_attribs);
	}

	template <typename T>
	attribute_location<T> get()
	{
		attribute_location<T> ind(m_current_index);
		m_current_index += detail::glslvar::index_count<T>::value;

		if (m_current_index > m_max_vertex_attribs)
			throw exception(0);

		return ind;
	}

private:
	int_t m_current_index;
	int_t m_max_vertex_attribs;
};

template <typename T>
class attribute
{
	friend class shader<shader_type::vertex>;

public:
	std::string const& get_name() const
	{
		return (*m_iter)->get_name();
	}

private:
	typedef std::list<std::unique_ptr<detail::variable_base>>::iterator iter_t;

	attribute(iter_t _iter)
		: m_iter(_iter)
	{}

	iter_t m_iter;
};

}

