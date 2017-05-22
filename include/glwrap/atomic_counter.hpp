#pragma once

namespace GLWRAP_NAMESPACE
{

namespace detail
{

struct atomic_counter_index
{};

}

// TODO: name
template <typename T>
using atomic_counter_binding = detail::buffer_index<detail::atomic_counter_index, T>;

// TODO: ugly
template <typename T>
using atomic_counter_binding_attribute = detail::buffer_index_attribute<detail::atomic_counter_index, T>;

// TODO: name?
class atomic_counter_binding_enumerator
{
public:
	template <typename T>
	using location_type = atomic_counter_binding<T>;
	
	// TODO: really need context?
	atomic_counter_binding_enumerator(context& _context)
		: m_current_index()
		, m_max_bindings()
	{
		// TODO: correct parameter?
		detail::gl_get(GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS, &m_max_bindings);
	}

	template <typename T>
	location_type<T> get()
	{
		if (m_current_index >= m_max_bindings)
			throw exception(0);
			
		location_type<T> ind(m_current_index);

		++m_current_index;

		return ind;
	}

private:
	int_t m_current_index;
	int_t m_max_bindings;
};

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

