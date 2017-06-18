#pragma once

#include "detail/attribute.hpp"
#include "detail/variable.hpp"
#include "detail/context.hpp"

#include <list>

namespace GLWRAP_NAMESPACE
{

namespace detail
{

// TODO: rename this to drawbuffer ?
// TODO: does it make sense for this to have a type?
// color_number ?
// "fragment_color"
// buffer index
struct fragdata_location_traits
{
	// TODO: actually use this typedef in typed_index
	typedef int_t index_type;

	static index_type get_index_count()
	{
		int_t val = 0;

		detail::gl_get(GL_MAX_DRAW_BUFFERS, &val);

		return val;
	}

	template <typename T>
	static index_type get_index_usage()
	{
		return 1;
	}
};

}

template <typename T>
using fragdata_location = detail::typed_index<int_t, detail::fragdata_location_traits, T>;

typedef detail::typed_index_enumerator<detail::fragdata_location_traits, fragdata_location>
	fragdata_location_enumerator;

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
