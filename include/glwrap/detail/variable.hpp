#pragma once

#include <string>
#include <functional>

#include <boost/format.hpp>

#include "../vector.hpp"
#include "../texture.hpp"

namespace gl
{
namespace detail
{
namespace variable
{

typedef std::string type_name_t;

template <typename T>
struct glsl_var_type;

template <typename T>
type_name_t get_type_name()
{
	return glsl_var_type<T>::name();
}

// scalars
template <>
type_name_t get_type_name<bool_t>()
{
	return "bool";
}

template <>
type_name_t get_type_name<int_t>()
{
	return "int";
}

template <>
type_name_t get_type_name<uint_t>()
{
	return "uint";
}

template <>
type_name_t get_type_name<float_t>()
{
	return "float";
}

template <>
type_name_t get_type_name<double_t>()
{
	return "double";
}

// vectors

template <typename T>
const char* vec_prefix();

template <>
const char* vec_prefix<bool_t>()
{
	return "b";
}

template <>
const char* vec_prefix<int_t>()
{
	return "i";
}

template <>
const char* vec_prefix<uint_t>()
{
	return "u";
}

template <>
const char* vec_prefix<float_t>()
{
	return "";
}

template <>
const char* vec_prefix<double_t>()
{
	return "d";
}

template <>
template <typename T, int D>
struct glsl_var_type<basic_vec<T, D>>
{
	static type_name_t name()
	{
		return (boost::format("%svec%d") % vec_prefix<T>() % D).str();
	}
};

// matrix
template <>
template <typename T, int R, int C>
struct glsl_var_type<basic_mat<T, R, C>>
{
	static_assert(std::is_same<T, float_t>::value ||
		std::is_same<T, double_t>::value,
		"only float and double matrices");

	static type_name_t name()
	{
		return (boost::format("%smat%dx%d") % vec_prefix<T>() % R % C).str();
	}
};

template <>
template <int D>
struct glsl_var_type<texture<D>>
{
	static type_name_t name()
	{
		return (boost::format("sampler%dD") % D).str();
	}
};

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

}
}
}
