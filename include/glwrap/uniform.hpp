#pragma once

#include "detail/uniform.hpp"

namespace gl
{

template <typename T>
class uniform;

class uniform_location_enumerator;

template <typename T>
class uniform_location
{
	friend class uniform_location_enumerator;

	static_assert(detail::is_valid_uniform_type<T>::value, "Invalid Uniform Type");

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
		// TODO: correct parameter?
		detail::gl_get(GL_MAX_UNIFORM_LOCATIONS, &m_max_uniform_locations);
	}

	template <typename T>
	location_type<T> get()
	{
		if (m_current_index >= m_max_uniform_locations)
			throw exception(0);
			
		location_type<T> ind(m_current_index);

		m_current_index += detail::uniform_location_count<T>::value;

		return ind;
	}

private:
	int_t m_current_index;
	int_t m_max_uniform_locations;
};

template <typename T>
class uniform_layout
{
public:
	typedef uniform_location<T> location_type;

	uniform_layout(const location_type& _loc)
		: m_location(_loc)
	{}

	location_type get_location() const
	{
		return m_location;
	}

	std::string get_string() const
	{
		// TODO: optional if -1
		return "location = " + std::to_string(m_location.get_index());
	}

private:
	location_type m_location;
};

}
