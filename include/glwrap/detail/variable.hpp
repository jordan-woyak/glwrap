#pragma once

#include <string>
#include <functional>

//#include <boost/format.hpp>
#include <sstream>

#include "../vector.hpp"
#include "../declarations.hpp"

#include "traits.hpp"

#include "../matrix.hpp"
#include "../vector.hpp"

namespace gl
{

namespace shader
{

// empty type defs for use with glsl variables
template <texture_type T, typename D>
struct basic_sampler {};

// TODO: eliminate some redundancy with macros:

template <typename T>
using basic_sampler_2d = basic_sampler<texture_type::texture_2d, T>;

template <typename T>
using basic_sampler_3d = basic_sampler<texture_type::texture_3d, T>;

// TODO: allow for sampler_2d[] syntax?
template <typename T>
using basic_sampler_2d_array = basic_sampler<texture_type::texture_2d_array, T>;

template <typename T>
using basic_sampler_cube_map = basic_sampler<texture_type::texture_cube_map, T>;

template <typename T>
using basic_sampler_2d_multisample = basic_sampler<texture_type::texture_2d_multisample, T>;

// float samplers
typedef basic_sampler_2d<float_t> sampler_2d;
typedef basic_sampler_3d<float_t> sampler_3d;
typedef basic_sampler_2d_array<float_t> sampler_2d_array;
typedef basic_sampler_cube_map<float_t> sampler_cube_map;
typedef basic_sampler_2d_multisample<float_t> sampler_2d_multisample;

// int samplers
typedef basic_sampler_2d<int_t> isampler_2d;
typedef basic_sampler_3d<int_t> isampler_3d;
typedef basic_sampler_2d_array<int_t> isampler_2d_array;
typedef basic_sampler_cube_map<int_t> isampler_cube;
typedef basic_sampler_2d_multisample<int_t> isampler_2d_multisample;

// uint samplers
typedef basic_sampler_2d<uint_t> usampler_2d;
typedef basic_sampler_3d<uint_t> usampler_3d;
typedef basic_sampler_2d_array<uint_t> usampler_2d_array;
typedef basic_sampler_cube_map<uint_t> usampler_cube;
typedef basic_sampler_2d_multisample<uint_t> usampler_2d_multisample;

template <texture_type T, typename D>
struct basic_image {};

template <typename T>
using basic_image_2d = basic_image<texture_type::texture_2d, T>;

template <typename T>
using basic_image_3d = basic_image<texture_type::texture_3d, T>;

// TODO: allow for image_2d[] syntax?
template <typename T>
using basic_image_2d_array = basic_image<texture_type::texture_2d_array, T>;

template <typename T>
using basic_image_cube_map = basic_image<texture_type::texture_cube_map, T>;

template <typename T>
using basic_image_2d_multisample = basic_image<texture_type::texture_2d_multisample, T>;

// float images
typedef basic_image_2d<float_t> image_2d;
typedef basic_image_3d<float_t> image_3d;
typedef basic_image_2d_array<float_t> image_2d_array;
typedef basic_image_cube_map<float_t> image_cube_map;
typedef basic_image_2d_multisample<float_t> image_2d_multisample;

// int images
typedef basic_image_2d<int_t> iimage_2d;
typedef basic_image_3d<int_t> iimage_3d;
typedef basic_image_2d_array<int_t> iimage_2d_array;
typedef basic_image_cube_map<int_t> iimage_cube_map;
typedef basic_image_2d_multisample<int_t> iimage_2d_multisample;

// uint images
typedef basic_image_2d<uint_t> uimage_2d;
typedef basic_image_3d<uint_t> uimage_3d;
typedef basic_image_2d_array<uint_t> uimage_2d_array;
typedef basic_image_cube_map<uint_t> uimage_cube_map;
typedef basic_image_2d_multisample<uint_t> uimage_2d_multisample;

// Decorator template for atomic counters
// Currently only used internally for typename string generation: atomic_uint
template <typename T>
struct atomic
{
	T val;
};

}

namespace detail
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
struct glsl_var_type<T, typename std::enable_if<is_vec<T>::value>::type>
{
	static type_name_t name()
	{
		typedef typename vec_traits<T>::value_type value_type;
		int const dimensions = vec_traits<T>::dimensions;
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
struct glsl_var_type<T, typename std::enable_if<is_mat<T>::value>::type>
{
	static type_name_t name()
	{
		typedef typename mat_traits<T>::value_type value_type;
		
		int const cols = mat_traits<T>::cols;
		int const rows = mat_traits<T>::rows;
		
#if 0
		return (boost::format("%smat%dx%d") % vec_prefix<value_type>() % cols % rows).str();
#else
		std::ostringstream ss;
		ss << vec_prefix<value_type>() << "mat" << cols << 'x' << rows;
		return ss.str();
#endif
	}
};

// TODO: template to get the sampler/image texture-type suffix

template <typename T>
struct glsl_var_type<shader::basic_sampler_2d<T>>
{
	static type_name_t name()
	{
		return vec_prefix<T>() + "sampler2D";
	}
};

template <typename T>
struct glsl_var_type<shader::basic_sampler_3d<T>>
{
	static type_name_t name()
	{
		return vec_prefix<T>() + "sampler3D";
	}
};

template <typename T>
struct glsl_var_type<shader::basic_sampler_2d_array<T>>
{
	static type_name_t name()
	{
		return vec_prefix<T>() + "sampler2DArray";
	}
};

template <typename T>
struct glsl_var_type<shader::basic_sampler_cube_map<T>>
{
	static type_name_t name()
	{
		return vec_prefix<T>() + "samplerCube";
	}
};

template <typename T>
struct glsl_var_type<shader::basic_sampler_2d_multisample<T>>
{
	static type_name_t name()
	{
		return vec_prefix<T>() + "sampler2DMS";
	}
};

template <typename T>
struct glsl_var_type<shader::atomic<T>>
{
	static type_name_t name()
	{
		return "atomic_" + get_type_name<T>();
	}
};

template <typename T, typename Enable = void>
struct is_valid_vec_size : std::false_type
{};

template <typename T>
struct is_valid_vec_size<T, typename std::enable_if<
	(vec_traits<T>::dimensions >= 2) &&
	(vec_traits<T>::dimensions <= 4)
	>::type> : std::true_type
{};

template <typename T, typename Enable = void>
struct is_valid_mat_size : std::false_type
{};

template <typename T>
struct is_valid_mat_size<T, typename std::enable_if<
	(mat_traits<T>::rows >= 2) &&
	(mat_traits<T>::rows <= 4) &&
	(mat_traits<T>::cols >= 2) &&
	(mat_traits<T>::cols <= 4)
	>::type> : std::true_type
{};

template <typename T, typename Enable = void>
struct is_valid_vec_value_type : std::false_type
{};

template <typename T>
struct is_valid_vec_value_type<T, typename std::enable_if<
	std::is_same<typename vec_traits<T>::value_type, bool_t>::value ||
	std::is_same<typename vec_traits<T>::value_type, int_t>::value ||
	std::is_same<typename vec_traits<T>::value_type, uint_t>::value ||
	std::is_same<typename vec_traits<T>::value_type, float_t>::value ||
	std::is_same<typename vec_traits<T>::value_type, double_t>::value
	>::type> : std::true_type
{};

template <typename T, typename Enable = void>
struct is_valid_mat_value_type : std::false_type
{};

template <typename T>
struct is_valid_mat_value_type<T, typename std::enable_if<
	std::is_same<typename mat_traits<T>::value_type, float_t>::value ||
	std::is_same<typename mat_traits<T>::value_type, double_t>::value
	>::type> : std::true_type
{};

// TODO: put this in the shader namespace?

template <typename T, typename Enable = void>
struct is_valid_shader_variable_type : std::false_type
{};

template <typename T>
struct is_valid_shader_variable_type<T, typename std::enable_if<
	std::is_same<T, bool_t>::value ||
	std::is_same<T, int_t>::value ||
	std::is_same<T, uint_t>::value ||
	std::is_same<T, float_t>::value ||
	std::is_same<T, double_t>::value
	>::type> : std::true_type
{};

template <typename T>
struct is_valid_shader_variable_type<T, typename std::enable_if<
	is_vec<T>::value &&
	is_valid_vec_size<T>::value &&
	is_valid_vec_value_type<T>::value
	>::type> : std::true_type
{};

template <typename T>
struct is_valid_shader_variable_type<T, typename std::enable_if<
	is_mat<T>::value &&
	is_valid_mat_size<T>::value &&
	is_valid_mat_value_type<T>::value
	>::type> : std::true_type
{};

template <typename T>
struct is_valid_shader_variable_type<T, typename std::enable_if<
	std::is_array<T>::value
	>::type> : is_valid_shader_variable_type<typename std::remove_extent<T>::type>
{};

// Breaks down a T vec,mat,array or T itself down to T
template <typename T, typename Enable = void>
struct underlying_type
{
	typedef T type;

	template <typename M>
	using modify = M;
};

template <typename T>
struct underlying_type<T, typename std::enable_if<is_vec<T>::value>::type>
{
	typedef typename vec_traits<T>::value_type type;

	template <typename M>
	using modify = basic_vec<M, vec_traits<T>::dimensions>;
};

template <typename T>
struct underlying_type<T, typename std::enable_if<is_mat<T>::value>::type>
{
	typedef typename mat_traits<T>::value_type type;

	template <typename M>
	using modify = basic_mat<M, mat_traits<T>::cols, mat_traits<T>::rows>;
};

template <typename T>
struct underlying_type<T, typename std::enable_if<std::is_array<T>::value>::type>
{
	typedef typename underlying_type<typename std::remove_extent<T>::type>::type type;

	// TODO: implement if needed:
	//template <typename M>
	//using modify = ;
};

}

}
