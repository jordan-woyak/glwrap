#pragma once

#include "vector.hpp"
#include "sampler.hpp"
#include "native_handle.hpp"
#include "util.hpp"

namespace gl
{

class context;

template <int D>
class texture : public sampler<D>, public native_handle_base<GLuint>
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

	explicit texture(device& _context)
		: native_handle_base<GLuint>(gen_return(glGenTextures))
	{}

private:
	void bind_texture_unit(int_t _unit)
	{
		glActiveTexture(GL_TEXTURE0 + _unit);
		bind();
	}

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

