#pragma once

#include "vector.hpp"
#include "native_handle.hpp"
#include "util.hpp"
#include "buffer.hpp"
#include "unpack_buffer.hpp"

namespace gl
{

class context;

template <int D>
class texture : public native_handle_base<GLuint>
{
	friend class context;

public:
	texture(const texture&) = delete;
	texture& operator=(const texture&) = delete;

	texture(texture&&) = default;
	texture& operator=(texture&&) = default;

	~texture()
	{
		auto const nh = native_handle();
		glDeleteTextures(1, &nh);
	}

	explicit texture(context& _context)
		: native_handle_base<GLuint>(gen_return(glGenTextures))
	{}

	template <typename T>
	void assign(unpack_buffer<T, D> const& _buffer)
	{
		bind();
		// TODO: super lame
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _buffer.m_dims.x, _buffer.m_dims.y,
			0, static_cast<GLenum>(_buffer.m_pfmt), detail::data_type_enum<T>(), _buffer.m_data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, 0);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

private:
	void bind();
};

typedef texture<1> texture_1d;
typedef texture<2> texture_2d;
typedef texture<3> texture_3d;

template <>
void texture<1>::bind()
{
	glBindTexture(GL_TEXTURE_1D, native_handle());
}

template <>
void texture<2>::bind()
{
	glBindTexture(GL_TEXTURE_2D, native_handle());
}

template <>
void texture<3>::bind()
{
	glBindTexture(GL_TEXTURE_3D, native_handle());
}

}

