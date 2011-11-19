#pragma once

#include "variable.hpp"
#include "array_buffer.hpp"

#include <list>

namespace gl
{

class program;

class attribute_location_alloter;

// TODO: rename attribute_location?
template <typename T>
class attribute_location
{
	friend class attribute_location_alloter;

public:
	uint_t get_index() const
	{
		return m_index;
	}

private:
	attribute_location(uint_t _index)
		: m_index(_index)
	{}

	uint_t m_index;
};

// TODO: name?
class attribute_location_alloter
{
public:
	// TODO: really need context?
	attribute_location_alloter(context& _context)
		: m_current_index()
		, m_max_vertex_attribs()
	{
		GLint max_attribs{};
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_attribs);
		m_max_vertex_attribs = max_attribs;
	}

	template <typename T>
	attribute_location<T> allot()
	{
		attribute_location<T> ind(m_current_index);
		m_current_index += detail::variable::get_index_count<T>();

		if (m_current_index > m_max_vertex_attribs)
			throw exception();

		return ind;
	}

private:
	uint_t m_current_index;
	uint_t m_max_vertex_attribs;
};

template <typename T>
class attribute
{
	friend class program;
	friend class vertex_array;

public:
	std::string const& get_name() const
	{
		return (*m_iter)->get_name();
	}

private:
	typedef std::list<std::unique_ptr<variable_base>>::iterator iter_t;

	attribute(iter_t _iter)
		: m_iter(_iter)
	{}

	iter_t m_iter;
};

}

