#pragma once

#include "detail/uniform.hpp"

namespace GLWRAP_NAMESPACE
{

namespace detail
{

struct uniform_index_traits
{
	static int_t get_index_count()
	{
		int_t val = 0;

		detail::gl_get(GL_MAX_UNIFORM_LOCATIONS, &val);

		return val;
	}

	template <typename T>
	static int_t get_index_usage()
	{
		return detail::uniform_location_count<T>::value;
	}
};

}

template <typename T>
using uniform_location = detail::typed_index<int_t, detail::uniform_index_traits, T>;

// TODO: name?
typedef detail::typed_index_enumerator<detail::uniform_index_traits, uniform_location>
	uniform_location_enumerator;

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

	// TODO: rename?
	void add_layout_part(std::string _key, std::string _val)
	{
		m_layout_parts[std::move(_key)] = std::move(_val);
	}

	std::string get_string() const
	{
		// TODO: optional if -1
		std::string result = "location = " + std::to_string(m_location.get_index());

		for (auto& layout_part : m_layout_parts)
		{
			result += ", " + layout_part.first;
			if (!layout_part.second.empty())
				result += " = " + layout_part.second;
		}

		return result;
	}

private:
	// TODO: move the location into the below map
	location_type m_location;
	std::map<std::string, std::string> m_layout_parts;
};

}
