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

namespace gl
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

// TODO: rename attribute_location?
template <typename T>
class texture_unit
{
	friend class texture_unit_alloter;

public:
	int_t get_index() const
	{
		return m_index;
	}

private:
	texture_unit(int_t _index)
		: m_index(_index)
	{}

	int_t m_index;
};

// TODO: name?
class texture_unit_alloter
{
public:
	// TODO: really need context?
	texture_unit_alloter(context& _context)
		: m_current_index()
		, m_max_comb_tunits()
	{
		detail::gl_get(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &m_max_comb_tunits);
	}

	// TODO: multiple targets are allowed on each texture unit

	template <typename T>
	texture_unit<T> allot()
	{
		if (m_current_index == m_max_comb_tunits)
			throw exception();

		return {m_current_index++};
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

template <typename T, typename V>
void set_texture_parameter(V);

template <>
void set_texture_parameter<texture_parameter::wrap_s, wrap_mode>(wrap_mode _mode)
{

}

template <texture_type Type>
class texture : public globject
{
	friend class context;

public:
	static const texture_type type = Type;
	static const int dimensions = detail::texture_dims<type>::value;

	void swap(texture& _other)
	{
		globject::swap(_other);
	}

	~texture()
	{
		auto const nh = native_handle();
		glDeleteTextures(1, &nh);
	}

	explicit texture(context& _context)
		: globject(gen_return(glGenTextures))
	{}

	// TODO: work for NPOT
	template <typename T>
	void assign(unpack_buffer<T, dimensions> const& _buffer, image_format _ifmt)
	{
		bind();
		glTexImage2D(get_target(), 0, _ifmt.value, _buffer.m_dims.x, _buffer.m_dims.y,
			0, static_cast<GLenum>(_buffer.m_pfmt), detail::data_type_enum<T>(), _buffer.m_data);
	}

	// TODO: level

	// TODO: work for NPOT
	void storage(basic_vec<int_t, dimensions> const& _dims, image_format _ifmt)
	{
		// TODO: for non 2d textures
		bind();
		glTexImage2D(get_target(), 0, _ifmt.value, _dims.x, _dims.y,
			0, GL_RED, GL_BYTE, nullptr);
	}

	// mipmap vs. mipmaps ?
	void generate_mipmap()
	{
		bind();
		glGenerateMipmap(get_target());
	}

	void set_swizzle(swizzle_component _comp, swizzle_value _val)
	{
		bind();
		glTexParameteri(get_target(), static_cast<GLenum>(_comp), static_cast<GLint>(_val));
	}

	void set_swizzle_rgba(swizzle_value _r, swizzle_value _g, swizzle_value _b, swizzle_value _a)
	{
		bind();
		glTexParameteri(get_target(), GL_TEXTURE_SWIZZLE_R, static_cast<GLint>(_r));
		glTexParameteri(get_target(), GL_TEXTURE_SWIZZLE_G, static_cast<GLint>(_g));
		glTexParameteri(get_target(), GL_TEXTURE_SWIZZLE_B, static_cast<GLint>(_b));
		glTexParameteri(get_target(), GL_TEXTURE_SWIZZLE_A, static_cast<GLint>(_a));
	}

	template <typename T>
	void set_parameter(T _param, typename T::value_type _val);

	// TODO: this should be automatic?
	void set_max_level(int_t _level)
	{
		set_parameter_raw(GL_TEXTURE_MAX_LEVEL, _level);
	}

	void set_parameter_raw(GLenum _pname, int_t _val)
	{
		bind();
		glTexParameteri(get_target(), _pname, _val);
	}

	void set_parameter_raw(GLenum _pname, float_t _val)
	{
		bind();
		glTexParameterf(get_target(), _pname, _val);
	}

	void set_min_filter(texture_filter _mode)
	{
		set_parameter_raw(GL_TEXTURE_MIN_FILTER, static_cast<int>(_mode));
	}

	void set_mag_filter(texture_filter _mode)
	{
		set_parameter_raw(GL_TEXTURE_MAG_FILTER, static_cast<int>(_mode));
	}

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

private:
	static GLenum get_target()
	{
		return detail::get_texture_target<type>();
	}

	void bind() const
	{
		glBindTexture(get_target(), native_handle());
	}
};

template <>
class texture<texture_type::texture_buffer> : public globject
{
	friend class context;

public:
	static const texture_type type = texture_type::texture_buffer;
	static const int dimensions = detail::texture_dims<type>::value;

	void swap(texture& _other)
	{
		globject::swap(_other);
	}

	~texture()
	{
		auto const nh = native_handle();
		glDeleteTextures(1, &nh);
	}

	explicit texture(context& _context)
		: globject(gen_return(glGenTextures))
	{}

	template <typename T>
	void bind_buffer(buffer<T> const& _buffer)
	{
		bind();
		// TODO: don't hardcode datatype!
		glTexBuffer(get_target(), GL_RGBA32F, _buffer.native_handle());
	}

private:
	static GLenum get_target()
	{
		return detail::get_texture_target<type>();
	}

	void bind() const
	{
		glBindTexture(get_target(), native_handle());
	}
};

}

