#pragma once

namespace GLWRAP_NAMESPACE
{

namespace detail
{

struct atomic_counter_index
{
	static int_t get_index_count()
	{
		int_t val = 0;

		if (is_extension_present(GL_ARB_shader_storage_buffer_object))
		{
			// TODO: this is the max per program
			detail::gl_get(GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS, &val);
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

// TODO: name
template <typename T>
using atomic_counter_binding = detail::buffer_index_attribute<detail::atomic_counter_index, T>;

// TODO: ugly, kill one of these
template <typename T>
using atomic_counter_binding_attribute = detail::buffer_index_attribute<detail::atomic_counter_index, T>;

// TODO: name?
typedef detail::typed_index_enumerator<detail::atomic_counter_index, atomic_counter_binding>
	atomic_counter_binding_enumerator;

template <typename T>
class atomic_counter_layout
{
public:
	typedef void location_type;

	atomic_counter_layout(const atomic_counter_binding_attribute<T>& _attrib)
		: m_attrib(_attrib)
	{}

	void get_location() const
	{}

	std::string get_string() const
	{
		// TODO: optional if -1
		return "binding = " + std::to_string(m_attrib.get_index())
			+ ", offset = " + std::to_string(m_attrib.get_offset());
	}

private:
	atomic_counter_binding_attribute<T> m_attrib;
};

}

