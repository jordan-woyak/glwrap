#pragma once

#include <GL/glew.h>

namespace gl
{

// TODO: put the handle Gen/Delete inside this class via templates
template <typename N>
class native_handle_base
{
public:
	typedef N native_handle_type;

	native_handle_type native_handle() const
	{
		return m_native_handle;
	}

protected:
	explicit native_handle_base(native_handle_type _native_handle)
		: m_native_handle(_native_handle)
	{}

	native_handle_base(const native_handle_base&) = delete;
	native_handle_base& operator=(const native_handle_base&) = delete;

protected:
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

typedef native_handle_base<GLuint> globject;

}
