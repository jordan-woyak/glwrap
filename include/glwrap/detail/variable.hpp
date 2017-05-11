#pragma once

#include <string>
#include <functional>

//#include <boost/format.hpp>
#include <sstream>

#include "../vector.hpp"
#include "../declarations.hpp"

#include "traits.hpp"

namespace gl
{

namespace detail
{

// TODO: kill this namespace?
namespace glslvar
{

typedef std::string type_name_t;

// glsl variable name suffix
template <typename T, typename Enable = void>
struct glsl_var_suffix
{};

template <typename T>
type_name_t get_type_suffix()
{
	return glsl_var_suffix<T>::suffix();
}

template <typename T>
struct glsl_var_suffix<T, std::enable_if<std::is_array<T>::value>>
{
	static type_name_t suffix()
	{
		// TODO: ??
		return "";
	}
};

// glsl variable type name
template <typename T, typename Enable = void>
struct glsl_var_type
{};

template <typename T>
type_name_t get_type_name()
{
	return glsl_var_type<T>::name();
}

template <typename T>
struct glsl_var_type<T, typename std::enable_if<std::is_array<T>::value>::type>
{
	static type_name_t name()
	{
		return get_type_name<typename std::remove_extent<T>::type>();
	}
};

// scalars
template <>
inline type_name_t get_type_name<bool_t>()
{
	return "bool";
}

template <>
inline type_name_t get_type_name<int_t>()
{
	return "int";
}

template <>
inline type_name_t get_type_name<uint_t>()
{
	return "uint";
}

template <>
inline type_name_t get_type_name<float_t>()
{
	return "float";
}

template <>
inline type_name_t get_type_name<double_t>()
{
	return "double";
}

// vectors

template <typename T>
const char* vec_prefix()
{
	static_assert(false && std::is_void<T>::value, "Unsupported vector type");
	return nullptr;
}

template <>
inline const char* vec_prefix<bool_t>()
{
	return "b";
}

template <>
inline const char* vec_prefix<int_t>()
{
	return "i";
}

template <>
inline const char* vec_prefix<uint_t>()
{
	return "u";
}

template <>
inline const char* vec_prefix<float_t>()
{
	return "";
}

template <>
inline const char* vec_prefix<double_t>()
{
	return "d";
}

template <typename T>
struct glsl_var_type<T, typename std::enable_if<detail::is_vec<T>::value>::type>
{
	static type_name_t name()
	{
		typedef typename detail::vec_traits<T>::value_type value_type;
		int const dimensions = detail::vec_traits<T>::dimensions;
#if 0
		return (boost::format("%svec%d") % vec_prefix<value_type>() % dimensions).str();
#else
		std::ostringstream ss;
		ss << vec_prefix<value_type>() << "vec" << dimensions;
		return ss.str();
#endif
	}
};

template <typename T>
struct glsl_var_type<T, typename std::enable_if<detail::is_mat<T>::value>::type>
{
	static type_name_t name()
	{
		typedef typename detail::mat_traits<T>::value_type value_type;
		
		int const cols = detail::mat_traits<T>::cols;
		int const rows = detail::mat_traits<T>::rows;

		static_assert(std::is_same<value_type, float_t>::value ||
			std::is_same<value_type, double_t>::value,
			"only float and double matrices supported");
		
#if 0
		return (boost::format("%smat%dx%d") % vec_prefix<value_type>() % cols % rows).str();
#else
		std::ostringstream ss;
		ss << vec_prefix<value_type>() << "mat" << cols << 'x' << rows;
		return ss.str();
#endif
	}
};

template <>
inline type_name_t get_type_name<texture_1d>()
{
	return "sampler1D";
}

template <>
inline type_name_t get_type_name<texture_2d>()
{
	return "sampler2D";
}

template <>
inline type_name_t get_type_name<texture_rectangle>()
{
	return "sampler2DRect";
}

template <>
inline type_name_t get_type_name<texture_3d>()
{
	return "sampler3D";
}

template <>
inline type_name_t get_type_name<texture_cube_map>()
{
	return "samplerCube";
}

template <>
inline type_name_t get_type_name<texture_buffer>()
{
	return "samplerBuffer";
}

// TODO: rename
template <typename T, typename Enable = void>
struct index_count
{};

template <typename T>
struct index_count<T, typename std::enable_if<
	std::is_same<T, int_t>::value ||
	std::is_same<T, uint_t>::value ||
	std::is_same<T, float_t>::value ||
	std::is_same<T, double_t>::value
	, void>::type>
{
	static const std::size_t value = 1;
};

template <typename T>
struct index_count<T, typename std::enable_if<detail::is_vec<T>::value>::type>
{
private:
	typedef typename detail::vec_traits<T>::value_type value_type;

public:
	static const std::size_t value = index_count<value_type>::value;
};

template <typename T>
struct index_count<T, typename std::enable_if<detail::is_mat<T>::value>::type>
{
private:
	typedef typename detail::mat_traits<T>::value_type value_type;
	
public:
	static const std::size_t value = index_count<value_type>::value * detail::mat_traits<T>::cols;
};

// TODO: complete and use these
template <typename T, typename Enable = void>
struct is_valid_glsl_vec_size : std::false_type
{};

template <typename T>
struct is_valid_glsl_vec_size<T, typename std::enable_if<
	(vec_traits<T>::dimensions >= 2) &&
	(vec_traits<T>::dimensions <= 4)
	>::type> : std::true_type
{};

template <typename T, typename Enable = void>
struct is_valid_glsl_mat_size : std::false_type
{};

template <typename T>
struct is_valid_glsl_mat_size<T, typename std::enable_if<
	(mat_traits<T>::rows >= 2) &&
	(mat_traits<T>::rows <= 4) &&
	(mat_traits<T>::cols >= 2) &&
	(mat_traits<T>::cols <= 4)
	>::type> : std::true_type
{};

template <typename T, typename Enable = void>
struct is_valid_glsl_vec_value_type : std::false_type
{};

template <typename T>
struct is_valid_glsl_vec_value_type<T, typename std::enable_if<
	std::is_same<typename vec_traits<T>::value_type, bool_t>::value ||
	std::is_same<typename vec_traits<T>::value_type, int_t>::value ||
	std::is_same<typename vec_traits<T>::value_type, uint_t>::value ||
	std::is_same<typename vec_traits<T>::value_type, float_t>::value ||
	std::is_same<typename vec_traits<T>::value_type, double_t>::value
	>::type> : std::true_type
{};

template <typename T, typename Enable = void>
struct is_valid_glsl_mat_value_type : std::false_type
{};

template <typename T>
struct is_valid_glsl_mat_value_type<T, typename std::enable_if<
	std::is_same<typename vec_traits<T>::value_type, float_t>::value ||
	std::is_same<typename vec_traits<T>::value_type, double_t>::value
	>::type> : std::true_type
{};

template <typename T, typename Enable = void>
struct is_valid_glsl_type : std::false_type
{};

template <typename T>
struct is_valid_glsl_type<T, typename std::enable_if<
	std::is_same<T, bool_t>::value ||
	std::is_same<T, int_t>::value ||
	std::is_same<T, uint_t>::value ||
	std::is_same<T, float_t>::value ||
	std::is_same<T, double_t>::value
	>::type> : std::true_type
{};

template <typename T>
struct is_valid_glsl_type<T, typename std::enable_if<
	is_vec<T>::value &&
	is_valid_glsl_vec_size<T>::value &&
	is_valid_glsl_vec_value_type<T>::value
	>::type> : std::true_type
{};

template <typename T>
struct is_valid_glsl_type<T, typename std::enable_if<
	is_mat<T>::value &&
	is_valid_glsl_mat_size<T>::value &&
	is_valid_glsl_mat_value_type<T>::value
	>::type> : std::true_type
{};

}
}
}
