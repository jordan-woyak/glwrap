
#pragma once

#include <cstdint>

#include <chrono>

#include <GL/glew.h>

#include "vector.hpp"
#include "exception.hpp"
#include "detail/traits.hpp"

namespace GLWRAP_NAMESPACE
{

// Zero error checking
//#define GLWRAP_GL_CALL(x) x

//#define GLWRAP_GL_CALL(x) detail::make_gl_function_caller(x, __PRETTY_FUNCTION__)
#define GLWRAP_GL_CALL(x) detail::make_gl_function_caller(x, #x) 

bool inline is_extension_present(bool _ext)
{
#if 1
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
					glFlush();
					glFinish();
					
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
