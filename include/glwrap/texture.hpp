#pragma once

#include "vector.hpp"
#include "native_handle.hpp"
#include "util.hpp"
#include "unpack_buffer.hpp"
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

struct texture_unit_index
{};

}

template <typename T>
using texture_unit = detail::buffer_index<detail::texture_unit_index, T>;

// TODO: name?
class texture_unit_enumerator
{
public:
	// TODO: really need context?
	texture_unit_enumerator(context& _context)
		: m_current_index()
		, m_max_comb_tunits()
	{
		// TODO: correct parameter?
		detail::gl_get(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &m_max_comb_tunits);
	}

	template <typename T>
	texture_unit<T> get()
	{
		if (m_current_index == m_max_comb_tunits)
			throw exception(0);

		return texture_unit<T>(m_current_index++);
	}

private:
	int_t m_current_index;
	int_t m_max_comb_tunits;
};

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
	static const int dimensions = detail::texture_dims<type>::value;

	// TODO: this is more of a detail..
	static const enum_t target = detail::texture_traits<type>::target;

	typedef typename internal_format<DataType>::enum_type internal_format_type;

	explicit basic_texture(context&)
		: detail::globject<detail::texture_obj>(static_cast<enum_t>(Type))
	{}

	// TODO: this can only be done once. do it in the constructor?
	// TODO: storage makes immutable textures. make this more obvious
	// TexStorage also sets the max mipmap level
	void define_storage(sizei_t _levels, internal_format_type _ifmt, detail::tex_dims<Type> const& _dims)
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
/*
	// TODO: force same internal format for each level
	template <typename T>
	void load_image(int_t _level, unpack_buffer<T, dimensions> const& _buffer, internal_format_type _ifmt)
	{
		// TODO: ugly
		detail::scoped_value<detail::parameter::texture<Type>> binding(native_handle());

		// TODO: allow more than level 0
		detail::gl_tex_image<Type>(_level, static_cast<enum_t>(_ifmt), _buffer.m_dims,
			static_cast<enum_t>(_buffer.m_pfmt), _buffer.m_data);
	}
*/

	// TODO: require unpack_buffer is of the proper type and has the right number of components

	template <typename T>
	void load_subimage(int_t _level, detail::tex_dims<Type> const& _offset, unpack_buffer<T, dimensions> const& _buffer)
	{
		if (is_extension_present(GL_ARB_direct_state_access))
		{
			detail::gl_texture_sub_image<Type>(native_handle(), _level, _offset, _buffer.m_dims,
				static_cast<enum_t>(_buffer.m_pfmt), _buffer.m_data);
		}
		else
		{
			detail::scoped_value<detail::parameter::texture<Type>> binding(native_handle());

			detail::gl_tex_sub_image<Type>(_level, _offset, _buffer.m_dims,
				static_cast<enum_t>(_buffer.m_pfmt), _buffer.m_data);
		}
	}

/*
	// TODO: rename / can I allow for the "assign" function to do this?
	void resize(sizei_t _level, internal_format_type _ifmt, detail::tex_dims<Type> const& _dims)
	{
		// TODO: ugly
		detail::scoped_value<detail::parameter::texture<Type>> binding(native_handle());

		detail::gl_tex_image<Type>(_level, static_cast<enum_t>(_ifmt), _dims, GL_RED, (ubyte_t*)nullptr);
	}
*/
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
	template <typename T>
	void set_parameter_raw(GLenum _pname, T _val)
	{
		detail::set_texture_parameter<type>(native_handle(), _pname, _val);
	}
};

}

