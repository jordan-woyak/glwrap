
#pragma once

#include <cstdint>
#include <vector>

#include <GL/glew.h>

#include "vector.hpp"

namespace gl
{

// TODO: move this
typedef decltype(glGenBuffers) glgenfunc;
inline GLuint gen_return(glgenfunc f)
{
	GLuint name;
	f(1, &name);
	return name;
}

namespace detail
{

// TODO: I'd like to kill this.
template <typename T>
class scoped_binder
{
public:
	scoped_binder(const scoped_binder&) = delete;
	scoped_binder& operator=(const scoped_binder&) = delete;

	explicit scoped_binder(const T& _obj)
		: m_prev_binding(T::get_current_binding())
		, m_desired_binding(_obj.native_handle())
	{
		if (m_prev_binding != m_desired_binding)
		{
			T::set_current_binding(m_desired_binding);
		}
	}

	scoped_binder(scoped_binder&& _other)
		: m_prev_binding(_other.m_prev_binding)
		, m_desired_binding(_other.m_desired_binding)
	{
		// _other will no longer unbind on destruction
		_other.m_desired_binding = m_prev_binding;
	}

	~scoped_binder()
	{
		if (m_prev_binding != m_desired_binding)
		{
			T::set_current_binding(m_prev_binding);
		}
	}

private:
	typename T::native_handle_type const m_prev_binding;
	typename T::native_handle_type m_desired_binding;
};

template <typename T>
scoped_binder<T> make_scoped_binder(const T& _obj)
{
	return scoped_binder<T>{_obj};
}

template <typename T>
typename std::enable_if<std::is_arithmetic<T>::value, T>::type* value_ptr(T& _val)
{
	return &_val;
}

struct exception
{
	GLenum error_flag;
};

void check_error()
{
	GLenum const err = glGetError();

	if (GL_NO_ERROR != err)
	{
		exception ex;
		ex.error_flag = err;
		throw ex;
	}
}

GLWRAP_FUNC_DECL
void
check_unlikely_error()
{
	//check_error();
}

template <typename T, typename Enable = void>
struct is_gl_integral : std::false_type
{};

template <typename T>
struct is_gl_integral<T, typename std::enable_if<
		std::is_same<gl::byte_t, T>::value ||
		std::is_same<gl::ubyte_t, T>::value ||
		std::is_same<gl::short_t, T>::value ||
		std::is_same<gl::ushort_t, T>::value ||
		std::is_same<gl::int_t, T>::value ||
		std::is_same<gl::uint_t, T>::value>::type> : std::true_type
{};

template <typename T, typename Enable = void>
struct is_gl_floating_point : std::false_type
{};

// TODO: half_t
template <typename T>
struct is_gl_floating_point<T, typename std::enable_if<
		//std::is_same<gl::half_t, T>::value ||
		std::is_same<gl::float_t, T>::value ||
		std::is_same<gl::double_t, T>::value>::type> : std::true_type
{};

template <typename T>
GLenum data_type_enum()
{
	static_assert(false && std::is_void<T>::value, "Invalid data type.");
	return 0;
}

template <>
inline GLenum data_type_enum<ubyte_t>()
{
	return GL_UNSIGNED_BYTE;
}

template <>
inline GLenum data_type_enum<byte_t>()
{
	return GL_BYTE;
}

template <>
inline GLenum data_type_enum<ushort_t>()
{
	return GL_UNSIGNED_SHORT;
}

template <>
inline GLenum data_type_enum<short_t>()
{
	return GL_SHORT;
}

template <>
inline GLenum data_type_enum<uint_t>()
{
	return GL_UNSIGNED_INT;
}

template <>
inline GLenum data_type_enum<int_t>()
{
	return GL_INT;
}

// TODO: half_t
/*
template <>
inline GLenum data_type_enum<half_t>()
{
	return GL_HALF_FLOAT;
}
*/
template <>
inline GLenum data_type_enum<float_t>()
{
	return GL_FLOAT;
}

template <>
inline GLenum data_type_enum<double_t>()
{
	return GL_DOUBLE;
}

template <typename T>
struct remove_reference_cv : std::remove_cv<typename std::remove_reference<T>::type>
{};

template <typename T, typename U>
struct is_same_ignore_reference_cv : std::is_same<
	typename remove_reference_cv<T>::type,
	typename remove_reference_cv<U>::type
	>
{};

template <typename T>
struct is_std_vector : std::false_type
{};

template <typename U>
struct is_std_vector<std::vector<U>> : std::true_type
{};

template <typename T>
struct is_std_array : std::false_type
{};

template <typename U, std::size_t V>
struct is_std_array<std::array<U, V>> : std::true_type
{};

// TODO: initializer list?

template <typename T>
struct is_contiguous : std::integral_constant<bool,
	(is_std_vector<typename remove_reference_cv<T>::type>::value ||
	is_std_array<typename remove_reference_cv<T>::type>::value ||
	std::is_array<typename remove_reference_cv<T>::type>::value) &&
	!std::is_same<std::vector<bool>, typename remove_reference_cv<T>::type>::value
	>
{};

static_assert(is_contiguous<std::vector<int>&>::value, "fail");
static_assert(is_contiguous<const std::array<float, 4>&>::value, "fail");
static_assert(is_contiguous<int[5]>::value, "fail");
static_assert(is_contiguous<float(&)[1]>::value, "fail");
static_assert(!is_contiguous<std::list<char>>::value, "fail");

// TODO: boost provide something for this?
template <typename T>
struct range_traits
{
	// am I being stupid?
	template <typename U>
	static U return_t();

	typedef decltype(*std::begin(return_t<T>())) value_type;
};

template <typename T, typename U>
typename std::enable_if<is_contiguous<U>::value &&
	is_same_ignore_reference_cv<T, typename range_traits<U>::value_type>::value, U>::type
get_contiguous_range(U&& _range)
{
	return std::forward<U>(_range);
}

// TODO: should I really just copy to a vector if the data is of the wrong type?
template <typename T, typename U>
typename std::enable_if<!is_contiguous<U>::value ||
	!is_same_ignore_reference_cv<T, typename range_traits<U>::value_type>::value, std::vector<T>>::type
get_contiguous_range(U&& _range)
{
	return {std::begin(_range), std::end(_range)};

	static_assert(sizeof(T) == 0, "using this guy");
}

template <typename T, typename M>
std::intptr_t get_member_offset(M T::*_member)
{
	const T* const null_obj = nullptr;
	return reinterpret_cast<std::intptr_t>(&(null_obj->*_member));
}

}

/*
// TODO: do something when not contiguous?
template <typename IterIn, typename IterOut>
void copy(IterIn _begin, IterIn _end, IterOut _dest)
{

}
*/

template <typename T, typename Min, typename Max>
auto clamp(T&& _val, Min&& _min, Max&& _max) -> typename std::remove_reference<T>::type
{
	return _val < _min ? _min : (_val < _max ? _val : _max);
}

}
