#pragma once

#include "detail/attribute.hpp"
#include "detail/variable.hpp"
#include "detail/context.hpp"

#include <list>

namespace GLWRAP_NAMESPACE
{

// TODO: rename this to drawbuffer ?
// TODO: does it make sense for this to have a type?
// color_number ?
// "fragment_color"
// buffer index
template <typename T>
class fragdata_location
{
public:
	fragdata_location(int_t _index)
		: m_index(_index)
	{}

	int_t get_index() const
	{
		return m_index;
	}

	int_t m_index;
};

// TODO: rename this
class fragdata_location_enumerator
{
public:
	template <typename T>
	using location_type = fragdata_location<T>;
	
	// TODO: really need context?
	fragdata_location_enumerator(context& _context)
		: m_current_index()
		, m_max_locations()
	{
		detail::gl_get(GL_MAX_DRAW_BUFFERS, &m_max_locations);
	}

	template <typename T>
	fragdata_location<T> get()
	{
		if (m_current_index >= m_max_locations)
			throw exception(0);
			
		fragdata_location<T> ind(m_current_index);

		m_current_index += 1;

		return ind;
	}

private:
	int_t m_current_index;
	int_t m_max_locations;
};

class program;

struct attach_point
{
	friend class context;

public:
	GLenum get_value() const
	{
		return m_attachment;
	}

protected:
	attach_point(GLenum _attachment)
		: m_attachment(_attachment)
	{}

	GLenum m_attachment;
};


// TODO: detail namespace
// TODO: name
template <typename T>
class fragdata_layout
{
public:
	typedef fragdata_location<T> location_type;

	fragdata_layout(const location_type& _loc)
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
