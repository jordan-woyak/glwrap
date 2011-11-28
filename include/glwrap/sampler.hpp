
#pragma once

#include "native_handle.hpp"
#include "constants.hpp"

namespace gl
{

class context;

enum class wrap_mode : GLenum
{
	clamp_to_edge = GL_CLAMP_TO_EDGE,
	mirrored_repeat = GL_MIRRORED_REPEAT,
	repeat = GL_REPEAT
};

/*
// TODO: make struct to combine pixel filter + mimmap filter, so only 2 enums
enum class filter_mode : GLenum
{
	nearest = GL_NEAREST,
	linear = GL_LINEAR,
	GL_NEAREST_MIPMAP_NEAREST,
	GL_LINEAR_MIPMAP_NEAREST,
	GL_NEAREST_MIPMAP_LINEAR,
	GL_LINEAR_MIPMAP_LINEAR,
};
*/

class sampler : public globject
{
	friend class context;

public:
	sampler(context& _context)
		: globject(gen_return(glGenSamplers))
	{}

	~sampler()
	{
		auto const nh = native_handle();
		glDeleteSamplers(1, &nh);
	}

	void set_wrap_s(wrap_mode _mode)
	{
		glSamplerParameteri(native_handle(), GL_TEXTURE_WRAP_S, static_cast<int>(_mode));
	}

	void set_wrap_t(wrap_mode _mode)
	{
		glSamplerParameteri(native_handle(), GL_TEXTURE_WRAP_T, static_cast<int>(_mode));
	}

	void set_wrap_r(wrap_mode _mode)
	{
		glSamplerParameteri(native_handle(), GL_TEXTURE_WRAP_R, static_cast<int>(_mode));
	}

	void set_min_filter(filter _mode)
	{
		glSamplerParameteri(native_handle(), GL_TEXTURE_MIN_FILTER, static_cast<int>(_mode));
	}

	void set_mag_filter(filter _mode)
	{
		glSamplerParameteri(native_handle(), GL_TEXTURE_MAG_FILTER, static_cast<int>(_mode));
	}

	void set_min_lod(float_t _lod)
	{
		glSamplerParameterf(native_handle(), GL_TEXTURE_MIN_LOD, _lod);
	}

	void set_max_lod(float_t _lod)
	{
		glSamplerParameterf(native_handle(), GL_TEXTURE_MAX_LOD, _lod);
	}

	// TODO: GL_TEXTURE_BORDER_COLOR
	// TODO: GL_TEXTURE_COMPARE_MODE
	// TODO: GL_TEXTURE_COMPARE_FUNC

private:
	void bind(GLuint _unit) const
	{
		glBindSampler(_unit, native_handle());
	}
};

}
