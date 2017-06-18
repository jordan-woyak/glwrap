#pragma once

#include <map>

namespace GLWRAP_NAMESPACE
{

enum class pixel_format : enum_t
{
	r = GL_RED,
	rg = GL_RG,
	rgb = GL_RGB,
	rgba = GL_RGBA,
	//depth_component = GL_DEPTH_COMPONENT,
	//depth_stencil = GL_DEPTH_STENCIL,
	//stencil_index = GL_STENCIL_INDEX,

	// TODO: GL_RGBA_INTEGER and family need to be used for non-normalized formats
};

#define GLWRAP_ENUM_DEF(dtype, bitdepth, suffix) \
	r##bitdepth##dtype = GL_R##bitdepth##suffix, \
	rg##bitdepth##dtype = GL_RG##bitdepth##suffix, \
	rgb##bitdepth##dtype = GL_RGB##bitdepth##suffix, \
	rgba##bitdepth##dtype = GL_RGBA##bitdepth##suffix, \

// TODO: rename to *_image_format
enum class normalized_image_format : enum_t
{
	// TODO: suffix? "u" or "".. or both?
	GLWRAP_ENUM_DEF(u, 8, )
	GLWRAP_ENUM_DEF(u, 16, )

	GLWRAP_ENUM_DEF(i, 8, _SNORM)
	GLWRAP_ENUM_DEF(i, 16, _SNORM)

	GLWRAP_ENUM_DEF(f, 16, F)
	GLWRAP_ENUM_DEF(f, 32, F)

	r5u_g6u_b5u = GL_RGB565,
	r11f_g11f_b10f = GL_R11F_G11F_B10F,
	
	//rgb9u_e5 = GL_RGB9_E5,
	//srgb8_a8 = GL_SRGB8_ALPHA8,
	//GL_SRGB8
	
	rgb5u_a1u = GL_RGB5_A1,
	rgb10u_a2u = GL_RGB10_A2,
};

// TODO: Do I like these names?
enum class depth_format : enum_t
{
	u16 = GL_DEPTH_COMPONENT16,
	u24 = GL_DEPTH_COMPONENT24,
	f32 = GL_DEPTH_COMPONENT32F,
};

enum class depth_stencil_format : enum_t
{
	u24u8 = GL_DEPTH24_STENCIL8,
	f32u8 = GL_DEPTH32F_STENCIL8,
};

enum class stencil_format : enum_t
{
	u8 =  GL_STENCIL_INDEX8,
};

enum class signed_image_format : enum_t
{
	GLWRAP_ENUM_DEF(i, 8, I)
	GLWRAP_ENUM_DEF(i, 16, I)
	GLWRAP_ENUM_DEF(i, 32, I)
};

enum class unsigned_image_format : enum_t
{
	// u or ui ?
	GLWRAP_ENUM_DEF(ui, 8, UI)
	GLWRAP_ENUM_DEF(ui, 16, UI)
	GLWRAP_ENUM_DEF(ui, 32, UI)

	rgb10ui_a2ui = GL_RGB10_A2UI,
};

enum class compressed_image_format : enum_t
{
	r11_eac = GL_COMPRESSED_R11_EAC,
	signed_r11_eac = GL_COMPRESSED_SIGNED_R11_EAC,
	rg11_eac = GL_COMPRESSED_RG11_EAC,
	signed_rg11_eac = GL_COMPRESSED_SIGNED_RG11_EAC,
	rgb8_etc2 = GL_COMPRESSED_RGB8_ETC2,
	srgb8_etc2 = GL_COMPRESSED_SRGB8_ETC2,
	rgb8_punchthrough_alpha1_etc2 = GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2,
	srgb8_punchthrough_alpha1_etc2 = GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2,
	rgba8_etc2_eac = GL_COMPRESSED_RGBA8_ETC2_EAC,
	srgb8_alpha8_etc2_eac = GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC,

	// EXT_texture_compression_s3tc
	rgb_s3tc_dxt1 = GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
	rgba_s3tc_dxt1 = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
	rgba_s3tc_dxt3 = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,
	rgba_s3tc_dxt5 = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,

	// EXT_texture_sRGB
	srgb_s3tc_dxt1 = GL_COMPRESSED_SRGB_S3TC_DXT1_EXT,
	srgb_alpha_s3tc_dxt1 = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,
	srgb_alpha_s3tc_dxt3 = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,
	srgb_alpha_s3tc_dxt5 = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,
};

#undef GLWRAP_ENUM_DEF

// TODO: kill this template?
template <typename DataType>
struct image_format;

template <>
struct image_format<float_t>
{
	typedef normalized_image_format enum_type;
	
	enum_type value;
};

template <>
struct image_format<int_t>
{
	typedef signed_image_format enum_type;
	
	enum_type value;
};

template <>
struct image_format<uint_t>
{
	typedef unsigned_image_format enum_type;
	
	enum_type value;
};

namespace detail
{

#define GLWRAP_FORMAT_STR_DEF(enum_suffix, str_suffix) \
	case (GL_RGBA##enum_suffix): return "rgba" #str_suffix; break; \
	case (GL_RG##enum_suffix): return "rg" #str_suffix; break; \
	case (GL_R##enum_suffix): return "r" #str_suffix; break;

inline const char* format_qualifier_string(normalized_image_format _fmt)
{		
	switch (static_cast<enum_t>(_fmt))
	{
	GLWRAP_FORMAT_STR_DEF(32F, 32f)
	GLWRAP_FORMAT_STR_DEF(16F, 16f)

	GLWRAP_FORMAT_STR_DEF(16, 16)
	GLWRAP_FORMAT_STR_DEF(8, 8)

	GLWRAP_FORMAT_STR_DEF(16_SNORM, 16_snorm)
	GLWRAP_FORMAT_STR_DEF(8_SNORM, 8_snorm)
		
	case (GL_R11F_G11F_B10F): return "r11f_g11f_b10f"; break;
	case (GL_RGB10_A2): return "rgb10_a2"; break;

	default:
		// TODO: throw exception?
		return 0;
		break;
	}
}

inline const char* format_qualifier_string(signed_image_format _fmt)
{
	switch (static_cast<enum_t>(_fmt))
	{
	GLWRAP_FORMAT_STR_DEF(32I, 32i)
	GLWRAP_FORMAT_STR_DEF(16I, 16i)
	GLWRAP_FORMAT_STR_DEF(8I, 8i)
	
	default:
		// TODO: throw exception?
		return 0;
		break;
	}
}

inline const char* format_qualifier_string(unsigned_image_format _fmt)
{
	switch (static_cast<enum_t>(_fmt))
	{
	GLWRAP_FORMAT_STR_DEF(32UI, 32ui)
	GLWRAP_FORMAT_STR_DEF(16UI, 16ui)
	GLWRAP_FORMAT_STR_DEF(8UI, 8ui)

	case (GL_RGB10_A2UI): return "rgb10_a2ui"; break;
	
	default:
		// TODO: throw exception?
		return 0;
		break;
	}
}

#undef GLWRAP_FORMAT_STR_DEF

}

}
