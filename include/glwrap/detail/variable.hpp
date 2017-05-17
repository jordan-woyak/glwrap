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

// TODO: move the basic_* types into namespace detail
// empty type defs for use with glsl variables
template <typename T>
struct basic_sampler_2d {};

template <typename T>
struct basic_sampler_3d {};

// TODO: allow for sampler_2d[] syntax
template <typename T>
struct basic_sampler_2d_array {};

template <typename T>
struct basic_sampler_cube {};

template <typename T>
struct basic_sampler_2dms {};

// float samplers
typedef basic_sampler_2d<float_t> sampler_2d;
typedef basic_sampler_3d<float_t> sampler_3d;
typedef basic_sampler_2d_array<float_t> sampler_2d_array;
typedef basic_sampler_cube<float_t> sampler_cube;
typedef basic_sampler_2dms<float_t> sampler_2dms;

// int samplers
typedef basic_sampler_2d<int_t> isampler_2d;
typedef basic_sampler_3d<int_t> isampler_3d;
typedef basic_sampler_2d_array<int_t> isampler_2d_array;
typedef basic_sampler_cube<int_t> isampler_cube;
typedef basic_sampler_2dms<int_t> isampler_2dms;

// uint samplers
typedef basic_sampler_2d<uint_t> usampler_2d;
typedef basic_sampler_3d<uint_t> usampler_3d;
typedef basic_sampler_2d_array<uint_t> usampler_2d_array;
typedef basic_sampler_cube<uint_t> usampler_cube;
typedef basic_sampler_2dms<uint_t> usampler_2dms;

template <typename T>
struct basic_image_2d {};

template <typename T>
struct basic_image_3d {};

// TODO: allow for image_2d[] syntax
template <typename T>
struct basic_image_2d_array {};

template <typename T>
struct basic_image_cube {};

template <typename T>
struct basic_image_2dms {};

// float images
typedef basic_image_2d<float_t> image_2d;
typedef basic_image_3d<float_t> image_3d;
typedef basic_image_2d_array<float_t> image_2d_array;
typedef basic_image_cube<float_t> image_cube;
typedef basic_image_2dms<float_t> image_2dms;

// int images
typedef basic_image_2d<int_t> iimage_2d;
typedef basic_image_3d<int_t> iimage_3d;
typedef basic_image_2d_array<int_t> iimage_2d_array;
typedef basic_image_cube<int_t> iimage_cube;
typedef basic_image_2dms<int_t> iimage_2dms;

// uint images
typedef basic_image_2d<uint_t> uimage_2d;
typedef basic_image_3d<uint_t> uimage_3d;
typedef basic_image_2d_array<uint_t> uimage_2d_array;
typedef basic_image_cube<uint_t> uimage_cube;
typedef basic_image_2dms<uint_t> uimage_2dms;

namespace detail
{

// TODO: kill this namespace?
namespace glslvar
{

typedef std::string type_name_t;

// glsl variable name suffix
template <typename T, typename Enable = void>
struct glsl_var_suffix;

template <typename T>
struct glsl_var_suffix<T, typename std::enable_if<std::is_array<T>::value>::type>
{
	static type_name_t suffix()
	{
		return "[" + std::to_string(std::extent<T>::value) + "]";
	}
};

template <typename T>
struct glsl_var_suffix<T, typename std::enable_if<!std::is_array<T>::value>::type>
{
	static type_name_t suffix()
	{
		return "";
	}
};

template <typename T>
type_name_t get_type_suffix()
{
	return glsl_var_suffix<T>::suffix();
}

// glsl variable type name
template <typename T, typename Enable = void>
struct glsl_var_type;

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
std::string vec_prefix()
{
	static_assert(false && std::is_void<T>::value, "Unsupported vector type");
	return nullptr;
}

template <>
inline std::string vec_prefix<bool_t>()
{
	return "b";
}

template <>
inline std::string vec_prefix<int_t>()
{
	return "i";
}

template <>
inline std::string vec_prefix<uint_t>()
{
	return "u";
}

template <>
inline std::string vec_prefix<float_t>()
{
	return "";
}

template <>
inline std::string vec_prefix<double_t>()
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

template <typename T>
struct glsl_var_type<basic_sampler_2d<T>>
{
	static type_name_t name()
	{
		return vec_prefix<T>() + "sampler2D";
	}
};

template <typename T>
struct glsl_var_type<basic_sampler_3d<T>>
{
	static type_name_t name()
	{
		return vec_prefix<T>() + "sampler3D";
	}
};

template <typename T>
struct glsl_var_type<basic_sampler_cube<T>>
{
	static type_name_t name()
	{
		return vec_prefix<T>() + "samplerCube";
	}
};

// TODO: rename
template <typename T, typename Enable = void>
struct index_count;

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

template <typename T>
struct index_count<T, typename std::enable_if<std::is_array<T>::value>::type>
{
private:
	typedef typename std::remove_extent<T>::type value_type;
	
public:
	static const std::size_t value = index_count<value_type>::value * std::extent<T>::value;
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
	std::is_same<typename mat_traits<T>::value_type, float_t>::value ||
	std::is_same<typename mat_traits<T>::value_type, double_t>::value
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

template <typename T>
struct is_valid_glsl_type<T, typename std::enable_if<
	std::is_array<T>::value
	>::type> : is_valid_glsl_type<typename std::remove_extent<T>::type>
{};

}
}
}
