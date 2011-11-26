
#pragma once

#include "vector.hpp"
#include "vertex_array.hpp"
#include "attribute.hpp"
#include "sampler.hpp"
#include "texture.hpp"
#include "constants.hpp"
#include "framebuffer.hpp"
#include "program.hpp"
#include "sync.hpp"
#include "draw.hpp"

namespace gl
{

class context;

class context
{
public:
	void clear_color(framebuffer_reference _write, vec4 const& _color)
	{
		_write.bind_draw();

		glClearColor(_color.x, _color.y, _color.z, _color.w);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void clear_stencil(framebuffer_reference _write, int_t _index)
	{
		_write.bind_draw();

		glClearStencil(_index);
		glClear(GL_STENCIL_BUFFER_BIT);
	}

	void clear_depth(framebuffer_reference _write, depth_t _depth)
	{
		_write.bind_draw();

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

	void stencil_func(comparison _test, int_t _ref, uint_t _mask, face _face = face::both)
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

	void blend_color(vec4 _color)
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

	void depth_func(comparison _compar)
	{
		glDepthFunc(static_cast<GLenum>(_compar));
	}

	void depth_range(depth_t _near, depth_t _far)
	{
		glDepthRange(_near, _far);
	}

	void depth_mask(bool_t _enable)
	{
		glDepthMask(_enable);
	}

	void sample_coverage(float_t _value, bool_t _invert)
	{
		glSampleCoverage(_value, _invert);
	}

	void hint(hint_target _target, hint_value _value)
	{
		glHint(static_cast<GLenum>(_target), static_cast<GLenum>(_value));
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

	template <typename T>
	void bind_texture(texture_unit<T>& _unit, T const& _texture)
	{
		glActiveTexture(GL_TEXTURE0 + _unit.get_index());
		_texture.bind();
	}

	template <typename T>
	void bind_sampler(texture_unit<T>& _unit, sampler const& _sampler)
	{
		_sampler.bind(_unit.get_index());
	}

	template <typename T>
	void bind_buffer(uniform_binding<T>& _unit, buffer_iterator<T> const& _iter, uint_t _size)
	{
		glBindBufferRange(GL_UNIFORM_BUFFER, _unit.get_index(),
			_iter.m_buffer,	reinterpret_cast<GLintptr>(_iter.m_offset), _size * sizeof(T));
	}

	template <typename T>
	void bind_buffer(transform_feedback_binding<T>& _unit, buffer_iterator<T> const& _iter, uint_t _size)
	{
		glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, _unit.get_index(),
			_iter.m_buffer, reinterpret_cast<GLintptr>(_iter.m_offset), _size * sizeof(T));
	}

	void blit_pixels(read_color_buffer const& _read, ivec2 const& _src_begin, ivec2 const& _src_end,
		framebuffer_reference _write, ivec2 const& _dst_begin, ivec2 const& _dst_end, filter _filter)
	{
		// TODO: mask
		auto _mask = GL_COLOR_BUFFER_BIT;

		_read.bind();
		_write.bind_draw();

		glBlitFramebuffer(
			_src_begin.x, _src_begin.y, _src_end.x, _src_end.y,
			_dst_begin.x, _dst_begin.y, _dst_end.x, _dst_end.y,
			_mask, static_cast<GLenum>(_filter));
	}

	void flush()
	{
		glFlush();
	}

	void finish()
	{
		glFinish();
	}

	sync fence_sync(sync_condition _cond, bitfield_t _flags)
	{
		return {glFenceSync(static_cast<GLenum>(_cond), _flags)};
	}

	// TODO: multi draw needed?
	// TODO: draw range elements [base vertex]

	void draw_arrays(technique& _tech, std::size_t _offset, std::size_t _count)
	{
		_tech.bind();

		glDrawArrays(_tech.get_mode(),
			_offset,
			_count);
	}

	void draw_arrays_instanced(technique& _tech, std::size_t _offset, std::size_t _count, std::size_t _instances)
	{
		_tech.bind();

		glDrawArraysInstanced(_tech.get_mode(),
			_offset,
			_count,
			_instances);
	}

	void draw_elements(technique& _tech, std::size_t _start, std::size_t _count)
	{
		_tech.bind();

		glDrawElements(_tech.get_mode(),
			_count,
			_tech.get_type(),
			reinterpret_cast<void*>((intptr_t)_start));
	}

	void draw_elements_offset(technique& _tech, std::size_t _start, std::size_t _count, std::size_t _offset)
	{
		_tech.bind();

		glDrawElementsBaseVertex(_tech.get_mode(),
			_count,
			_tech.get_type(),
			reinterpret_cast<void*>((intptr_t)_start),
			_offset);
	}

	void draw_elements_instanced(technique& _tech, std::size_t _start, std::size_t _count, std::size_t _instances)
	{
		_tech.bind();

		glDrawElementsInstanced(_tech.get_mode(),
			_count,
			_tech.get_type(),
			reinterpret_cast<void*>((intptr_t)_start),
			_instances);
	}

	color_number draw_buffer(uint_t _index)
	{
		return {_index};
	}

	color_attach_point color_buffer(uint_t _index)
	{
		return {static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + _index)};
	}

	attach_point stencil_buffer()
	{
		return {GL_STENCIL_ATTACHMENT};
	}

	attach_point depth_buffer()
	{
		return {GL_DEPTH_ATTACHMENT};
	}
};

}
