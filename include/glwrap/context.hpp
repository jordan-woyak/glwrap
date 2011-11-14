
#pragma once

#include "vector.hpp"
#include "array_buffer.hpp"
#include "index_buffer.hpp"
#include "vertex_array.hpp"
#include "attribute.hpp"
#include "texture.hpp"
#include "constants.hpp"
#include "program.hpp"

namespace gl
{

class context;

class context
{
public:
	context()
		: m_fb(nullptr)
	{}

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

	void viewport(basic_vec<int_t, 2> _pos, basic_vec<int_t, 2> _size)
	{
		glViewport(_pos.x, _pos.y, _size.x, _size.y);
	}

	void scissor(basic_vec<int_t, 2> _pos, basic_vec<int_t, 2> _size)
	{
		glScissor(_pos.x, _pos.y, _size.x, _size.y);
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
	void bind_vertex_attribute(const attribute<T>& _attrib, const array_buffer_component<T>& _comp)
	{
		// TODO: unbind VAO
		auto const index = _attrib.get_location();

		glEnableVertexAttribArray(index);
		_comp.bind_to_attrib(index);
	}

	// TODO: rename?
	/*
	template <typename T>
	void draw_buffers(fragdata<T>& _fragdata, color_buffer _cbuf)
	{
		std::array<GLenum,

		glDrawBuffers
	}
	*/

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

	void blit_pixels(pixel_block const& src, pixel_block const& dst, filter _filter)
	{
		// TODO: mask
		auto _mask = GL_COLOR_BUFFER_BIT;

		src.bind(GL_READ_FRAMEBUFFER);
		dst.bind(GL_DRAW_FRAMEBUFFER);

		glBlitFramebuffer(
			src.m_lower.x, src.m_lower.y, src.m_lower.x, src.m_lower.y,
			dst.m_lower.x, dst.m_lower.y, dst.m_upper.x, dst.m_upper.y,
			_mask, static_cast<GLenum>(_filter));
	}

	void draw_arrays(program& _prog, primitive _mode, vertex_array& _arrays, int_t _first, sizei_t _count)
	{
		// TODO: kill
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		_prog.bind();
		_arrays.bind();
		glDrawArrays(static_cast<GLenum>(_mode), _first, _count);
	}

	template <typename T>
	void draw_elements(program& _prog, primitive _mode, vertex_array& _arrays, index_buffer<T>& _indices, int_t _first, sizei_t _count)
	{
		// TODO: kill
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		_prog.bind();
		_arrays.bind();
		_indices.bind();
		glDrawElements(static_cast<GLenum>(_mode), _count,
			detail::data_type_enum<T>(), std::add_pointer<char>::type() + _first * sizeof(T));
	}

	framebuffer& default_framebuffer()
	{
		return m_fb;
	}

private:
	framebuffer m_fb;
};

}
