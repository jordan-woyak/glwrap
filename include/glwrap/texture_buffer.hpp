#pragma once

#include "native_handle.hpp"

namespace gl
{

class context;

class texture_buffer : public globject
{
	friend class context;

public:
	explicit texture_buffer(context& _context)
		: globject(gen_return(glGenTextures))
	{}

	~texture_buffer()
	{
		auto const nh = native_handle();
		glDeleteTextures(1, &nh);
	}

	// TODO: internal format
	/*
	template <typename T>
	void bind_buffer(array_buffer<T> const& _buffer)
	{
		bind();
		glTexBuffer(GL_TEXTURE_BUFFER, 0, _buffer.native_handle());
	}
	*/

private:
	void bind() const
	{
		glBindTexture(GL_TEXTURE_BUFFER, native_handle());
	}
};

}
