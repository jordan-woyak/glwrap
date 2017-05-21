
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
	explicit context()
		: m_element_type()
		, m_element_type_size()
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

		//glewInit();

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

	// TODO: This is not in OpenGL ES
	template <typename T>
	void color_mask(fragdata_location<T> const& _buf, basic_vec<bool, 4> const& _mask)
	{
		GLWRAP_EC_CALL(glColorMaski)(_buf.get_index(), _mask.x, _mask.y, _mask.z, _mask.w);
	}

	void hint(hint_target _target, hint_value _value)
	{
		GLWRAP_EC_CALL(glHint)(static_cast<GLenum>(_target), static_cast<GLenum>(_value));
	}

	// TODO: function name?
	void memory_barrier(gl::memory_barrier _barrier)
	{
		GLWRAP_EC_CALL(glMemoryBarrier)(static_cast<GLenum>(_barrier));
	}

	void memory_barrier_by_region(gl::memory_barrier _barrier)
	{
		GLWRAP_EC_CALL(glMemoryBarrierByRegion)(static_cast<GLenum>(_barrier));
	}
	
	void enable(capability _cap)
	{
		GLWRAP_EC_CALL(glEnable)(static_cast<GLenum>(_cap));
	}

	void disable(capability _cap)
	{
		GLWRAP_EC_CALL(glDisable)(static_cast<GLenum>(_cap));
	}

	template <texture_type T, typename D>
	void bind_texture(texture_unit<shader::basic_sampler<T, D>> const& _unit, basic_texture<T, D> const& _texture)
	{
		GLWRAP_EC_CALL(glActiveTexture)(GL_TEXTURE0 + _unit.get_index());
		GLWRAP_EC_CALL(glBindTexture)(_texture.target, _texture.native_handle());
	}

	// TODO: work for more than image_2d
	void bind_image_texture(texture_unit<shader::image_2d> const& _unit, texture_2d const& _texture)
	{
		// TODO: levels
		// TODO: layer
		// TODO: access
		// TODO: format
		
		GLWRAP_EC_CALL(glBindImageTexture)(
			_unit.get_index(),
			_texture.native_handle(),
			0,
			GL_FALSE,
			0,
			GL_READ_WRITE,

			// TODO: this format must match exactly the format declared in the shader
			// (if reads are to be performed)

			// it also needs to match by size (or more safely, by class) with the texture's
			// internal format

			// TODO: perhaps I could type the texture_unit_location with this format?
			GL_RGBA32F);
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

/*
	// TODO: should allow reference to buffer element as well
	// range is probably not needed/wanted for uniform buffer
	template <typename T>
	void bind_buffer(uniform_block_binding<T> const& _unit, uniform_buffer_iterator<T> const& _iter, uint_t _size)
	{
		GLWRAP_EC_CALL(glBindBufferRange)(GL_UNIFORM_BUFFER, _unit.get_index(),
			_iter.get_buffer(),	_iter.get_offset() - (ubyte_t*)0, _iter.get_stride() * _size);
	}
*/
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
		return sync{_cond, _flags};
	}

	// TODO: multi draw needed?
	// TODO: draw range elements [base vertex]

	void draw_arrays(primitive _mode, int_t _offset, sizei_t _count)
	{
		GLWRAP_EC_CALL(glDrawArrays)(
			static_cast<enum_t>(_mode),
			_offset,
			_count);
	}

	// TODO: allow buffer offset
	void draw_arrays_indirect(primitive _mode, const draw_arrays_indirect_cmd* _cmd)
	{
		GLWRAP_EC_CALL(glDrawArraysIndirect)(
			static_cast<enum_t>(_mode),
			_cmd);
	}

	void draw_arrays_instanced(primitive _mode, int_t _offset, sizei_t _count, sizei_t _instances)
	{
		GLWRAP_EC_CALL(glDrawArraysInstanced)(
			static_cast<enum_t>(_mode),
			_offset,
			_count,
			_instances);
	}

	// TODO: should these just take the element array and bind it?
	// TODO: good parameter order?
	void draw_elements(primitive _mode, sizei_t _start, sizei_t _count)
	{
		GLWRAP_EC_CALL(glDrawElements)(
			static_cast<enum_t>(_mode),
			_count,
			get_element_type(),
			(ubyte_t*)0 + _start * m_element_type_size);
	}

	// TODO: allow buffer offset
	void draw_elements_indirect(primitive _mode, const draw_elements_indirect_cmd* _cmd)
	{
		GLWRAP_EC_CALL(glDrawElementsIndirect)(
			static_cast<enum_t>(_mode),
			get_element_type(),
			_cmd);
	}

	// TODO: should these just take the element array and bind it?
	// TODO: good parameter order?
	void draw_elements_instanced(primitive _mode, sizei_t _start, sizei_t _count, sizei_t _instances)
	{
		GLWRAP_EC_CALL(glDrawElementsInstanced)(
			static_cast<enum_t>(_mode),
			_count,
			get_element_type(),
			(ubyte_t*)0 + _start * m_element_type_size,
			_instances);
	}

	void draw_range_elements(primitive _mode, sizei_t _start, uint_t _min, uint_t _max, sizei_t _count)
	{
		GLWRAP_EC_CALL(glDrawRangeElements)(
			static_cast<enum_t>(_mode),
			_min,
			_max,
			_count,
			get_element_type(),
			(ubyte_t*)0 + _start * m_element_type_size);
	}

	// TODO: dispatch compute indirect
	void dispatch_compute(const uvec3& _num_groups)
	{
		GLWRAP_EC_CALL(glDispatchCompute)(_num_groups.x, _num_groups.y, _num_groups.z);
	}

	void use_program(program& _prog)
	{
		GLWRAP_EC_CALL(glUseProgram)(_prog.native_handle());
	}

	// TODO: rename to juse use_program?
	void use_program_pipeline(program_pipeline& _prog)
	{
		GLWRAP_EC_CALL(glBindProgramPipeline)(_prog.native_handle());
	}

	void use_vertex_array(vertex_array& _vert)
	{
		GLWRAP_EC_CALL(glBindVertexArray)(_vert.native_handle());
	}

	void use_transform_feedback(transform_feedback& _tf)
	{
		GLWRAP_EC_CALL(glBindTransformFeedback)(GL_TRANSFORM_FEEDBACK, _tf.native_handle());
	}

	// TODO: iterator needs to be atomic counter aligned
	template <typename T, sizei_t S>
	void bind_buffer(atomic_counter_binding<T> const& _binding, const static_buffer_iterator<T, S>& _iter)
	{
		GLWRAP_EC_CALL(glBindBufferRange)(GL_ATOMIC_COUNTER_BUFFER, _binding.get_index(),
			_iter.get_buffer(), _iter.get_offset() - (ubyte_t*)0, _iter.get_stride());
	}

	// TODO: iterator needs to be shader storage aligned
	// TODO: end iterator might be more sensible
	template <typename T, sizei_t S>
	void bind_buffer(shader_storage_location<T[]> const& _binding, const static_buffer_iterator<T, S>& _iter, sizei_t _size)
	{
		GLWRAP_EC_CALL(glBindBufferRange)(GL_SHADER_STORAGE_BUFFER, _binding.get_index(),
			_iter.get_buffer(), _iter.get_offset() - (ubyte_t*)0, _iter.get_stride() * _size);
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

	void use_draw_framebuffer(const framebuffer_reference& _fb)
	{
		GLWRAP_EC_CALL(glBindFramebuffer)(GL_DRAW_FRAMEBUFFER, _fb.native_handle());
	}

	void use_read_framebuffer(const framebuffer_reference& _fb)
	{
		GLWRAP_EC_CALL(glBindFramebuffer)(GL_READ_FRAMEBUFFER, _fb.native_handle());
	}

	// Binds draw and read framebuffer
	void use_framebuffer(const framebuffer_reference& _fb)
	{
		GLWRAP_EC_CALL(glBindFramebuffer)(GL_FRAMEBUFFER, _fb.native_handle());
	}

	// TODO: kill these:
/*
	color_attachment color_buffer(uint_t _index)
	{
		return {static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + _index)};
	}
*/
	color_attachment stencil_buffer()
	{
		return {GL_STENCIL_ATTACHMENT};
	}

	color_attachment depth_buffer()
	{
		return {GL_DEPTH_ATTACHMENT};
	}

	std::string get_vendor_name()
	{
		return detail::get_string(GL_VENDOR);
	}

	std::string get_renderer_name()
	{
		return detail::get_string(GL_RENDERER);
	}

	std::string get_version()
	{
		return detail::get_string(GL_VERSION);
	}

	std::string get_shading_language_version()
	{
		return detail::get_string(GL_SHADING_LANGUAGE_VERSION);
	}

	// TODO: allow customization of the debug level
	void enable_debugging()
	{
		if (GL_KHR_debug)
		{
			GLWRAP_EC_CALL(glDebugMessageCallback)(&context::debug_message_callback, this);
			GLWRAP_EC_CALL(glDebugMessageControl)(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);
			GLWRAP_EC_CALL(glEnable)(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			GLWRAP_EC_CALL(glEnable)(GL_DEBUG_OUTPUT);
		}
	}

	void disable_debugging()
	{
		if (GL_KHR_debug)
		{
			GLWRAP_EC_CALL(glDisable)(GL_DEBUG_OUTPUT);
		}
	}

private:
	void debug_message_handler(
		GLenum _source,
		GLenum _type,
		GLuint _id,
		GLenum _severity,
		GLsizei _length,
		const GLchar* _message) const
	{
		std::string msg("Debug: ");

		msg += "source: ";
		switch (_source)
		{
		case GL_DEBUG_SOURCE_API:
			msg += "API";
			break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
			msg += "WINDOW_SYSTEM";
			break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER:
			msg += "SHADER_COMPILER";
			break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:
			msg += "THIRD_PARTY";
			break;
		case GL_DEBUG_SOURCE_APPLICATION:
			msg += "APPLICATION";
			break;
		case GL_DEBUG_SOURCE_OTHER:
			msg += "OTHER";
			break;
		default:
			msg += "UNKNOWN";
			break;
		}
		
		msg += ", type: ";
		switch (_type)
		{
		case GL_DEBUG_TYPE_ERROR:
			msg += "ERROR";
			break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			msg += "DEPRECATED_BEHAVIOR";
			break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			msg += "UNDEFINED_BEHAVIOR";
			break;
		case GL_DEBUG_TYPE_PORTABILITY:
			msg += "PORTABILITY";
			break;
		case GL_DEBUG_TYPE_PERFORMANCE:
			msg += "PERFORMANCE";
			break;
			// TODO: marker, push-group, pop-group
		case GL_DEBUG_TYPE_OTHER:
			msg += "OTHER";
			break;
		default:
			msg += "UNKNOWN";
			break;
		}

		msg += ", id: " + std::to_string(_id);
		
		msg += ", severity: ";
		switch (_severity)
		{
		case GL_DEBUG_SEVERITY_LOW:
			msg += "LOW";
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			msg += "MEDIUM";
			break;
		case GL_DEBUG_SEVERITY_HIGH:
			msg += "HIGH";
			break;
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			msg += "NOTIFICATION";
			break;
		default:
			msg += "UNKNOWN";
			break;
		}
		
		std::cerr << msg << ", msg: " << std::string(_message, _length) << std::endl;
	}

	static void debug_message_callback(
		GLenum _source,
		GLenum _type,
		GLuint _id,
		GLenum _severity,
		GLsizei _length,
		const GLchar* _message,
		const void* _userParam)
	{
		static_cast<const context*>(_userParam)->debug_message_handler
			(_source, _type, _id, _severity, _length, _message);
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

	GLenum m_element_type;
	sizei_t m_element_type_size;

#ifndef GLWRAP_NO_SFML
	std::unique_ptr<sf::RenderWindow> m_sf_window;
#endif
};

}
