#pragma once

#include <map>

namespace gl
{

enum class base_format
{
	r = GL_RED,
	rg = GL_RG,
	rgba = GL_RGBA,
	d = GL_DEPTH_COMPONENT
};

enum class data_type
{
	unsigned_normalized,
	signed_normalized,
	float_t,
	signed_integral,
	unsigned_integral
};

struct image_format
{
	image_format(base_format _bfmt)
		: value(static_cast<GLenum>(_bfmt))
	{}

	// TODO: privacy
	image_format(GLenum _value)
		: value(_value)
	{}

	GLenum value;
};

image_format base_image_format(base_format _bfmt)
{
	return {_bfmt};
}

/*
image_format sized_image_format(base_format _bfmt, int _bits)
{
	return {};
}
*/

#if 0
image_format specific_image_format(base_format _bfmt, data_type _dtype, int _bits)
{
	// TODO: initialize elsewhere?
	struct fmt_params
	{
		base_format bfmt;
		data_type dtype;
		int bits;

		bool operator<(fmt_params const& _rhs) const
		{
			// TODO: this is silly
			typedef std::tuple<base_format, data_type, int> tpl;

			tpl t(bfmt, dtype, bits), rhs(_rhs.bfmt, _rhs.dtype, _rhs.bits);
			return t < rhs;
		}
	};

	auto const r = base_format::r;
	auto const rg = base_format::rg;
	auto const rgba = base_format::rgba;

	//auto const unorm = data_type::unsigned_normalized;
	auto const flt = data_type::float_t;
	auto const sint = data_type::signed_integral;
	auto const uint = data_type::unsigned_integral;

	std::map<fmt_params, GLenum> supported_formats =
	{
		// unsigned normalized
		/*
		{{r, unorm, 8}, GL_R8_UNORM},
		{{rg, unorm, 8}, GL_RG8_UNORM},
		{{rgba, unorm, 8}, GL_RGBA8_UNORM},
		{{r, unorm, 16}, GL_R8_UNORM},
		{{rg, unorm, 16}, GL_RG8_UNORM},
		{{rgba, unorm, 16}, GL_RGBA8_UNORM},
		*/

		// float
		{{r, flt, 16}, GL_R16F},
		{{rg, flt, 16}, GL_RG16F},
		{{rgba, flt, 16}, GL_RGBA16F},
		{{r, flt, 32}, GL_R32F},
		{{rg, flt, 32}, GL_RG32F},
		{{rgba, flt, 32}, GL_RGBA32F},

		// signed integral
		{{r, sint, 8}, GL_R8I},
		{{rg, sint, 8}, GL_RG8I},
		{{rgba, sint, 8}, GL_RGBA8I},
		{{r, sint, 16}, GL_R16I},
		{{rg, sint, 16}, GL_RG16I},
		{{rgba, sint, 16}, GL_RGBA16I},
		{{r, sint, 32}, GL_R32I},
		{{rg, sint, 32}, GL_RG32I},
		{{rgba, sint, 32}, GL_RGBA32I},

		// unsigned integral
		{{r, uint, 8}, GL_R8UI},
		{{rg, uint, 8}, GL_RG8UI},
		{{rgba, uint, 8}, GL_RGBA8UI},
		{{r, uint, 16}, GL_R16UI},
		{{rg, uint, 16}, GL_RG16UI},
		{{rgba, uint, 16}, GL_RGBA16UI},
		{{r, uint, 32}, GL_R32UI},
		{{rg, uint, 32}, GL_RG32UI},
		{{rgba, uint, 32}, GL_RGBA32UI},
	};

	auto it = supported_formats.find({_bfmt, _dtype, _bits});
	if (it != supported_formats.end());
		return {it->second};

	// TODO: throw if invalid?
	return {};
}
#endif
}
