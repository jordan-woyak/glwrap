#pragma once

#include <list>

#include "variable.hpp"
#include "texture.hpp"
#include "detail/uniform.hpp"

namespace gl
{

class program;

template <typename T>
class uniform;

class uniform_location_enumerator;

template <typename T>
class uniform_location
{
	friend class uniform_location_enumerator;

public:
	int_t get_index() const
	{
		return m_index;
	}

private:
	uniform_location(int_t _index)
		: m_index(_index)
	{}

	int_t m_index;
};

// TODO: name?
class uniform_location_enumerator
{
public:
	template <typename T>
	using location_type = uniform_location<T>;
	
	// TODO: really need context?
	uniform_location_enumerator(context& _context)
		: m_current_index()
		, m_max_uniform_locations()
	{
		detail::gl_get(GL_MAX_UNIFORM_LOCATIONS, &m_max_uniform_locations);
	}

	template <typename T>
	uniform_location<T> get()
	{
		if (m_current_index >= m_max_uniform_locations)
			throw exception(0);
			
		uniform_location<T> ind(m_current_index);

		// TODO: every uniform takes up just one location, right?
		m_current_index += 1;

		return ind;
	}

private:
	int_t m_current_index;
	int_t m_max_uniform_locations;
};

}
