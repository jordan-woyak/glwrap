#pragma once

#include <string>
#include <functional>

#include "../vector.hpp"
#include "../texture.hpp"

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
type_name_t get_type_name<vec2>()
{
	return "vec2";
}

template <>
type_name_t get_type_name<vec3>()
{
	return "vec3";
}

template <>
type_name_t get_type_name<vec4>()
{
	return "vec4";
}

template <>
type_name_t get_type_name<mat3>()
{
	return "mat3";
}

template <>
type_name_t get_type_name<mat4>()
{
	return "mat4";
}

template <>
type_name_t get_type_name<texture_1d>()
{
	return "sampler1D";
}

template <>
type_name_t get_type_name<texture_2d>()
{
	return "sampler2D";
}

template <>
type_name_t get_type_name<texture_3d>()
{
	return "sampler3D";
}

template <typename T>
struct index_count;

template <typename T>
std::size_t get_index_count()
{
	return index_count<T>::value;
}

template <>
struct index_count<float_t>
{
	static const std::size_t value = 1;
};

template <>
template <typename T, int D>
struct index_count<basic_vec<T, D>>
{
	static const std::size_t value = D;
};

template <>
template <typename T, int R, int C>
struct index_count<basic_mat<T, R, C>>
{
	static const std::size_t value = R * C;
};

// TODO: make this not needed
template <>
struct index_count<texture_2d>
{
	static const std::size_t value = 0;
};

}
}
}
