
#pragma once

#include <cstdint>
#include <vector>
#include <list>

#include <chrono>

#include <GL/glew.h>

#include "vector.hpp"
#include "exception.hpp"

namespace GLWRAP_NAMESPACE
{

// Zero error checking
//#define GLWRAP_GL_CALL(x) x

//#define GLWRAP_GL_CALL(x) detail::make_gl_function_caller(x, __PRETTY_FUNCTION__)
#define GLWRAP_GL_CALL(x) detail::make_gl_function_caller(x, #x) 

template <typename T>
bool inline is_extension_present(const T& _ext)
{
#if 0
	return _ext;
#else
	return false;
#endif
}

namespace detail
{

// TODO: naming?
struct adopt_value_t {};
constexpr adopt_value_t adopt_value {};

// TODO: poorly named
// TODO: I'd like to kill this.
template <typename T>
class scoped_value
{
public:
	scoped_value(const scoped_value&) = delete;
	scoped_value& operator=(const scoped_value&) = delete;

	explicit scoped_value(adopt_value_t, const typename T::value_type& _val = {})
		: m_prev_value(_val)
		, m_current_value(_val)
	{}

	explicit scoped_value(const typename T::value_type& _val)
		: m_prev_value(T::get())
		, m_current_value(m_prev_value)
	{
		set(_val);
	}

	scoped_value(scoped_value&& _other)
		: m_prev_value(_other.m_prev_value)
		, m_current_value(_other.m_current_value)
	{
		// _other will no longer unbind on destruction
		_other.m_current_value = m_prev_value;
	}

	void set(const typename T::value_type& _val)
	{
		if (_val != m_current_value)
		{
			m_current_value = _val;
			T::set(m_current_value);
		}
	}

	~scoped_value()
	{
		if (m_prev_value != m_current_value)
		{
			T::set(m_prev_value);
		}
	}

private:
	typename T::value_type const m_prev_value;
	typename T::value_type m_current_value;
};

template <typename T>
typename std::enable_if<std::is_arithmetic<T>::value, T>::type* value_ptr(T& _val)
{
	return &_val;
}

void check_error(const char* func_name)
{
	GLenum const err = glGetError();

	if (GL_NO_ERROR != err)
	{
		exception ex(err);

#if 0
		throw ex;
#else
		std::cerr << "glGetError: " << ex.get_string() << " : " << func_name << std::endl;
		exit(1);
#endif
	}
}

template <typename R, typename... Args>
class gl_function_caller
{
public:
	gl_function_caller(R(*_func)(Args...), const char* _name)
		: m_func(_func)
		, m_name(_name)
	{}

	// TODO: fowarding might not be perfect.
	// .. not that it needs to be for glFunctions
	R operator()(Args... _args) const
	{
		typedef std::chrono::high_resolution_clock clock;
		
		struct dtor_checker
		{
			const char* name;
			clock::time_point start_time;

			dtor_checker(const char* _name)
				: name(_name)
			{
				if (g_profile_every_gl_call)
				{
					start_time = clock::now();
				}
			}
			
			~dtor_checker()
			{
				if (g_profile_every_gl_call)
				{
					//glFlush();
					//glFinish();
					
					auto end_time = clock::now();
					auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);

					// TODO: do more than just print the timings, keep a log
					std::cerr << "Time: " << name << " : " << duration.count() << " ns." << std::endl;
				}
				
				if (g_check_every_gl_call)
				{
					check_error(name);
				}
			}
		};

		dtor_checker checker(m_name);

		return m_func(std::forward<Args>(_args)...);;
	}

private:
	R(*m_func)(Args...);
	const char* m_name;
};

template <typename R, typename... Args>
gl_function_caller<R, Args...> make_gl_function_caller(R(*_func)(Args...), const char* _name)
{
	return {_func, _name};
}

template <typename T, typename Enable = void>
struct is_gl_integral : std::false_type {};

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

template <typename LFunc, typename GFunc>
std::string get_shader_string(uint_t _handle, LFunc&& _len_func, enum_t _param, GFunc&& _get_func)
{
	int_t str_length = {};
	_len_func(_handle, _param, &str_length);

	std::string str;

	if (str_length)
	{
		std::vector<GLchar> buf(str_length);

		sizei_t ret_length = {};
		_get_func(_handle, buf.size(), &ret_length, buf.data());

		str.assign(buf.begin(), buf.begin() + ret_length);
	}

	return str;
}

}

// TODO: detail namespace
// TODO: modify an index inside state?
// TODO: more members
template <typename State>
class indexing_iterator : public std::iterator<std::random_access_iterator_tag, typename State::value_type>
{
public:
	typedef typename State::index_type index_type;
	typedef typename State::value_type value_type;

	typedef std::intptr_t diff_type;
	
	indexing_iterator(index_type _index, const State& _state)
		: m_index(_index)
		, m_state(_state)
	{}

	indexing_iterator& operator++()
	{
		return *this += 1;
	}

	indexing_iterator& operator--()
	{
		return *this -= 1;
	}

	indexing_iterator& operator+=(diff_type _val)
	{
		m_index += _val;
		return *this;
	}

	indexing_iterator& operator-=(diff_type _val)
	{
		return *this += -_val;
	}

	value_type& operator*() const
	{
		return m_state.deref(m_index);
	}

	value_type* operator->() const
	{
		return &**this;
	}

	bool operator==(indexing_iterator const& _rhs) const
	{
		return m_index == _rhs.m_index;
	}

	bool operator!=(indexing_iterator const& _rhs) const
	{
		return !(*this == _rhs);
	}

	friend diff_type operator-(indexing_iterator const& _lhs, indexing_iterator const& _rhs)
	{
		return _lhs.m_index - _rhs.m_index;
	}

	friend indexing_iterator operator+(indexing_iterator _lhs, diff_type _offset)
	{
		return _lhs += _offset;
	}

	friend indexing_iterator operator-(const indexing_iterator& _lhs, diff_type _offset)
	{
		return _lhs + -_offset;
	}

private:
	index_type m_index;
	State m_state;
};

namespace detail
{

template <typename I, typename B, typename T>
class typed_index
{
public:
	typedef I index_type;
	typedef B binding_type;
	typedef T value_type;

	typed_index(index_type _index)
		: m_index(_index)
	{}

	index_type get_index() const
	{
		return m_index;
	}

private:
	index_type m_index;
};

template <typename I, typename B, typename T>
class typed_index_attribute
{
public:
	typedef I index_type;
	typedef B binding_type;
	typedef T value_type;

	typed_index_attribute(index_type _index, uint_t _offset = {})
		: m_index(_index)
		, m_offset(_offset)
	{}

	index_type get_index() const
	{
		return m_index;
	}

	uint_t get_offset() const
	{
		return m_offset;
	}

private:
	index_type m_index;
	uint_t m_offset;
};

template <typename I, typename B, typename T, typename M>
typed_index_attribute<I, B, M> operator|(typed_index_attribute<I, B, T> const& _attr, M T::*_member)
{
	// TODO: only do this for types that need aligning:
	//static_assert((detail::get_member_offset(_member) % sizeof(M)) == 0, "Member is not aligned.");
	
	return typed_index_attribute<I, B, M>(_attr.get_index(), _attr.get_offset() + detail::get_member_offset(_member));
}

template <typename B, typename T>
using buffer_index = typed_index<uint_t, B, T>;

// TODO: kill this?
template <typename B, typename T>
using buffer_index_attribute = typed_index_attribute<uint_t, B, T>;

}

}
