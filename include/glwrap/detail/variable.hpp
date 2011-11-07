#pragma once

#include <string>
#include <functional>

#include "../vector.hpp"
#include "../sampler.hpp"

namespace gl
{
namespace detail
{
namespace variable
{

typedef const char* type_name_t;

template <typename T>
type_name_t get_type_name();

template <>
type_name_t get_type_name<float>()
{
	return "float";
}

template <>
type_name_t get_type_name<fvec2>()
{
	return "vec2";
}

template <>
type_name_t get_type_name<fvec3>()
{
	return "vec3";
}

template <>
type_name_t get_type_name<fvec4>()
{
	return "vec4";
}

template <>
type_name_t get_type_name<matrix4>()
{
	return "mat4";
}

template <>
type_name_t get_type_name<sampler_1d>()
{
	return "sampler1D";
}

template <>
type_name_t get_type_name<sampler_2d>()
{
	return "sampler2D";
}

template <>
type_name_t get_type_name<sampler_3d>()
{
	return "sampler3D";
}

template <typename T>
std::size_t get_index_count();

template <>
std::size_t get_index_count<float_t>()
{
	return 1;
}

template <>
std::size_t get_index_count<fvec2>()
{
	return 2;
}

template <>
std::size_t get_index_count<fvec3>()
{
	return 3;
}

template <>
std::size_t get_index_count<fvec4>()
{
	return 4;
}

template <>
std::size_t get_index_count<matrix4>()
{
	return 4 * 4;
}

}
}
}
