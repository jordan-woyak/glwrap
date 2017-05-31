#pragma once

#include <GL/glew.h>

namespace GLWRAP_NAMESPACE
{

struct adopt_handle_t {};
constexpr adopt_handle_t adopt_handle {}; 

namespace detail
{

template <typename N, typename Generator>
class native_handle_base
{
public:
	typedef N native_handle_type;
	typedef Generator obj_generator;

	native_handle_type native_handle() const
	{
		return m_native_handle;
	}

protected:
	template <typename... T>
	explicit native_handle_base(T&&... _args)
		: m_native_handle()
	{
		obj_generator::create_objs(std::forward<T>(_args)..., 1, &m_native_handle);
	}

	explicit native_handle_base(native_handle_type _handle, adopt_handle_t)
		: m_native_handle(_handle)
	{}

	~native_handle_base()
	{
		// TODO: test for zero value?
		
		obj_generator::delete_objs(1, &m_native_handle);
	}

	native_handle_base(const native_handle_base&) = delete;
	native_handle_base& operator=(const native_handle_base&) = delete;

	native_handle_base(native_handle_base&& _other)
		: m_native_handle()
	{
		swap(_other);
	}

	native_handle_base& operator=(native_handle_base&& _other)
	{
		// TODO: this is ugly:
		swap(_other);
		return *this;
	}

	void swap(native_handle_base& _other)
	{
		std::swap(m_native_handle, _other.m_native_handle);
	}

private:
	native_handle_type m_native_handle;
};

template <typename Generator>
using globject = native_handle_base<GLuint, Generator>;

}

}
