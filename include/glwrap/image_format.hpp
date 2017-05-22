#pragma once

#include <map>

namespace GLWRAP_NAMESPACE
{

enum class base_format
{
	r = GL_RED,
	rg = GL_RG,
	rgb = GL_RGB,
	rgba = GL_RGBA,
	d = GL_DEPTH_COMPONENT

	// TODO: GL_RGBA_INTEGER and family need to be used for non-normalized formats
};

enum class data_type
{
	unsigned_normalized,
	signed_normalized,
	floating_point,
	signed_integral,
	unsigned_integral
};

#define GLWRAP_ENUM_DEF(dtype, bitdepth, suffix) \
	r##bitdepth##dtype = GL_R##bitdepth##suffix, \
	rg##bitdepth##dtype = GL_RG##bitdepth##suffix, \
	rgb##bitdepth##dtype = GL_RGB##bitdepth##suffix, \
	rgba##bitdepth##dtype = GL_RGBA##bitdepth##suffix, \

enum class normalized_internal_format
{
	GLWRAP_ENUM_DEF(u, 8, )
	GLWRAP_ENUM_DEF(u, 16, )

	GLWRAP_ENUM_DEF(s, 8, _SNORM)
	GLWRAP_ENUM_DEF(s, 16, _SNORM)

	GLWRAP_ENUM_DEF(f, 16, F)
	GLWRAP_ENUM_DEF(f, 32, F)
};

enum class signed_internal_format
{
	GLWRAP_ENUM_DEF(i, 8, I)
	GLWRAP_ENUM_DEF(i, 16, I)
	GLWRAP_ENUM_DEF(i, 32, I)
};

enum class unsigned_internal_format
{
	GLWRAP_ENUM_DEF(ui, 8, UI)
	GLWRAP_ENUM_DEF(ui, 16, UI)
	GLWRAP_ENUM_DEF(ui, 32, UI)
};

#undef GLWRAP_ENUM_DEF

// TODO: kill this template?
template <typename DataType>
struct internal_format;

template <>
struct internal_format<float_t>
{
	typedef normalized_internal_format enum_type;
	
	enum_type value;
};

template <>
struct internal_format<int_t>
{
	typedef signed_internal_format enum_type;
	
	enum_type value;
};

template <>
struct internal_format<uint_t>
{
	typedef unsigned_internal_format enum_type;
	
	enum_type value;
};

/*
template <typename DataType>
internal_format<DataType> get_internal_format()
{
	return {};
}
*/

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

inline image_format base_image_format(base_format _bfmt)
{
	return {_bfmt};
}

/*
inline image_format sized_image_format(base_format _bfmt, int _bits)
{
	return {};
}
*/

inline image_format specific_image_format(base_format _bfmt, data_type _dtype, int _bits)
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
	auto const rgb = base_format::rgb;
	auto const rgba = base_format::rgba;

	auto const unorm = data_type::unsigned_normalized;
	auto const snorm = data_type::signed_normalized;
	auto const flt = data_type::floating_point;
	auto const sint = data_type::signed_integral;
	auto const uint = data_type::unsigned_integral;

#define GLWRAP_FORMAT_DEF(dtype, bitdepth, suffix) \
	{{r, dtype, bitdepth}, GL_R##bitdepth##suffix}, \
	{{rg, dtype, bitdepth}, GL_RG##bitdepth##suffix}, \
	{{rgb, dtype, bitdepth}, GL_RGB##bitdepth##suffix}, \
	{{rgba, dtype, bitdepth}, GL_RGBA##bitdepth##suffix},

	std::map<fmt_params, GLenum> supported_formats =
	{
		// unsigned normalized
		GLWRAP_FORMAT_DEF(unorm, 8, )
		GLWRAP_FORMAT_DEF(unorm, 16, )
		
		// signed normalized
		GLWRAP_FORMAT_DEF(snorm, 8, _SNORM)
		GLWRAP_FORMAT_DEF(snorm, 16, _SNORM)
		
		// float
		GLWRAP_FORMAT_DEF(flt, 16, F)
		GLWRAP_FORMAT_DEF(flt, 32, F)

		// signed integral
		GLWRAP_FORMAT_DEF(sint, 8, I)
		GLWRAP_FORMAT_DEF(sint, 16, I)
		GLWRAP_FORMAT_DEF(sint, 32, I)

		// unsigned integral
		GLWRAP_FORMAT_DEF(uint, 8, UI)
		GLWRAP_FORMAT_DEF(uint, 16, UI)
		GLWRAP_FORMAT_DEF(uint, 32, UI)

		// TODO: the few odd ones..
	};

#undef GLWRAP_FORMAT_DEF

	auto it = supported_formats.find({_bfmt, _dtype, _bits});
	if (it != supported_formats.end())
		return {it->second};

	// TODO: throw if invalid?
	return {base_format()};
}

}
