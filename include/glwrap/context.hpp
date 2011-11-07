
#pragma once

#include "vector.hpp"
#include "vertex_buffer.hpp"
#include "attribute.hpp"

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

typedef double_t depth_t;

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

	template <typename T>
	void bind_vertex_attribute(const attribute<T>& _attrib, const vertex_buffer_component<T>& _comp)
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
};

}
