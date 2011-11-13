#pragma once

#include "vector.hpp"
#include "native_handle.hpp"
#include "util.hpp"
#include "array_buffer.hpp"
#include "unpack_buffer.hpp"

namespace gl
{

class context;

template <int D>
class texture : public native_handle_base<GLuint>
{
	friend class context;

public:
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

	static GLenum get_target();

private:
	void bind() const
	{
		glBindTexture(get_target(), native_handle());
	}
};

typedef texture<1> texture_1d;
typedef texture<2> texture_2d;
typedef texture<3> texture_3d;

template <>
GLenum texture<1>::get_target()
{
	return GL_TEXTURE_1D;
}

template <>
GLenum texture<2>::get_target()
{
	return GL_TEXTURE_2D;
}

template <>
GLenum texture<3>::get_target()
{
	return GL_TEXTURE_3D;
}

template <int D>
struct bound_texture
{
public:
	int_t get_unit() const
	{
		return m_unit;
	}

private:
	friend class context;

	explicit bound_texture(int_t _unit)
		: m_unit(_unit)
	{}

	bound_texture(const bound_texture&) = default;
	bound_texture& operator=(const bound_texture&) = delete;

	int_t const m_unit;
};

}

