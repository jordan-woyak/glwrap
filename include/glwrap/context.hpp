
#pragma once

#define GLWRAP_NO_SFML

#ifndef GLWRAP_NO_SFML
#include <SFML/Graphics.hpp>
#endif

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

#include "detail/context.hpp"

namespace gl
{

class display;

class context
{
	friend class display;

public:
	context()
		: m_primitive_mode(static_cast<GLenum>(primitive::triangles))
		, m_element_type()
#ifndef GLWRAP_NO_SFML
		, m_sf_window(new sf::RenderWindow)
#endif
	{
#if !defined(GLWRAP_FAKE_CONTEXT) && defined(GLWRAP_NO_SFML)
		int dummy = 0;
		glutInit(&dummy, 0);
		
		glutInitContextVersion(3, 0);
		glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
		glutInitContextProfile(GLUT_CORE_PROFILE);

		// TODO: this is gross
		//int const w = glutCreateWindow("");
		//glutHideWindow();

		glewInit();

		//glutDestroyWindow(w);
#endif
	}

	context(context const&) = delete;
	context& operator=(context const&) = delete;

	// TODO: kill these 3, provide clear functionality like ogl
	void clear_color(vec4 const& _color = vec4{})
	{
		GLWRAP_EC_CALL(glClearColor)(_color.x, _color.y, _color.z, _color.w);
		GLWRAP_EC_CALL(glClear)(GL_COLOR_BUFFER_BIT);
	}

	void clear_stencil(int_t _index = 0)
	{
		GLWRAP_EC_CALL(glClearStencil)(_index);
		GLWRAP_EC_CALL(glClear)(GL_STENCIL_BUFFER_BIT);
	}

	void clear_depth(depth_t _depth = 1.0)
	{
		GLWRAP_EC_CALL(glClearDepth)(_depth);
		GLWRAP_EC_CALL(glClear)(GL_DEPTH_BUFFER_BIT);
	}

	void line_width(float_t _width)
	{
		GLWRAP_EC_CALL(glLineWidth)(_width);
	}

	void polygon_offset(float_t _factor, float_t _units)
	{
		GLWRAP_EC_CALL(glPolygonOffset)(_factor, _units);
	}

	void point_size(float_t _size)
	{
		GLWRAP_EC_CALL(glPointSize)(_size);
	}

	void provoking_vertex(provoke_mode _mode)
	{
		GLWRAP_EC_CALL(glProvokingVertex)(static_cast<GLenum>(_mode));
	}

	void front_face(orientation _orient)
	{
		GLWRAP_EC_CALL(glFrontFace)(static_cast<GLenum>(_orient));
	}

	void stencil_op(stencil_action _fail, stencil_action _pass_fail, stencil_action _pass, face _face = face::both)
	{
		GLWRAP_EC_CALL(glStencilOpSeparate)(static_cast<GLenum>(_face),
			static_cast<GLenum>(_fail), static_cast<GLenum>(_pass_fail), static_cast<GLenum>(_pass));
	}

	void stencil_func(comparison _test, int_t _ref, uint_t _mask, face _face = face::both)
	{
		GLWRAP_EC_CALL(glStencilFuncSeparate)(static_cast<GLenum>(_face), static_cast<GLenum>(_test), _ref, _mask);
	}

	void stencil_mask(uint_t _mask, face _face = face::both)
	{
		GLWRAP_EC_CALL(glStencilMaskSeparate)(static_cast<GLenum>(_face), _mask);
	}

	void viewport(ivec2 _pos, ivec2 _size)
	{
		GLWRAP_EC_CALL(glViewport)(_pos.x, _pos.y, _size.x, _size.y);
	}

	void scissor(ivec2 _pos, ivec2 _size)
	{
		GLWRAP_EC_CALL(glScissor)(_pos.x, _pos.y, _size.x, _size.y);
	}

	void blend_color(vec4 _color)
	{
		GLWRAP_EC_CALL(glBlendColor)(_color.x, _color.y, _color.z, _color.w);
	}

	void blend_equation(blend_mode _mode)
	{
		GLWRAP_EC_CALL(glBlendEquation)(static_cast<GLenum>(_mode));
	}

	void blend_equation(blend_mode _mode_rgb, blend_mode _mode_alpha)
	{
		GLWRAP_EC_CALL(glBlendEquationSeparate)(static_cast<GLenum>(_mode_rgb), static_cast<GLenum>(_mode_alpha));
	}

	void blend_func(blend_factor _sfactor, blend_factor _dfactor)
	{
		GLWRAP_EC_CALL(glBlendFunc)(static_cast<GLenum>(_sfactor), static_cast<GLenum>(_dfactor));
	}

	void blend_func(blend_factor _sfactor_rgb, blend_factor _dfactor_rgb, blend_factor _sfactor_alpha, blend_factor _dfactor_alpha)
	{
		GLWRAP_EC_CALL(glBlendFuncSeparate)(static_cast<GLenum>(_sfactor_rgb), static_cast<GLenum>(_dfactor_rgb),
			static_cast<GLenum>(_sfactor_alpha), static_cast<GLenum>(_dfactor_alpha));
	}

	void depth_func(comparison _compar)
	{
		GLWRAP_EC_CALL(glDepthFunc)(static_cast<GLenum>(_compar));
	}

	void depth_range(depth_t _near, depth_t _far)
	{
		GLWRAP_EC_CALL(glDepthRange)(_near, _far);
	}

	void depth_mask(bool_t _enable)
	{
		GLWRAP_EC_CALL(glDepthMask)(_enable);
	}

	void sample_coverage(float_t _value, bool_t _invert)
	{
		GLWRAP_EC_CALL(glSampleCoverage)(_value, _invert);
	}

	void sample_mask(uint_t _mask_number, bitfield_t _mask)
	{
		// TODO: assert _mask_number < GL_MAX_SAMPLE_MASK_WORDS

		GLWRAP_EC_CALL(glSampleMaski)(_mask_number, _mask);
	}

	void color_mask(basic_vec<bool, 4> const& _mask)
	{
		GLWRAP_EC_CALL(glColorMask)(_mask.x, _mask.y, _mask.z, _mask.w);
	}

	void color_mask(color_number const& _buf, basic_vec<bool, 4> const& _mask)
	{
		GLWRAP_EC_CALL(glColorMaski)(_buf.get_index(), _mask.x, _mask.y, _mask.z, _mask.w);
	}

	void hint(hint_target _target, hint_value _value)
	{
		GLWRAP_EC_CALL(glHint)(static_cast<GLenum>(_target), static_cast<GLenum>(_value));
	}

	template <typename T>
	void disable_vertex_attribute(const attribute<T>& _attrib)
	{
		GLWRAP_EC_CALL(glDisableVertexAttribArray)(_attrib.get_location());
	}

	void enable(capability _cap)
	{
		GLWRAP_EC_CALL(glEnable)(static_cast<GLenum>(_cap));
	}

	void disable(capability _cap)
	{
		GLWRAP_EC_CALL(glDisable)(static_cast<GLenum>(_cap));
	}

	template <typename T>
	void bind_texture(texture_unit<T> const& _unit, T const& _texture)
	{
		GLWRAP_EC_CALL(glActiveTexture)(GL_TEXTURE0 + _unit.get_index());
		GLWRAP_EC_CALL(glBindTexture)(T::target, _texture.native_handle());
	}

	template <typename T>
	void bind_sampler(texture_unit<T> const& _unit, sampler const& _sampler)
	{
		GLWRAP_EC_CALL(glBindSampler)(_unit.get_index(), _sampler.native_handle());
	}

	template <typename T>
	void unbind_sampler(texture_unit<T> const& _unit)
	{
		GLWRAP_EC_CALL(glBindSampler)(_unit.get_index(), 0);
	}

	// TODO: should allow reference to buffer element as well
	// range is probably not needed/wanted for uniform buffer
	template <typename T>
	void bind_buffer(uniform_block_binding<T> const& _unit, uniform_buffer_iterator<T> const& _iter, uint_t _size)
	{
		GLWRAP_EC_CALL(glBindBufferRange)(GL_UNIFORM_BUFFER, _unit.get_index(),
			_iter.get_buffer(),	_iter.get_offset() - (ubyte_t*)0, _iter.get_stride() * _size);
	}

	void start_transform_feedback(primitive _mode)
	{
		GLWRAP_EC_CALL(glBeginTransformFeedback)(static_cast<GLenum>(_mode));
	}

	void stop_transform_feedback()
	{
		GLWRAP_EC_CALL(glEndTransformFeedback)();
	}

	void blit_pixels(ivec2 const& _src_begin, ivec2 const& _src_end,
		ivec2 const& _dst_begin, ivec2 const& _dst_end, filter _filter)
	{
		// TODO: mask
		auto _mask = GL_COLOR_BUFFER_BIT;

		GLWRAP_EC_CALL(glBlitFramebuffer)(
			_src_begin.x, _src_begin.y, _src_end.x, _src_end.y,
			_dst_begin.x, _dst_begin.y, _dst_end.x, _dst_end.y,
			_mask, static_cast<GLenum>(_filter));
	}

	void flush()
	{
		GLWRAP_EC_CALL(glFlush)();
	}

	void finish()
	{
		GLWRAP_EC_CALL(glFinish)();
	}

	sync fence_sync(sync_condition _cond, bitfield_t _flags)
	{
		return {GLWRAP_EC_CALL(glFenceSync)(static_cast<GLenum>(_cond), _flags)};
	}

	// TODO: multi draw needed?
	// TODO: draw range elements [base vertex]

	void draw_arrays(std::size_t _offset, std::size_t _count)
	{
		GLWRAP_EC_CALL(glDrawArrays)(get_primitive_mode(),
			_offset,
			_count);
	}

	void draw_arrays_instanced(std::size_t _offset, std::size_t _count, std::size_t _instances)
	{
		GLWRAP_EC_CALL(glDrawArraysInstanced)(get_primitive_mode(),
			_offset,
			_count,
			_instances);
	}

	// TODO: start values need to be adjusted to byte offsets

	void draw_elements(std::size_t _start, std::size_t _count)
	{
		GLWRAP_EC_CALL(glDrawElements)(get_primitive_mode(),
			_count,
			get_element_type(),
			reinterpret_cast<void*>((intptr_t)_start));
	}

	void draw_elements_offset(std::size_t _start, std::size_t _count, std::size_t _offset)
	{
		GLWRAP_EC_CALL(glDrawElementsBaseVertex)(get_primitive_mode(),
			_count,
			get_element_type(),
			reinterpret_cast<void*>((intptr_t)_start),
			_offset);
	}

	void draw_elements_instanced(std::size_t _start, std::size_t _count, std::size_t _instances)
	{
		GLWRAP_EC_CALL(glDrawElementsInstanced)(get_primitive_mode(),
			_count,
			get_element_type(),
			reinterpret_cast<void*>((intptr_t)_start),
			_instances);
	}

	void use_program(program& _prog)
	{
		GLWRAP_EC_CALL(glUseProgram)(_prog.native_handle());
	}

	void use_vertex_array(vertex_array& _vert)
	{
		GLWRAP_EC_CALL(glBindVertexArray)(_vert.native_handle());
	}

	void use_transform_feedback(transform_feedback& _tf)
	{
		GLWRAP_EC_CALL(glBindTransformFeedback)(GL_TRANSFORM_FEEDBACK, _tf.native_handle());
	}

	template <typename T>
	void use_element_array(buffer<T>& _buff)
	{
		static_assert(std::is_same<T, ubyte_t>::value
			|| std::is_same<T, ushort_t>::value
			|| std::is_same<T, uint_t>::value
			, "must be ubyte, uint, ushort");

		GLWRAP_EC_CALL(glBindBuffer)(GL_ELEMENT_ARRAY_BUFFER, _buff.native_handle());

		m_element_type = detail::data_type_enum<T>();
	}

	// TODO: kill this?
	void use_primitive_mode(primitive _prim)
	{
		m_primitive_mode = static_cast<GLenum>(_prim);
	}

	void use_draw_framebuffer(framebuffer_reference _fb)
	{
		GLWRAP_EC_CALL(glBindFramebuffer)(GL_DRAW_FRAMEBUFFER, _fb.native_handle());
	}

	void use_read_framebuffer(framebuffer_reference _fb)
	{
		GLWRAP_EC_CALL(glBindFramebuffer)(GL_READ_FRAMEBUFFER, _fb.native_handle());
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
	GLenum get_primitive_mode() const
	{
		return m_primitive_mode;
	}

	GLenum get_element_type() const
	{
		return m_element_type;
	}

#ifndef GLWRAP_NO_SFML
	sf::RenderWindow& get_window()
	{
		return *m_sf_window;
	}
#endif

	// TODO: why do I save this?
	GLenum m_primitive_mode;
	GLenum m_element_type;

#ifndef GLWRAP_NO_SFML
	std::unique_ptr<sf::RenderWindow> m_sf_window;
#endif
};

}
