#pragma once

#include "detail/context.hpp"

namespace gl
{

namespace detail
{

struct shader_storage_index
{};

}

template <typename T>
using shader_storage_location = detail::buffer_index<detail::shader_storage_index, T>;

class shader_storage_location_enumerator
{
public:
	// TODO: really need context?
	shader_storage_location_enumerator(context& _context)
		: m_current_index()
		//, m_max_uniform_buffer_bindings()
	{
		// TODO: correct parameter?
		//detail::gl_get(GL_MAX_UNIFORM_BUFFER_BINDINGS, &m_max_uniform_buffer_bindings);
	}

	template <typename T>
	shader_storage_location<T> get()
	{
		//if (m_current_index == m_max_uniform_buffer_bindings)
			//throw exception(0);

		return shader_storage_location<T>(m_current_index++);
	}

private:
	int_t m_current_index;
	//int_t m_max_bindings;
};

// TODO: detail namespace
// TODO: name
template <typename T>
class shader_storage_layout
{
public:
	typedef shader_storage_location<T> location_type;

	shader_storage_layout(const location_type& _loc)
		: m_location(_loc)
	{}

	location_type get_location() const
	{
		return m_location;
	}

	std::string get_string() const
	{
		return "std430, binding = " + std::to_string(m_location.get_index());
	}

private:
	location_type m_location;
};

}
