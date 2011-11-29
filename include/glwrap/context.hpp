
#pragma once

#include <SFML/Graphics.hpp>

#include "vector.hpp"
#include "vertex_array.hpp"
#include "attribute.hpp"
#include "sampler.hpp"
#include "texture.hpp"
#include "constants.hpp"
#include "framebuffer.hpp"
#include "program.hpp"
#include "uniform_block.hpp"
#include "transform_feedback.hpp"
#include "sync.hpp"

namespace gl
{

class display;

class context
{
	friend class display;

public:
	context()
		: m_program()
		, m_vertex_array()
		, m_draw_fbo(), m_read_fbo()
		, m_primitive_mode(static_cast<GLenum>(primitive::triangles))
		, m_element_type()
		, m_sf_window(new sf::RenderWindow)
	{
		glewInit();
	}

	context(context const&) = delete;
	context& operator=(context const&) = delete;

	// TODO: kill these 3, provide clear functionality like ogl
	void clear_color(vec4 const& _color = vec4{})
	{
		prepare_use_fb();

		glClearColor(_color.x, _color.y, _color.z, _color.w);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void clear_stencil(int_t _index = 0)
	{
		prepare_use_fb();

		glClearStencil(_index);
		glClear(GL_STENCIL_BUFFER_BIT);
	}

	void clear_depth(depth_t _depth = 1.0)
	{
		prepare_use_fb();

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

	void sample_mask(uint_t _mask_number, bitfield_t _mask)
	{
		// TODO: assert _mask_number < GL_MAX_SAMPLE_MASK_WORDS

		glSampleMaski(_mask_number, _mask);
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

	// TODO: should allow reference to buffer element as well
	// range is probably not needed/wanted for uniform buffer
	// TODO: require "uniform_block_align<T>"
	template <typename T>
	void bind_buffer(uniform_block_binding<T>& _unit, buffer_iterator<T> const& _iter, uint_t _size)
	{
		glBindBufferRange(GL_UNIFORM_BUFFER, _unit.get_index(),
			_iter.m_buffer,	reinterpret_cast<GLintptr>(_iter.m_offset), _iter.stride() * _size);
	}

	template <typename T>
	void bind_buffer(transform_feedback_binding<T>& _unit, buffer_iterator<T> const& _iter, uint_t _size)
	{
		glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, _unit.get_index(),
			_iter.m_buffer, reinterpret_cast<GLintptr>(_iter.m_offset), _iter.stride() * _size);
	}

	void start_transform_feedback(primitive _mode)
	{
		glBeginTransformFeedback(static_cast<GLenum>(_mode));
	}

	void stop_transform_feedback()
	{
		glEndTransformFeedback();
	}

	void blit_pixels(ivec2 const& _src_begin, ivec2 const& _src_end,
		ivec2 const& _dst_begin, ivec2 const& _dst_end, filter _filter)
	{
		// TODO: mask
		auto _mask = GL_COLOR_BUFFER_BIT;

		prepare_use_fb();

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

	void draw_arrays(std::size_t _offset, std::size_t _count)
	{
		prepare_draw();

		glDrawArrays(get_primitive_mode(),
			_offset,
			_count);
	}

	void draw_arrays_instanced(std::size_t _offset, std::size_t _count, std::size_t _instances)
	{
		prepare_draw();

		glDrawArraysInstanced(get_primitive_mode(),
			_offset,
			_count,
			_instances);
	}

	void draw_elements(std::size_t _start, std::size_t _count)
	{
		prepare_draw();

		glDrawElements(get_primitive_mode(),
			_count,
			get_element_type(),
			reinterpret_cast<void*>((intptr_t)_start));
	}

	void draw_elements_offset(std::size_t _start, std::size_t _count, std::size_t _offset)
	{
		prepare_draw();

		glDrawElementsBaseVertex(get_primitive_mode(),
			_count,
			get_element_type(),
			reinterpret_cast<void*>((intptr_t)_start),
			_offset);
	}

	void draw_elements_instanced(std::size_t _start, std::size_t _count, std::size_t _instances)
	{
		prepare_draw();

		glDrawElementsInstanced(get_primitive_mode(),
			_count,
			get_element_type(),
			reinterpret_cast<void*>((intptr_t)_start),
			_instances);
	}

	void use_program(program& _prog)
	{
		m_program = _prog.native_handle();
		glUseProgram(m_program);
	}

	void use_vertex_array(vertex_array& _vert)
	{
		m_vertex_array = _vert.native_handle();
		//glBindVertexArray(_vert.native_handle());
	}

	template <typename T>
	void use_element_array(buffer<T>& _buff)
	{
		static_assert(std::is_same<T, ubyte_t>::value
			|| std::is_same<T, ushort_t>::value
			|| std::is_same<T, uint_t>::value
			, "must be ubyte, uint, ushort");

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _buff.native_handle());

		m_element_type = detail::data_type_enum<T>();
	}

	// TODO: kill this?
	void use_primitive_mode(primitive _prim)
	{
		m_primitive_mode = static_cast<GLenum>(_prim);
	}

	void use_draw_framebuffer(framebuffer_reference _fb)
	{
		m_draw_fbo = _fb.native_handle();
		//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fb.native_handle());
	}

	void use_read_framebuffer(framebuffer_reference _fb)
	{
		m_read_fbo = _fb.native_handle();
		//glBindFramebuffer(GL_READ_FRAMEBUFFER, _fb.native_handle());
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

private:
	void prepare_draw()
	{
		// TODO: make this not necessary
		glUseProgram(m_program);
		glBindVertexArray(m_vertex_array);
		prepare_use_fb();
	}

	void prepare_use_fb()
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_draw_fbo);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_read_fbo);
	}

	GLenum get_primitive_mode() const
	{
		return m_primitive_mode;
	}

	GLenum get_element_type() const
	{
		return m_element_type;
	}

	sf::RenderWindow& get_window()
	{
		return *m_sf_window;
	}

	GLuint m_program;
	GLuint m_vertex_array;

	GLuint m_draw_fbo, m_read_fbo;

	GLenum m_primitive_mode;
	GLenum m_element_type;

	std::unique_ptr<sf::RenderWindow> m_sf_window;
};

}
