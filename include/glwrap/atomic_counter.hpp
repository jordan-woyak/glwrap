#pragma once

#include "texture.hpp"
#include "detail/uniform.hpp"

namespace gl
{

template <typename T>
class uniform;

class atomic_counter_binding_enumerator;

template <typename T>
class atomic_counter_binding
{
	friend class uniform_location_enumerator;

public:
	int_t get_index() const
	{
		return m_index;
	}

private:
	atomic_counter_binding(int_t _index)
		: m_index(_index)
	{}

	int_t m_index;
};

// TODO: name?
class atomic_counter_binding_enumerator
{
public:
	template <typename T>
	using location_type = atomic_counter_binding<T>;
	
	// TODO: really need context?
	atomic_counter_binding_enumerator(context& _context)
		: m_current_index()
		, m_max_uniform_locations()
	{
		// TODO: correct parameter?
		detail::gl_get(GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS, &m_max_bindings);
	}

	template <typename T>
	uniform_location<T> get()
	{
		if (m_current_index >= m_max_uniform_locations)
			throw exception(0);
			
		uniform_location<T> ind(m_current_index);

		m_current_index += detail::uniform_location_count<T>::value;

		return ind;
	}

private:
	int_t m_current_index;
	int_t m_max_bindings;
};



}

