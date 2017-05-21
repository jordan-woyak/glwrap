#pragma once

#include <GL/glew.h>

namespace gl
{

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
	explicit native_handle_base()
		: m_native_handle()
	{
		obj_generator::create_objs(1, &m_native_handle);
	}

	// TODO: make this more explicit with a dummy parameter
	explicit native_handle_base(native_handle_type _handle)
		: m_native_handle(_handle)
	{}

	~native_handle_base()
	{
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
