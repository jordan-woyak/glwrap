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
enum class normalized_internal_format : enum_t
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
	rgb10ui_a2ui = GL_RGB10_A2UI,
};

enum class signed_internal_format : enum_t
{
	GLWRAP_ENUM_DEF(i, 8, I)
	GLWRAP_ENUM_DEF(i, 16, I)
	GLWRAP_ENUM_DEF(i, 32, I)
};

enum class unsigned_internal_format : enum_t
{
	// u or ui ?
	GLWRAP_ENUM_DEF(ui, 8, UI)
	GLWRAP_ENUM_DEF(ui, 16, UI)
	GLWRAP_ENUM_DEF(ui, 32, UI)
};

enum class compressed_internal_format : enum_t
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
// TODO: rename image_format?
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

namespace detail
{

// TODO: separate function for each of the 3 image format types
inline const char* format_qualifier_string(enum_t _format)
{

#define GLWRAP_FORMAT_STR_DEF(enum_suffix, str_suffix) \
	case (GL_RGBA##enum_suffix): return "rgba" #str_suffix; break; \
	case (GL_RG##enum_suffix): return "rg" #str_suffix; break; \
	case (GL_R##enum_suffix): return "r" #str_suffix; break;
		
	switch (_format)
	{
	GLWRAP_FORMAT_STR_DEF(32F, 32f)
	GLWRAP_FORMAT_STR_DEF(16F, 16f)

	GLWRAP_FORMAT_STR_DEF(16, 16)
	GLWRAP_FORMAT_STR_DEF(8, 8)

	GLWRAP_FORMAT_STR_DEF(16_SNORM, 16_snorm)
	GLWRAP_FORMAT_STR_DEF(8_SNORM, 8_snorm)

	GLWRAP_FORMAT_STR_DEF(32UI, 32ui)
	GLWRAP_FORMAT_STR_DEF(16UI, 16ui)
	GLWRAP_FORMAT_STR_DEF(8UI, 8ui)

	GLWRAP_FORMAT_STR_DEF(32I, 32i)
	GLWRAP_FORMAT_STR_DEF(16I, 16i)
	GLWRAP_FORMAT_STR_DEF(8I, 8i)
		
	case (GL_R11F_G11F_B10F): return "r11f_g11f_b10f"; break;
	case (GL_RGB10_A2UI): return "rgb10_a2ui"; break;
	case (GL_RGB10_A2): return "rgb10_a2"; break;

	default:
		// TODO: throw exception?
		return 0;
		break;
	}

#undef GLWRAP_FORMAT_STR_DEF

}

}

}
