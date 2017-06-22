#pragma once

#include "vector.hpp"
#include "native_handle.hpp"
#include "util.hpp"
#include "pixel_store.hpp"
#include "image_format.hpp"
#include "buffer.hpp"
#include "detail/texture.hpp"
#include "detail/context.hpp"

#include "sampler.hpp"

namespace GLWRAP_NAMESPACE
{

enum class swizzle_component : GLenum
{
	r = GL_TEXTURE_SWIZZLE_R,
	g = GL_TEXTURE_SWIZZLE_G,
	b = GL_TEXTURE_SWIZZLE_B,
	a = GL_TEXTURE_SWIZZLE_A,
	rgba = GL_TEXTURE_SWIZZLE_RGBA
};

enum class swizzle_value : GLenum
{
	r = GL_RED,
	g = GL_GREEN,
	b = GL_BLUE,
	a = GL_ALPHA,
	zero = GL_ZERO,
	one = GL_ONE,
};

class context;

namespace detail
{

struct texture_unit_index_traits
{
	static int_t get_index_count()
	{
		int_t val = 0;

		// TODO: This is the max per program..
		detail::gl_get(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &val);

		return val;
	}

	template <typename T>
	static int_t get_index_usage()
	{
		return 1;
	}
};

struct image_unit_index_traits
{
	static int_t get_index_count()
	{
		int_t val = 0;

		detail::gl_get(GL_MAX_IMAGE_UNITS, &val);

		return val;
	}

	template <typename T>
	static int_t get_index_usage()
	{
		return 1;
	}
};

}

template <typename T>
using texture_unit = detail::typed_index<int_t, detail::texture_unit_index_traits, T>;

typedef detail::typed_index_enumerator<detail::texture_unit_index_traits, texture_unit>
	texture_unit_enumerator;

template <typename T>
using image_unit = detail::typed_index<int_t, detail::image_unit_index_traits, T>;

typedef detail::typed_index_enumerator<detail::image_unit_index_traits, image_unit>
	image_unit_enumerator;

namespace texture_parameter
{

template <typename T>
struct parameter_base
{
	//typedef T value_type;
};

struct base_level : parameter_base<int_t>
{};

struct wrap_s : parameter_base<int_t>
{};

}

namespace detail
{

struct texture_obj
{
	static void create_objs(enum_t _target, sizei_t _n, uint_t* _objs)
	{
		if (is_extension_present(GL_ARB_direct_state_access))
		{
			GLWRAP_GL_CALL(glCreateTextures)(_target, _n, _objs);
		}
		else
		{
			GLWRAP_GL_CALL(glGenTextures)(_n, _objs);
		}
	}

	static void delete_objs(sizei_t _n, uint_t* _objs)
	{
		GLWRAP_GL_CALL(glDeleteTextures)(_n, _objs);
	}
};

}

// TODO: pair the texture type with the data type
// the pair of them appears all over the place

template <texture_type Type, typename DataType>
class basic_texture : public detail::globject<detail::texture_obj>
{
	friend class context;

public:
	static const texture_type type = Type;
	static const int dimensions = detail::texture_traits<type>::dimensions;
	typedef typename detail::texture_traits<type>::dimension_type dimension_type;

	// TODO: this is more of a detail..
	static const enum_t target = detail::texture_traits<type>::target;

	typedef typename image_format<DataType>::enum_type image_format_type;

	explicit basic_texture(context&)
		: detail::globject<detail::texture_obj>(static_cast<enum_t>(Type))
	{}

	// TODO: this can only be done once. do it in the constructor?
	// TODO: storage makes immutable textures. make this more obvious
	// TexStorage also sets the max mipmap level
	void define_storage(sizei_t _levels, image_format_type _ifmt, dimension_type const& _dims)
	{
		if (is_extension_present(GL_ARB_direct_state_access))
		{
			detail::gl_texture_storage<Type>(native_handle(), _levels, static_cast<enum_t>(_ifmt), _dims);
		}
		else
		{
			detail::scoped_value<detail::parameter::texture<Type>> binding(native_handle());

			detail::gl_tex_storage<Type>(_levels, static_cast<enum_t>(_ifmt), _dims);
		}
	}

	// TODO: Allow testing if this is available.
	// TODO: Change interface for multi sample and non-layered textures
	// TODO: Allow for other targets
	void define_view(basic_texture& _orig, image_format_type _ifmt, uint_t _min_level, uint_t _num_levels, uint_t _min_layer, uint_t _num_layers)
	{
		if (is_extension_present(GL_ARB_texture_view))
		{
			GLWRAP_GL_CALL(glTextureView)(native_handle(), target, _orig.native_handle(), static_cast<enum_t>(_ifmt),
				_min_level, _num_levels, _min_layer, _num_layers);
		}
		else
		{
			throw std::exception();
		}
	}

	// TODO: require unpack_buffer is of the proper type and has the right number of components

	template <typename T>
	void load_sub_image(int_t _level, dimension_type const& _offset, unpack_buffer<T, dimensions> const& _buffer)
	{
		_buffer.bind();

		const enum_t pfmt = get_pixel_format_enum(_buffer.m_pfmt);

		if (is_extension_present(GL_ARB_direct_state_access))
		{
			detail::gl_texture_sub_image<Type>(native_handle(), _level, _offset, _buffer.m_dims,
				pfmt, _buffer.m_data);
		}
		else
		{
			detail::scoped_value<detail::parameter::texture<Type>> binding(native_handle());

			detail::gl_tex_sub_image<Type>(_level, _offset, _buffer.m_dims,
				pfmt, _buffer.m_data);
		}
	}

	// TODO: take a value by-ref and assume the format from it.
	template <typename T>
	void clear_image(int_t _level, pixel_format _pfmt, const T* _data)
	{
		const enum_t pfmt = get_pixel_format_enum(_pfmt);

		if (is_extension_present(GL_ARB_clear_texture))
		{
			GLWRAP_GL_CALL(glClearTexImage)(native_handle(), _level,
				pfmt, detail::data_type_enum<T>(), _data);
		}
		else
		{
			// TODO: emulate glClearImage
			throw std::exception();
		}
	}

	// TODO: take a value by-ref and assume the format from it.
	template <typename T>
	void clear_sub_image(int_t _level, dimension_type const& _offset, dimension_type const& _size, pixel_format _pfmt, const T* _data)
	{
		const enum_t pfmt = get_pixel_format_enum(_pfmt);

		if (is_extension_present(GL_ARB_clear_texture))
		{
			ivec3 offset{0, 0, 0};
			ivec3 size{1, 1, 1};

			// TODO: can this be less messy..
			for (int i = 0; i != dimensions; ++i)
			{
				value_ptr(offset)[i] = value_ptr(_offset)[i];
				value_ptr(size)[i] = value_ptr(_size)[i];
			}

			GLWRAP_GL_CALL(glClearTexSubImage)(native_handle(), _level,
				offset.x, offset.y, offset.z,
				size.x, size.y, size.z,
				pfmt, detail::data_type_enum<T>(), _data);
		}
		else
		{
			// TODO: emulate glClearTexSubImage
			throw std::exception();
		}
	}

	// TODO: rename?
	// TODO: should this take and bind a framebuffer?
	void copy_sub_image(int_t _level, dimension_type const& _offset, const ivec2& _pos, const ivec2& _size)
	{
		if (is_extension_present(GL_ARB_direct_state_access))
		{
			detail::gl_copy_texture_sub_image<Type>(native_handle(), _level,
				_offset, _pos, _size);
		}
		else
		{
			detail::scoped_value<detail::parameter::texture<Type>> binding(native_handle());

			detail::gl_copy_tex_sub_image<Type>(_level,
				_offset, _pos, _size);
		}
	}

	// MAX_LEVEL is observed
	void generate_mipmaps()
	{
		if (is_extension_present(GL_ARB_direct_state_access))
		{
			GLWRAP_GL_CALL(glGenerateTextureMipmap)(native_handle());
		}
		else
		{
			// TODO: ugly
			detail::scoped_value<detail::parameter::texture<Type>> binding(native_handle());

			GLWRAP_GL_CALL(glGenerateMipmap)(target);
		}
	}

	void set_swizzle(swizzle_component _comp, swizzle_value _val)
	{
		set_parameter_raw(static_cast<GLenum>(_comp), static_cast<int_t>(_val));
	}

	// TODO: basic_vec<swizzle_value, 3> ?
	void set_swizzle_rgba(swizzle_value _r, swizzle_value _g, swizzle_value _b, swizzle_value _a)
	{
		set_parameter_raw(GL_TEXTURE_SWIZZLE_R, static_cast<int_t>(_r));
		set_parameter_raw(GL_TEXTURE_SWIZZLE_G, static_cast<int_t>(_g));
		set_parameter_raw(GL_TEXTURE_SWIZZLE_B, static_cast<int_t>(_b));
		set_parameter_raw(GL_TEXTURE_SWIZZLE_A, static_cast<int_t>(_a));
	}

	// TODO: this should be automatic?
	void set_max_level(int_t _level)
	{
		set_parameter_raw(GL_TEXTURE_MAX_LEVEL, _level);
	}

	void set_min_filter(texture_filter _tex, mipmap_filter _mip)
	{
		set_parameter_raw(GL_TEXTURE_MIN_FILTER, detail::get_texture_filter_enum(_tex, _mip));
	}

	void set_mag_filter(texture_filter _tex)
	{
		set_parameter_raw(GL_TEXTURE_MAG_FILTER, detail::get_texture_filter_enum(_tex));
	}

	void set_wrap_s(wrap_mode _mode)
	{
		set_parameter_raw(GL_TEXTURE_WRAP_S, static_cast<int_t>(_mode));
	}

	void set_wrap_t(wrap_mode _mode)
	{
		set_parameter_raw(GL_TEXTURE_WRAP_T, static_cast<int_t>(_mode));
	}

	void set_wrap_r(wrap_mode _mode)
	{
		set_parameter_raw(GL_TEXTURE_WRAP_R, static_cast<int_t>(_mode));
	}

private:
	enum_t get_pixel_format_enum(pixel_format _fmt)
	{
		enum_t pfmt = static_cast<enum_t>(_fmt);

		// TODO: This is ugly and seems like it might not be needed with gl4?
		if (!std::is_same<float_t, DataType>::value)
		{
			if (pixel_format::r == _fmt)
				pfmt = GL_RED_INTEGER;
			else if (pixel_format::rg == _fmt)
				pfmt = GL_RG_INTEGER;
			else if (pixel_format::rgb == _fmt)
				pfmt = GL_RGB_INTEGER;
			else if (pixel_format::rgba == _fmt)
				pfmt = GL_RGBA_INTEGER;
		}

		return pfmt;
	}

	template <typename T>
	void set_parameter_raw(GLenum _pname, T _val)
	{
		detail::set_texture_parameter<type>(native_handle(), _pname, _val);
	}
};

}

