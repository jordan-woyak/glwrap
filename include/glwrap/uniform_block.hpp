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

struct uniform_block_index_traits
{
	static int_t get_index_count()
	{
		int_t val = 0;

		if (is_extension_present(GL_ARB_shader_storage_buffer_object))
		{
			// TODO: this is the total per program.. should really return the total for each stage
			detail::gl_get(GL_MAX_COMBINED_UNIFORM_BLOCKS, &val);
		}

		return val;
	}

	template <typename T>
	static int_t get_index_usage()
	{
		return 1;
	}
};

struct uniform_block_binding_traits
{
	static int_t get_index_count()
	{
		int_t val = 0;

		if (is_extension_present(GL_ARB_shader_storage_buffer_object))
		{
			detail::gl_get(GL_MAX_UNIFORM_BUFFER_BINDINGS, &val);
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
using uniform_block_location = detail::typed_index<int_t, detail::uniform_block_index_traits, T>;

typedef detail::typed_index_enumerator<detail::uniform_block_index_traits, uniform_block_location>
	uniform_block_location_enumerator;

template <typename T>
using uniform_block_binding = detail::buffer_index<detail::uniform_block_binding_traits, T>;

typedef detail::typed_index_enumerator<detail::uniform_block_binding_traits, uniform_block_binding>
	uniform_block_binding_enumerator;

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
		return "std140, binding = " + std::to_string(m_location.get_index());
	}

private:
	location_type m_location;
};

}
