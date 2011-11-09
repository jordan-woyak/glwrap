
#pragma once

#include "vector.hpp"
#include "buffer.hpp"
#include "attribute.hpp"
#include "texture.hpp"

namespace gl
{

enum class capability : GLenum
{
	blend = GL_BLEND,
	color_logic_op = GL_COLOR_LOGIC_OP,
	cull_face = GL_CULL_FACE,
	depth_clamp = GL_DEPTH_CLAMP,
	dither = GL_DITHER,
	line_smooth = GL_LINE_SMOOTH,
	multisample = GL_MULTISAMPLE,

	polygon_offset_fill = GL_POLYGON_OFFSET_FILL,
	polygon_offset_line = GL_POLYGON_OFFSET_LINE,
	polygon_offset_point = GL_POLYGON_OFFSET_POINT,
	polygon_smooth = GL_POLYGON_SMOOTH,

	primitive_restart = GL_PRIMITIVE_RESTART,

	sample_alpha_to_coverage = GL_SAMPLE_ALPHA_TO_COVERAGE,
	sample_alpha_to_one = GL_SAMPLE_ALPHA_TO_ONE,
	sample_coverage = GL_SAMPLE_COVERAGE,

	depth_test = GL_DEPTH_TEST,
	scissor_test = GL_SCISSOR_TEST,
	stencil_test = GL_STENCIL_TEST,

	texture_cube_map_seamless = GL_TEXTURE_CUBE_MAP_SEAMLESS,
	program_point_size = GL_PROGRAM_POINT_SIZE
};

enum class provoke_mode : GLenum
{
	first = GL_FIRST_VERTEX_CONVENTION,
	last = GL_LAST_VERTEX_CONVENTION,
};

enum class stencil_action : GLenum
{
	keep = GL_KEEP,
	zero = GL_ZERO,
	replace = GL_REPLACE,
	increment = GL_INCR,
	increment_wrap = GL_INCR_WRAP,
	decrement = GL_DECR,
	decrement_wrap = GL_DECR_WRAP,
	invert = GL_INVERT
};

enum class stencil_test : GLenum
{
	never = GL_NEVER,
	less = GL_LESS,
	less_equal = GL_LEQUAL,
	greater = GL_GREATER,
	greater_equal = GL_GEQUAL,
	equal = GL_EQUAL,
	not_equal = GL_NOTEQUAL,
	always = GL_ALWAYS
};

enum class face : GLenum
{
	back = GL_BACK,
	front = GL_FRONT,
	both = GL_FRONT_AND_BACK
};

enum class orientation : GLenum
{
	cw = GL_CW,
	ccw = GL_CCW
};

enum class blend_mode : GLenum
{
	add = GL_FUNC_ADD,
	subtract = GL_FUNC_SUBTRACT,
	reverse_subtract = GL_FUNC_REVERSE_SUBTRACT,
	min = GL_MIN,
	max = GL_MAX
};

enum class blend_factor : GLenum
{
	zero = GL_ZERO,
	one = GL_ONE,
	src_color = GL_SRC_COLOR,
	inverse_src_color = GL_ONE_MINUS_SRC_COLOR,
	dst_color = GL_DST_COLOR,
	inverse_dst_color = GL_ONE_MINUS_DST_COLOR,
	src_alpha = GL_SRC_ALPHA,
	inverse_src_alpha = GL_ONE_MINUS_SRC_ALPHA,
	dst_alpha = GL_DST_ALPHA,
	inverse_dst_alpha = GL_ONE_MINUS_DST_ALPHA,
	constant_color = GL_CONSTANT_COLOR,
	inverse_constant_color = GL_ONE_MINUS_CONSTANT_COLOR,
	constant_alpha = GL_CONSTANT_ALPHA,
	inverse_constant_alpha = GL_ONE_MINUS_CONSTANT_ALPHA
};

class context;

template <int D>
struct bound_texture
{
public:
	int_t get_unit() const
	{
		return m_unit;
	}

private:
	friend class context;

	explicit bound_texture(int_t _unit)
		: m_unit(_unit)
	{}

	bound_texture(const bound_texture&) = default;
	bound_texture& operator=(const bound_texture&) = delete;

	int_t m_unit;
};

class context
{
public:
	void clear_color(fvec4 const& _color)
	{
		glClearColor(_color.x, _color.y, _color.z, _color.w);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void clear_stencil(int_t _index)
	{
		glClearStencil(_index);
		glClear(GL_STENCIL_BUFFER_BIT);
	}

	void clear_depth(depth_t _depth)
	{
		glClearDepth(_depth);
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	void line_width(int_t _width)
	{
		glLineWidth(_width);
	}

	void polygon_offset(float_t _factor, float_t _units)
	{
		glPolygonOffset(_factor, _units);
	}

	void point_size(float_t _size)
	{
		glPointSize(_size);
	}

	void provoking_vertex(provoke_mode _mode)
	{
		glProvokingVertex(static_cast<GLenum>(_mode));
	}

	void front_face(orientation _orient)
	{
		glFrontFace(static_cast<GLenum>(_orient));
	}

	void stencil_op(stencil_action _fail, stencil_action _pass_fail, stencil_action _pass, face _face = face::both)
	{
		glStencilOpSeparate(static_cast<GLenum>(_face),
			static_cast<GLenum>(_fail), static_cast<GLenum>(_pass_fail), static_cast<GLenum>(_pass));
	}

	void stencil_func(stencil_test _test, int_t _ref, uint_t _mask, face _face = face::both)
	{
		glStencilFuncSeparate(static_cast<GLenum>(_face), static_cast<GLenum>(_test), _ref, _mask);
	}

	void stencil_mask(uint_t _mask, face _face = face::both)
	{
		glStencilMaskSeparate(static_cast<GLenum>(_face), _mask);
	}

	void viewport(int_t x, int_t y, size_t w, size_t h)
	{
		glViewport(x, y, w, h);
	}

	void blend_color(fvec4 _color)
	{
		glBlendColor(_color.x, _color.y, _color.z, _color.w);
	}

	void blend_equation(blend_mode _mode)
	{
		glBlendEquation(static_cast<GLenum>(_mode));
	}

	void blend_equation(blend_mode _mode_rgb, blend_mode _mode_alpha)
	{
		glBlendEquationSeparate(static_cast<GLenum>(_mode_rgb), static_cast<GLenum>(_mode_alpha));
	}

	void blend_func(blend_factor _sfactor, blend_factor _dfactor)
	{
		glBlendFunc(static_cast<GLenum>(_sfactor), static_cast<GLenum>(_dfactor));
	}

	void blend_func(blend_factor _sfactor_rgb, blend_factor _dfactor_rgb, blend_factor _sfactor_alpha, blend_factor _dfactor_alpha)
	{
		glBlendFuncSeparate(static_cast<GLenum>(_sfactor_rgb), static_cast<GLenum>(_dfactor_rgb),
			static_cast<GLenum>(_sfactor_alpha), static_cast<GLenum>(_dfactor_alpha));
	}

	template <typename T>
	void bind_vertex_attribute(const attribute<T>& _attrib, const buffer_component<T>& _comp)
	{
		auto const index = _attrib.get_location();

		glEnableVertexAttribArray(index);
		_comp.bind_to_attrib(index);
	}

	template <typename T>
	void disable_vertex_attribute(const attribute<T>& _attrib)
	{
		glDisableVertexAttribArray(_attrib.get_location());
	}

	void enable(capability _cap)
	{
		glEnable(static_cast<GLenum>(_cap));
	}

	void disable(capability _cap)
	{
		glDisable(static_cast<GLenum>(_cap));
	}

	template <int D>
	bound_texture<D> bind_texture(size_t _unit, texture<D>& _texture)
	{
		glActiveTexture(GL_TEXTURE0 + _unit);
		_texture.bind();
		return bound_texture<D>(_unit);
	}
};

}
