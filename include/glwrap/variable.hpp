#pragma once

#include "matrix.hpp"

#include "detail/variable.hpp"

#include "attribute.hpp"
#include "uniform.hpp"
#include "atomic_counter.hpp"
#include "fragdata.hpp"
#include "shader_storage.hpp"
//#include "transform_feedback.hpp"


namespace GLWRAP_NAMESPACE
{

template <typename T, typename L>
class variable_description
{
public:
	typedef T value_type;
	typedef L layout_type;

	variable_description(std::string _name, const layout_type& _layout)
		: m_name(std::move(_name))
		, m_layout(_layout)
	{}

	std::string const& get_name() const
	{
		return m_name;
	}

	layout_type const& get_layout() const
	{
		return m_layout;
	}

	// TODO: private:
	std::vector<std::string> m_memory_qualifiers;

private:
	std::string m_name;
	layout_type m_layout;
};

// TODO: complete and use this
/*
template <typename T>
class typed_variable_layout
{
	// TODO: rename?
	void add_layout_part(std::string _key, std::string _val)
	{
		m_layout_parts[std::move(_key)] = std::move(_val);
	}

	std::string get_string() const
	{
		for (auto& layout_part : m_layout_parts)
		{
			result += ", " + layout_part.first;
			if (!layout_part.second.empty())
				result += " = " + layout_part.second;
		}

		return result;
	}

private:
	std::map<std::string, std::string> m_layout_parts;
};

template <typename T>
void add_layout_part(typed_variable_layout<T>& _layout, T::location_type)
{

}
*/

// Vertex Attribs
template <typename T>
auto variable(std::string _name, attribute_location_enumerator& _enum) -> variable_description<T, attribute_layout<T>>
{
	return {std::move(_name), _enum.template get<T>()};
}

// Fragdatas
template <typename T>
auto variable(std::string _name, fragdata_location_enumerator& _enum) -> variable_description<T, fragdata_layout<T>>
{
	return {std::move(_name), _enum.template get<T>()};
}

// Uniforms
template <typename T>
auto variable(std::string _name, uniform_location_enumerator& _enum)
	-> variable_description<T, uniform_layout<T>>
{
	return {std::move(_name), _enum.template get<T>()};
}

// Image uniform. HACKY!!!
template <typename T, typename F>
auto variable(std::string _name, uniform_location_enumerator& _enum, const F& _img_fmt)
	-> variable_description<T, uniform_layout<T>>
{
	static_assert(std::is_same<typename image_format<typename T::data_type>::enum_type, F>::value, "Wrong image format for specified image type.");

	auto img_layout = gl::uniform_layout<T>(_enum.get<T>());
	img_layout.add_layout_part(gl::detail::format_qualifier_string(_img_fmt), "");

	return {std::move(_name), img_layout};
}

// Atomic Counters
template <typename T>
auto variable(std::string _name, const atomic_counter_binding<T>& _attrib)
	-> variable_description<shader::atomic<T>, atomic_counter_layout<T>>
{
	static_assert(std::is_same<T, uint_t>::value, "Atomic counters must be of type: uint.");

	return {std::move(_name), _attrib};
}

// Shader Storage
template <typename T>
auto variable(std::string _name, shader_storage_location_enumerator& _enum)
	-> variable_description<T, shader_storage_layout<T>>
{
	//static_assert(std::is_array<T>::value && 0 == std::extent<T>::value,
		//"Shader storage currently only supports arrays of indeterminate length.");

	return {std::move(_name), _enum.template get<T>()};
}

// Uniform Block
template <typename T>
auto variable(std::string _name, uniform_block_location_enumerator& _enum)
	-> variable_description<T, uniform_block_layout<T>>
{
	// TODO: require T is struct

	return {std::move(_name), _enum.template get<T>()};
}

/*
// TODO: templatify all the location types so this can assume the variable type from the location
template <typename T, typename L>
auto variable(std::string _name, L& _loc) -> variable_description<T, L>
{
	return {std::move(_name), _loc};
}
*/

}

