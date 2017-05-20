
#pragma once

#include <cstdint>
#include <vector>
#include <list>

#include <GL/glew.h>

#include "vector.hpp"
#include "exception.hpp"

namespace gl
{

// Zero error checking
//#define GLWRAP_EC_CALL(x) x

//#define GLWRAP_EC_CALL(x) detail::make_gl_function_caller(x, __PRETTY_FUNCTION__)
#define GLWRAP_EC_CALL(x) detail::make_gl_function_caller(x, #x) 

namespace detail
{

// TODO: poorly named
// TODO: I'd like to kill this.
template <typename T>
class scoped_value
{
public:
	scoped_value(const scoped_value&) = delete;
	scoped_value& operator=(const scoped_value&) = delete;

	explicit scoped_value(const typename T::value_type& _val)
		: m_prev_value(T::get())
		, m_desired_value(_val)
	{
		if (m_prev_value != m_desired_value)
		{
			T::set(m_desired_value);
		}
	}

	scoped_value(scoped_value&& _other)
		: m_prev_value(_other.m_prev_value)
		, m_desired_value(_other.m_desired_value)
	{
		// _other will no longer unbind on destruction
		_other.m_desired_value = m_prev_value;
	}

	~scoped_value()
	{
		if (m_prev_value != m_desired_value)
		{
			T::set(m_prev_value);
		}
	}

private:
	typename T::value_type const m_prev_value;
	typename T::value_type m_desired_value;
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
		std::cerr << "error: " << ex.get_string() << " : " << func_name << std::endl;
		exit(1);
#endif
	}
}

// TODO: use/kill this?
// TODO: have varying levels of unlikeliness
GLWRAP_FUNC_DECL
void
check_unlikely_error()
{
	//check_error();
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
		struct dtor_checker
		{
			const char* name;
			
			~dtor_checker()
			{
				check_error(name);
			}
		};

		dtor_checker checker{m_name};

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

// TODO: move this
typedef decltype(glGenBuffers) glgenfunc;
inline GLuint gen_return(glgenfunc _glFunc)
{
	GLuint name;
	GLWRAP_EC_CALL(_glFunc)(1, &name);
	return name;
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

// TODO: half_t
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
	static_assert(sizeof(T) == 0, "using this guy");
	
	return {std::begin(_range), std::end(_range)};
}

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

// TODO: something better than int for offsets
// TODO: detail namespace
// TODO: modify an index inside state?
template <typename State>
class indexing_iterator : public std::iterator<std::random_access_iterator_tag, typename State::value_type>
{
public:
	typedef typename State::index_type index_type;
	typedef typename State::value_type value_type;
	
	indexing_iterator(index_type _index, const State& _state)
		: m_index(_index)
		, m_state(_state)
	{}

	indexing_iterator& operator++()
	{
		return *this += 1;
	}

	indexing_iterator& operator+=(int _val)
	{
		m_index += _val;
		return *this;
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

	// TODO: member or free func?
	friend indexing_iterator operator+(const indexing_iterator& _lhs, int _offset)
	{
		return _lhs += _offset;
	}

private:
	index_type m_index;
	State m_state;
};

// This is used for atomic counter, transform feedback, and vertex array
template <template<typename> typename B, typename T>
class binding_attribute
{
public:
	binding_attribute(int_t _index, uint_t _offset = {})
		: m_index(_index)
		, m_offset(_offset)
	{}

	int_t get_index() const
	{
		return m_index;
	}

	uint_t get_offset() const
	{
		return m_offset;
	}

private:
	int_t m_index;
	uint_t m_offset;
};

template <template<typename> typename B, typename T, typename M>
binding_attribute<B, M> operator|(binding_attribute<B, T> const& _attr, M T::*_member)
{
	// TODO: only do this for types that need aligning:
	//static_assert((detail::get_member_offset(_member) % sizeof(M)) == 0, "Member is not aligned.");
	
	return binding_attribute<B, M>(_attr.get_index(), _attr.get_offset() + detail::get_member_offset(_member));
}

}
