
#pragma once

#include <vector>
#include <array>
#include <list>
#include <type_traits>

namespace GLWRAP_NAMESPACE
{

namespace detail
{

template <typename T>
struct is_vec : std::false_type
{};

template <typename T, glm::precision P>
struct is_vec<glm::tvec2<T, P>> : std::true_type
{};

template <typename T, glm::precision P>
struct is_vec<glm::tvec3<T, P>> : std::true_type
{};

template <typename T, glm::precision P>
struct is_vec<glm::tvec4<T, P>> : std::true_type
{};


template <typename T>
struct is_mat : std::false_type
{};

template <typename T, glm::precision P>
struct is_mat<glm::tmat2x2<T, P>> : std::true_type
{};

template <typename T, glm::precision P>
struct is_mat<glm::tmat2x3<T, P>> : std::true_type
{};

template <typename T, glm::precision P>
struct is_mat<glm::tmat2x4<T, P>> : std::true_type
{};

template <typename T, glm::precision P>
struct is_mat<glm::tmat3x2<T, P>> : std::true_type
{};

template <typename T, glm::precision P>
struct is_mat<glm::tmat3x3<T, P>> : std::true_type
{};

template <typename T, glm::precision P>
struct is_mat<glm::tmat3x4<T, P>> : std::true_type
{};

template <typename T, glm::precision P>
struct is_mat<glm::tmat4x2<T, P>> : std::true_type
{};

template <typename T, glm::precision P>
struct is_mat<glm::tmat4x3<T, P>> : std::true_type
{};

template <typename T, glm::precision P>
struct is_mat<glm::tmat4x4<T, P>> : std::true_type
{};

template <typename T, typename Enable = void>
struct is_gl_integral : std::false_type {};

template <typename T>
struct is_gl_integral<T, typename std::enable_if<
		std::is_same<byte_t, T>::value ||
		std::is_same<ubyte_t, T>::value ||
		std::is_same<short_t, T>::value ||
		std::is_same<ushort_t, T>::value ||
		std::is_same<int_t, T>::value ||
		std::is_same<uint_t, T>::value>::type> : std::true_type
{};

template <typename T, typename Enable = void>
struct is_gl_floating_point : std::false_type
{};

// TODO: half_t
template <typename T>
struct is_gl_floating_point<T, typename std::enable_if<
		//std::is_same<half_t, T>::value ||
		std::is_same<float_t, T>::value ||
		std::is_same<double_t, T>::value>::type> : std::true_type
{};

template <typename T>
constexpr GLenum data_type_enum()
{
	static_assert(false && std::is_void<T>::value, "Invalid data type.");
	return 0;
}

template <>
constexpr inline GLenum data_type_enum<ubyte_t>()
{
	return GL_UNSIGNED_BYTE;
}

template <>
constexpr inline GLenum data_type_enum<byte_t>()
{
	return GL_BYTE;
}

template <>
constexpr inline GLenum data_type_enum<ushort_t>()
{
	return GL_UNSIGNED_SHORT;
}

template <>
constexpr inline GLenum data_type_enum<short_t>()
{
	return GL_SHORT;
}

template <>
constexpr inline GLenum data_type_enum<uint_t>()
{
	return GL_UNSIGNED_INT;
}

template <>
constexpr inline GLenum data_type_enum<int_t>()
{
	return GL_INT;
}
/*
template <>
constexpr inline GLenum data_type_enum<half_t>()
{
	return GL_HALF_FLOAT;
}
*/
template <>
constexpr inline GLenum data_type_enum<float_t>()
{
	return GL_FLOAT;
}

template <>
constexpr inline GLenum data_type_enum<double_t>()
{
	return GL_DOUBLE;
}

// C++17's void_t
template <class...>
using void_t = void;

template <typename T>
struct remove_cvref : std::remove_cv<typename std::remove_reference<T>::type> {};

template <typename T>
struct is_std_vector : std::false_type {};

template <typename... T>
struct is_std_vector<std::vector<T...>> : std::true_type {};

template <typename T>
struct is_std_vector_of_bool : std::false_type {};

template <typename A>
struct is_std_vector_of_bool<std::vector<bool, A>> : std::true_type {};

template <typename T>
struct is_std_array : std::false_type {};

template <typename T, std::size_t N>
struct is_std_array<std::array<T, N>> : std::true_type {};

template <typename T>
struct is_std_initializer_list : std::false_type {};

template <typename T>
struct is_std_initializer_list<std::initializer_list<T>> : std::true_type {};

template<typename T, typename Enable = void>
struct is_range : std::false_type {};

template<typename T>
struct is_range<T, void_t<
		//typename T::value_type,
		//typename T::size_type,
		//typename T::allocator_type,
		//typename T::iterator,
		//typename T::const_iterator,
		decltype(std::begin(std::declval<T>())),
		decltype(std::end(std::declval<T>()))
		//decltype(std::declval<T>().end()),
		//decltype(std::declval<T>().cbegin()),
		//decltype(std::declval<T>().cend())
			>> : std::true_type {};

// TODO: initializer list?

template <typename T>
struct is_contiguous
{
	typedef typename remove_cvref<T>::type adj_type;

	static const bool value =
		(is_std_vector<adj_type>::value && !is_std_vector_of_bool<adj_type>::value)
		|| is_std_array<adj_type>::value
		|| std::is_array<adj_type>::value;
};

static_assert(is_contiguous<std::vector<int>&>::value, "fail");
static_assert(is_contiguous<const std::array<float, 4>&>::value, "fail");
static_assert(is_contiguous<int[5]>::value, "fail");
static_assert(is_contiguous<float(&)[1]>::value, "fail");
static_assert(!is_contiguous<std::list<char>>::value, "fail");
static_assert(!is_contiguous<std::vector<bool>>::value, "fail");

template <typename T, typename M>
constexpr std::intptr_t get_member_offset(M T::*_member)
{
	// TODO: not portable
	return reinterpret_cast<std::intptr_t>(&(static_cast<const T*>(nullptr)->*_member));
}

}

}
