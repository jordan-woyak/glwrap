#pragma once

#include "vector.hpp"
#include "native_handle.hpp"
#include "util.hpp"
#include "unpack_buffer.hpp"
#include "image_format.hpp"

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
	uint_t get_index() const
	{
		return m_index;
	}

private:
	texture_unit(uint_t _index)
		: m_index(_index)
	{}

	uint_t m_index;
};

// TODO: name?
class texture_unit_alloter
{
public:
	// TODO: really need context?
	texture_unit_alloter(context& _context)
		: m_current_index()
		, m_max_combined_texture_image_units()
	{
		GLint max_tunits{};
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &max_tunits);
		m_max_combined_texture_image_units = max_tunits;
	}

	template <typename T>
	texture_unit<T> allot()
	{
		if (m_current_index == m_max_combined_texture_image_units)
			throw exception();

		return {m_current_index++};
	}

private:
	uint_t m_current_index;
	uint_t m_max_combined_texture_image_units;
};

template <int D>
class texture : public globject
{
	friend class context;

public:
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
	void assign(unpack_buffer<T, D> const& _buffer, image_format _ifmt)
	{
		bind();
		glTexImage2D(GL_TEXTURE_2D, 0, _ifmt.value, _buffer.m_dims.x, _buffer.m_dims.y,
			0, static_cast<GLenum>(_buffer.m_pfmt), detail::data_type_enum<T>(), _buffer.m_data);
	}

	// TODO: level

	void storage(basic_vec<int_t, D> const& _dims, image_format _ifmt)
	{
		// TODO: for non 2d textures
		bind();
		glTexImage2D(GL_TEXTURE_2D, 0, _ifmt.value, _dims.x, _dims.y,
			0, GL_RED, GL_BYTE, nullptr);
	}

	// mipmap vs. mipmaps ?
	void generate_mipmap()
	{
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

	// TODO: this should be automatic?
	void set_max_level(int_t _level)
	{
		bind();
		glTexParameteri(get_target(), GL_TEXTURE_MAX_LEVEL, _level);
	}

	static GLenum get_target();

private:
	void bind() const
	{
		glBindTexture(get_target(), native_handle());
	}
};

typedef texture<1> texture_1d;
typedef texture<2> texture_2d;
typedef texture<3> texture_3d;

template <>
inline GLenum texture<1>::get_target()
{
	return GL_TEXTURE_1D;
}

template <>
inline GLenum texture<2>::get_target()
{
	return GL_TEXTURE_2D;
}

template <>
inline GLenum texture<3>::get_target()
{
	return GL_TEXTURE_3D;
}

}

