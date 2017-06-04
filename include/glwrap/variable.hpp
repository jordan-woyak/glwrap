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

// TODO: variable_description should just hold the name
// not the location..
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

private:
	std::string m_name;
	layout_type m_layout;
};

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

// Atomic Counters
template <typename T>
auto variable(std::string _name, const atomic_counter_binding_attribute<T>& _attrib)
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

