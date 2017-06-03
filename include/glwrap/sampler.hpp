
#pragma once

#include "native_handle.hpp"
#include "constants.hpp"

namespace GLWRAP_NAMESPACE
{

class context;

enum class wrap_mode : GLenum
{
	clamp_to_edge = GL_CLAMP_TO_EDGE,
	mirrored_repeat = GL_MIRRORED_REPEAT,
	repeat = GL_REPEAT
};

enum class texture_filter : GLenum
{
	nearest = GL_NEAREST,
	linear = GL_LINEAR,
};

enum class mipmap_filter
{
	none = 0,
	nearest = GL_NEAREST_MIPMAP_NEAREST - GL_NEAREST,
	linear = GL_NEAREST_MIPMAP_LINEAR - GL_NEAREST,
};

// Hax, making use of GL constant values.
static_assert((mipmap_filter)0x100 == mipmap_filter::nearest, "Sanity Check");
static_assert((mipmap_filter)0x102 == mipmap_filter::linear, "Sanity Check");

namespace detail
{

constexpr int_t get_texture_filter_enum(texture_filter _tex_filter, mipmap_filter _mip_filter = mipmap_filter::none)
{
	return static_cast<int_t>(_tex_filter) | static_cast<int_t>(_mip_filter);
}

static_assert(get_texture_filter_enum(texture_filter::linear, mipmap_filter::linear) == GL_LINEAR_MIPMAP_LINEAR, "Sanity Check");

struct sampler_obj
{
	static void create_objs(sizei_t _n, uint_t* _objs)
	{
		if (GL_ARB_direct_state_access)
		{
			GLWRAP_GL_CALL(glCreateSamplers)(_n, _objs);
		}
		else
		{
			GLWRAP_GL_CALL(glGenSamplers)(_n, _objs);
		}
	}

	static void delete_objs(sizei_t _n, uint_t* _objs)
	{
		GLWRAP_GL_CALL(glDeleteSamplers)(_n, _objs);
	}
};

}

class sampler : public detail::globject<detail::sampler_obj>
{
public:
	explicit sampler(context&)
	{}

	void set_wrap_s(wrap_mode _mode)
	{
		set_parameter_raw(GL_TEXTURE_WRAP_S, static_cast<int>(_mode));
	}

	void set_wrap_t(wrap_mode _mode)
	{
		set_parameter_raw(GL_TEXTURE_WRAP_T, static_cast<int>(_mode));
	}

	void set_wrap_r(wrap_mode _mode)
	{
		set_parameter_raw(GL_TEXTURE_WRAP_R, static_cast<int>(_mode));
	}

	void set_min_filter(texture_filter _tex, mipmap_filter _mip)
	{
		set_parameter_raw(GL_TEXTURE_MIN_FILTER, detail::get_texture_filter_enum(_tex, _mip));
	}

	void set_mag_filter(texture_filter _tex)
	{
		set_parameter_raw(GL_TEXTURE_MAG_FILTER, detail::get_texture_filter_enum(_tex));
	}

	void set_min_lod(float_t _lod)
	{
		set_parameter_raw(GL_TEXTURE_MIN_LOD, _lod);
	}

	void set_max_lod(float_t _lod)
	{
		set_parameter_raw(GL_TEXTURE_MAX_LOD, _lod);
	}

	void set_lod_bias(float_t _bias)
	{
		set_parameter_raw(GL_TEXTURE_LOD_BIAS, _bias);
	}

	// TODO: name with *_ext ?
	void set_max_anisotropy(float_t _anisotropy)
	{
		// TODO: query GL_EXT_texture_filter_anisotropic for availability
		// TODO: query GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT for maximum value
		
		set_parameter_raw(GL_TEXTURE_MAX_ANISOTROPY_EXT, _anisotropy);
	}

	// TODO: GL_TEXTURE_BORDER_COLOR
	// TODO: GL_TEXTURE_COMPARE_MODE
	// TODO: GL_TEXTURE_COMPARE_FUNC

private:
	void set_parameter_raw(GLenum _pname, int_t _val)
	{
		GLWRAP_GL_CALL(glSamplerParameteri)(native_handle(), _pname, _val);
	}

	void set_parameter_raw(GLenum _pname, float_t _val)
	{
		GLWRAP_GL_CALL(glSamplerParameterf)(native_handle(), _pname, _val);
	}
};

}
