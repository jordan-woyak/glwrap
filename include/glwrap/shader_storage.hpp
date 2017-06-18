#pragma once

#include "detail/context.hpp"

namespace GLWRAP_NAMESPACE
{

namespace detail
{

// TODO: rename everything shader_storage_block_*
struct shader_storage_index
{
	static int_t get_index_count()
	{
		int_t val = 0;

		if (is_extension_present(GL_ARB_shader_storage_buffer_object))
		{
			// TODO: this is the total per program.. should really return the total for each stage
			detail::gl_get(GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS, &val);
		}

		return val;
	}

	template <typename T>
	static int_t get_index_usage()
	{
		return 1;
	}
};

struct shader_storage_binding
{
	static int_t get_index_count()
	{
		int_t val = 0;

		if (is_extension_present(GL_ARB_shader_storage_buffer_object))
		{
			detail::gl_get(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &val);
		}

		return val;
	}

	template <typename T>
	static int_t get_index_usage()
	{
		return 1;
	}
};

}

template <typename T>
using shader_storage_location = detail::buffer_index<detail::shader_storage_index, T>;

typedef detail::typed_index_enumerator<detail::shader_storage_index, shader_storage_location>
	shader_storage_location_enumerator;

template <typename T>
using shader_storage_binding = detail::buffer_index<detail::shader_storage_binding, T>;

typedef detail::typed_index_enumerator<detail::shader_storage_binding, shader_storage_binding>
	shader_storage_binding_enumerator;

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
