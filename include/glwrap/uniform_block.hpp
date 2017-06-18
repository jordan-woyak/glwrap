#pragma once

#include <map>
#include <list>
#include <memory>

#include "detail/context.hpp"

namespace GLWRAP_NAMESPACE
{

class program;

namespace detail
{

struct uniform_block_index {};
struct uniform_block_binding {};

}

template <typename T>
using uniform_block_location = detail::buffer_index<detail::uniform_block_index, T>;

class uniform_block_location_enumerator
{
public:
	// TODO: really need context?
	uniform_block_location_enumerator(context& _context)
		: m_current_index()
		, m_max_uniform_buffer_bindings()
	{
		if (is_extension_present(GL_ARB_uniform_buffer_object))
		{
			// TODO: this is the total per program.. should really return the total for each stage
			detail::gl_get(GL_MAX_COMBINED_UNIFORM_BLOCKS, &m_max_uniform_buffer_bindings);
		}
	}

	template <typename T>
	uniform_block_location<T> get()
	{
		if (m_current_index == m_max_uniform_buffer_bindings)
			throw exception(0);

		return uniform_block_location<T>(m_current_index++);
	}

private:
	int_t m_current_index;
	int_t m_max_uniform_buffer_bindings;
};

template <typename T>
using uniform_block_binding = detail::buffer_index<detail::uniform_block_binding, T>;

class uniform_block_binding_enumerator
{
public:
	// TODO: really need context?
	uniform_block_binding_enumerator(context& _context)
		: m_current_index()
		, m_max_index()
	{
		if (is_extension_present(GL_ARB_uniform_buffer_object))
		{
			detail::gl_get(GL_MAX_UNIFORM_BUFFER_BINDINGS, &m_max_index);
		}
	}

	template <typename T>
	uniform_block_binding<T> get()
	{
		if (m_current_index == m_max_index)
			throw exception(0);

		return uniform_block_binding<T>(m_current_index++);
	}

private:
	int_t m_current_index;
	int_t m_max_index;
};

// TODO: can shader storage and uniform block share the layout type?

// TODO: detail namespace
// TODO: name
template <typename T>
class uniform_block_layout
{
public:
	typedef uniform_block_location<T> location_type;

	uniform_block_layout(const location_type& _loc)
		: m_location(_loc)
	{}

	location_type get_location() const
	{
		return m_location;
	}

	std::string get_string() const
	{
		return "binding = " + std::to_string(m_location.get_index());
	}

private:
	location_type m_location;
};

}
