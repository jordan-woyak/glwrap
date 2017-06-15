
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

namespace GLWRAP_NAMESPACE
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
		, m_draw_indirect_buffer_binding(detail::adopt_value)
		, m_dispatch_indirect_buffer_binding(detail::adopt_value)
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

		detail::g_current_context = this;
	}

	~context()
	{
		detail::g_current_context = nullptr;

		// TODO: de-init
	}

	context(context const&) = delete;
	context& operator=(context const&) = delete;

	void set_clear_color(vec4 const& _color = vec4{})
	{
		GLWRAP_GL_CALL(glClearColor)(_color.x, _color.y, _color.z, _color.w);
	}

	void set_clear_stencil(int_t _index = 0)
	{
		GLWRAP_GL_CALL(glClearStencil)(_index);
	}

	void set_clear_depth(depth_t _depth = 1.0)
	{
		GLWRAP_GL_CALL(glClearDepth)(_depth);
	}

	// TODO: rename clear_draw_buffers?
	void clear(buffer_mask _mask)
	{
		GLWRAP_GL_CALL(glClear)(static_cast<bitfield_t>(_mask));
	};

	// TODO: rename all these to set_* ?
	void line_width(float_t _width)
	{
		GLWRAP_GL_CALL(glLineWidth)(_width);
	}

	void polygon_offset(float_t _factor, float_t _units)
	{
		GLWRAP_GL_CALL(glPolygonOffset)(_factor, _units);
	}

	void point_size(float_t _size)
	{
		GLWRAP_GL_CALL(glPointSize)(_size);
	}

	void provoking_vertex(provoke_mode _mode)
	{
		GLWRAP_GL_CALL(glProvokingVertex)(static_cast<GLenum>(_mode));
	}

	void front_face(orientation _orient)
	{
		GLWRAP_GL_CALL(glFrontFace)(static_cast<GLenum>(_orient));
	}

	void stencil_op(stencil_action _fail, stencil_action _pass_fail, stencil_action _pass, face _face = face::both)
	{
		GLWRAP_GL_CALL(glStencilOpSeparate)(static_cast<GLenum>(_face),
			static_cast<GLenum>(_fail), static_cast<GLenum>(_pass_fail), static_cast<GLenum>(_pass));
	}

	void stencil_func(comparison _test, int_t _ref, uint_t _mask, face _face = face::both)
	{
		GLWRAP_GL_CALL(glStencilFuncSeparate)(static_cast<GLenum>(_face), static_cast<GLenum>(_test), _ref, _mask);
	}

	void stencil_mask(uint_t _mask, face _face = face::both)
	{
		GLWRAP_GL_CALL(glStencilMaskSeparate)(static_cast<GLenum>(_face), _mask);
	}

	void viewport(ivec2 _pos, ivec2 _size)
	{
		GLWRAP_GL_CALL(glViewport)(_pos.x, _pos.y, _size.x, _size.y);
	}

	void scissor(ivec2 _pos, ivec2 _size)
	{
		GLWRAP_GL_CALL(glScissor)(_pos.x, _pos.y, _size.x, _size.y);
	}

	void blend_color(vec4 _color)
	{
		GLWRAP_GL_CALL(glBlendColor)(_color.x, _color.y, _color.z, _color.w);
	}

	void blend_equation(blend_mode _mode)
	{
		GLWRAP_GL_CALL(glBlendEquation)(static_cast<GLenum>(_mode));
	}

	void blend_equation(blend_mode _mode_rgb, blend_mode _mode_alpha)
	{
		GLWRAP_GL_CALL(glBlendEquationSeparate)(static_cast<GLenum>(_mode_rgb), static_cast<GLenum>(_mode_alpha));
	}

	void blend_func(blend_factor _sfactor, blend_factor _dfactor)
	{
		GLWRAP_GL_CALL(glBlendFunc)(static_cast<GLenum>(_sfactor), static_cast<GLenum>(_dfactor));
	}

	void blend_func(blend_factor _sfactor_rgb, blend_factor _dfactor_rgb, blend_factor _sfactor_alpha, blend_factor _dfactor_alpha)
	{
		GLWRAP_GL_CALL(glBlendFuncSeparate)(static_cast<GLenum>(_sfactor_rgb), static_cast<GLenum>(_dfactor_rgb),
			static_cast<GLenum>(_sfactor_alpha), static_cast<GLenum>(_dfactor_alpha));
	}

	void depth_func(comparison _compar)
	{
		GLWRAP_GL_CALL(glDepthFunc)(static_cast<GLenum>(_compar));
	}

	void depth_range(float_t _near, float_t _far)
	{
		GLWRAP_GL_CALL(glDepthRangef)(_near, _far);
	}

	void depth_mask(bool_t _enable)
	{
		GLWRAP_GL_CALL(glDepthMask)(_enable);
	}

	void sample_coverage(float_t _value, bool_t _invert)
	{
		GLWRAP_GL_CALL(glSampleCoverage)(_value, _invert);
	}

	void sample_mask(uint_t _mask_number, bitfield_t _mask)
	{
		// TODO: assert _mask_number < GL_MAX_SAMPLE_MASK_WORDS

		GLWRAP_GL_CALL(glSampleMaski)(_mask_number, _mask);
	}

	void color_mask(basic_vec<bool, 4> const& _mask)
	{
		GLWRAP_GL_CALL(glColorMask)(_mask.x, _mask.y, _mask.z, _mask.w);
	}

	// TODO: This is not in OpenGL ES
	template <typename T>
	void color_mask(fragdata_location<T> const& _buf, basic_vec<bool, 4> const& _mask)
	{
		GLWRAP_GL_CALL(glColorMaski)(_buf.get_index(), _mask.x, _mask.y, _mask.z, _mask.w);
	}

	void hint(hint_target _target, hint_value _value)
	{
		GLWRAP_GL_CALL(glHint)(static_cast<GLenum>(_target), static_cast<GLenum>(_value));
	}

	// TODO: function name? insert_* ?
	void memory_barrier(enum memory_barrier _barrier)
	{
		GLWRAP_GL_CALL(glMemoryBarrier)(static_cast<GLenum>(_barrier));
	}

	void memory_barrier_by_region(enum memory_barrier _barrier)
	{
		GLWRAP_GL_CALL(glMemoryBarrierByRegion)(static_cast<GLenum>(_barrier));
	}
	
	void enable(capability _cap)
	{
		GLWRAP_GL_CALL(glEnable)(static_cast<GLenum>(_cap));
	}

	void disable(capability _cap)
	{
		GLWRAP_GL_CALL(glDisable)(static_cast<GLenum>(_cap));
	}

	bool is_enabled(capability _cap)
	{
		return GLWRAP_GL_CALL(glIsEnabled)(static_cast<GLenum>(_cap));
	}

	template <texture_type T, typename D>
	void bind_texture(texture_unit<shader::basic_sampler<T, D>> const& _unit, basic_texture<T, D> const& _texture)
	{
		if (is_extension_present(GL_ARB_direct_state_access))
		{
			GLWRAP_GL_CALL(glBindTextureUnit)(_unit.get_index(), _texture.native_handle());
		}
		else
		{
			GLWRAP_GL_CALL(glActiveTexture)(GL_TEXTURE0 + _unit.get_index());
			GLWRAP_GL_CALL(glBindTexture)(_texture.target, _texture.native_handle());
		}
	}

	// TODO: the format parameter, embed it into the image_unit type

	// Layered Binding (all layers) for layered textures
	// Non-layered (layer-0) for non-layered textures.
	template <texture_type T, typename D>
	void bind_image_texture(image_unit<shader::basic_image<T, D>> const& _unit,
		basic_texture<T, D> const& _texture, int_t _level, image_access _access, typename image_format<D>::enum_type _format)
	{
		//static_assert(detail::texture_traits<T>::has_layers,
			//"Layered image binding is only valid for textures with layers.");
		
		GLWRAP_GL_CALL(glBindImageTexture)(
			_unit.get_index(),
			_texture.native_handle(),
			_level,
			detail::texture_traits<T>::has_layers,
			0,
			static_cast<enum_t>(_access),
			static_cast<enum_t>(_format));
	}

	// Non-Layered Binding (single layer):
	template <texture_type T, typename D>
	void bind_image_texture(image_unit<shader::basic_image<detail::texture_traits<T>::layer_target, D>> const& _unit,
		basic_texture<T, D> const& _texture, int_t _level, int_t _layer, image_access _access, typename image_format<D>::enum_type _format)
	{
		// TODO: should this fail to compile for non-layered textures?
		// force use of the other overload?
		
		GLWRAP_GL_CALL(glBindImageTexture)(
			_unit.get_index(),
			_texture.native_handle(),
			_level,
			GL_FALSE,
			_layer,
			static_cast<enum_t>(_access),
			static_cast<enum_t>(_format));
	}

	template <typename T>
	void bind_sampler(texture_unit<T> const& _unit, sampler const& _sampler)
	{
		GLWRAP_GL_CALL(glBindSampler)(_unit.get_index(), _sampler.native_handle());
	}

	// TODO: allow bind to take a nullptr or something instead of this.
	template <typename T>
	void unbind_sampler(texture_unit<T> const& _unit)
	{
		GLWRAP_GL_CALL(glBindSampler)(_unit.get_index(), 0);
	}

	void start_transform_feedback(primitive _mode)
	{
		GLWRAP_GL_CALL(glBeginTransformFeedback)(static_cast<GLenum>(_mode));
	}

	void pause_transform_feedback()
	{
		GLWRAP_GL_CALL(glPauseTransformFeedback)();
	}

	void resume_transform_feedback()
	{
		GLWRAP_GL_CALL(glResumeTransformFeedback)();
	}

	void stop_transform_feedback()
	{
		GLWRAP_GL_CALL(glEndTransformFeedback)();
	}

	// TODO: move into fbo class?
	// TODO: rename
	void blit_pixels(ivec2 const& _src_begin, ivec2 const& _src_end,
		ivec2 const& _dst_begin, ivec2 const& _dst_end, buffer_mask _mask, filter _filter)
	{
		GLWRAP_GL_CALL(glBlitFramebuffer)(
			_src_begin.x, _src_begin.y, _src_end.x, _src_end.y,
			_dst_begin.x, _dst_begin.y, _dst_end.x, _dst_end.y,
			static_cast<bitfield_t>(_mask), static_cast<enum_t>(_filter));
	}

	void flush()
	{
		GLWRAP_GL_CALL(glFlush)();
	}

	void finish()
	{
		GLWRAP_GL_CALL(glFinish)();
	}

	sync fence_sync(sync_condition _cond, bitfield_t _flags)
	{
		return sync{_cond, _flags};
	}

	// TODO: multi draw (with looping fallback on ES)
	// TODO: draw range elements base vertex // not on ES

	void draw_arrays(primitive _mode, int_t _offset, sizei_t _count)
	{
		GLWRAP_GL_CALL(glDrawArrays)(
			static_cast<enum_t>(_mode),
			_offset,
			_count);
	}

	// TODO: should I break MultiDraw into its own function?
	template <typename A>
	void draw_arrays_indirect(primitive _mode, buffer_iterator<draw_arrays_indirect_cmd, A> _cmd,
		sizei_t _drawcount = 1)
	{
		m_draw_indirect_buffer_binding.set(_cmd.get_buffer());

		if (is_extension_present(GL_ARB_multi_draw_indirect))
		{
			GLWRAP_GL_CALL(glMultiDrawArraysIndirect)(
				static_cast<enum_t>(_mode),
				_cmd.get_offset(),
				_drawcount,
				_cmd.get_stride());
		}
		else
		{
			while (_drawcount--)
			{
				GLWRAP_GL_CALL(glDrawArraysIndirect)(
					static_cast<enum_t>(_mode),
					_cmd.get_offset());

					++_cmd;
			}
		}
	}

	void draw_arrays_instanced(primitive _mode, int_t _offset, sizei_t _count, sizei_t _instances)
	{
		GLWRAP_GL_CALL(glDrawArraysInstanced)(
			static_cast<enum_t>(_mode),
			_offset,
			_count,
			_instances);
	}

	// TODO: should these just take the element array and bind it?
	// TODO: good parameter order?
	void draw_elements(primitive _mode, sizei_t _start, sizei_t _count)
	{
		GLWRAP_GL_CALL(glDrawElements)(
			static_cast<enum_t>(_mode),
			_count,
			get_element_type(),
			(ubyte_t*)0 + _start * m_element_type_size);
	}

	// TODO: should I break MultiDraw into its own function?
	template <typename A>
	void draw_elements_indirect(primitive _mode, buffer_iterator<draw_elements_indirect_cmd, A> _cmd,
		sizei_t _drawcount = 1)
	{
		m_draw_indirect_buffer_binding.set(_cmd.get_buffer());

		if (is_extension_present(GL_ARB_multi_draw_indirect))
		{
			GLWRAP_GL_CALL(glMultiDrawElementsIndirect)(
				static_cast<enum_t>(_mode),
				get_element_type(),
				_cmd.get_offset(),
				_drawcount,
				_cmd.get_stride());
		}
		else
		{		
			while (_drawcount--)
			{
				GLWRAP_GL_CALL(glDrawElementsIndirect)(
					static_cast<enum_t>(_mode),
					get_element_type(),
					_cmd.get_offset());

					++_cmd;
			}
		}
	}

	// TODO: should these just take the element array and bind it?
	// TODO: good parameter order?
	void draw_elements_instanced(primitive _mode, sizei_t _start, sizei_t _count, sizei_t _instances)
	{
		GLWRAP_GL_CALL(glDrawElementsInstanced)(
			static_cast<enum_t>(_mode),
			_count,
			get_element_type(),
			(ubyte_t*)0 + _start * m_element_type_size,
			_instances);
	}

	void draw_range_elements(primitive _mode, sizei_t _start, uint_t _min, uint_t _max, sizei_t _count)
	{
		GLWRAP_GL_CALL(glDrawRangeElements)(
			static_cast<enum_t>(_mode),
			_min,
			_max,
			_count,
			get_element_type(),
			(ubyte_t*)0 + _start * m_element_type_size);
	}

	void dispatch_compute(const uvec3& _num_groups)
	{
		GLWRAP_GL_CALL(glDispatchCompute)(_num_groups.x, _num_groups.y, _num_groups.z);
	}

	// TODO: uvec3 is fine or have an actual cmd struct?
	template <typename A>
	void dispatch_compute_indirect(const buffer_iterator<uvec3, A>& _cmd)
	{
		m_dispatch_indirect_buffer_binding.set(_cmd.get_buffer());

		GLWRAP_GL_CALL(glDispatchComputeIndirect)(_cmd.get_offset() - (ubyte_t*)0);
	}

	void use_program(program& _prog)
	{
		GLWRAP_GL_CALL(glUseProgram)(_prog.native_handle());
	}

	// TODO: Do I like nullptr here?
	void use_program(std::nullptr_t)
	{
		GLWRAP_GL_CALL(glUseProgram)(0);
	}

	// FYI: "If a program is bound with glUseProgram, then any bound program pipelines will be ignored."
	// TODO: allow un-using a program pipeline, use nullptr or something
	void use_program_pipeline(program_pipeline& _prog)
	{
		GLWRAP_GL_CALL(glBindProgramPipeline)(_prog.native_handle());
	}

	void use_vertex_array(vertex_array& _vert)
	{
		GLWRAP_GL_CALL(glBindVertexArray)(_vert.native_handle());
	}

	void use_transform_feedback(transform_feedback& _tf)
	{
		GLWRAP_GL_CALL(glBindTransformFeedback)(GL_TRANSFORM_FEEDBACK, _tf.native_handle());
	}

	template <typename T, sizei_t S>
	void bind_buffer(atomic_counter_binding<T> const& _binding, const static_buffer_iterator<T, S>& _iter)
	{
		static_assert(S % 4 == 0, "Atomic counter buffer must be aligned to 4 bytes.");
		
		GLWRAP_GL_CALL(glBindBufferRange)(GL_ATOMIC_COUNTER_BUFFER, _binding.get_index(),
			_iter.get_buffer(), _iter.get_offset() - (ubyte_t*)0, _iter.get_stride() * 1);
	}

	template <typename T>
	void bind_buffer(shader_storage_location<T> const& _binding, const shader_storage_buffer_iterator<T>& _iter)
	{
		GLWRAP_GL_CALL(glBindBufferRange)(GL_SHADER_STORAGE_BUFFER, _binding.get_index(),
			_iter.get_buffer(), _iter.get_offset() - (ubyte_t*)0, _iter.get_stride() * 1);
	}

	// TODO: rename?
	template <typename T>
	void bind_buffer(shader_storage_location<T[]> const& _binding, const shader_storage_buffer_iterator<T>& _iter, uint_t _count)
	{
		GLWRAP_GL_CALL(glBindBufferRange)(GL_SHADER_STORAGE_BUFFER, _binding.get_index(),
			_iter.get_buffer(), _iter.get_offset() - (ubyte_t*)0, _iter.get_stride() * _count);
	}

	template <typename T>
	void bind_buffer(uniform_block_location<T> const& _binding, const uniform_buffer_iterator<T>& _iter)
	{
		GLWRAP_GL_CALL(glBindBufferRange)(GL_UNIFORM_BUFFER, _binding.get_index(),
			_iter.get_buffer(), _iter.get_offset() - (ubyte_t*)0, _iter.get_stride() * 1);
	}
	
	template <typename T>
	void use_element_array(buffer<T>& _buff)
	{
		static_assert(std::is_same<T, ubyte_t>::value
			|| std::is_same<T, ushort_t>::value
			|| std::is_same<T, uint_t>::value
			, "must be ubyte, uint, ushort");

		GLWRAP_GL_CALL(glBindBuffer)(GL_ELEMENT_ARRAY_BUFFER, _buff.native_handle());

		m_element_type = detail::data_type_enum<T>();
		m_element_type_size = sizeof(T);
	}

	void use_draw_framebuffer(const framebuffer_reference& _fb)
	{
		GLWRAP_GL_CALL(glBindFramebuffer)(GL_DRAW_FRAMEBUFFER, _fb.native_handle());
	}

	void use_read_framebuffer(const framebuffer_reference& _fb)
	{
		GLWRAP_GL_CALL(glBindFramebuffer)(GL_READ_FRAMEBUFFER, _fb.native_handle());
	}

	// Binds draw and read framebuffer
	void use_framebuffer(const framebuffer_reference& _fb)
	{
		GLWRAP_GL_CALL(glBindFramebuffer)(GL_FRAMEBUFFER, _fb.native_handle());
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

	void release_shader_compiler()
	{
		GLWRAP_GL_CALL(glReleaseShaderCompiler)();
	}

	// TODO: allow customization of the debug level
	void enable_debugging()
	{
		if (GL_KHR_debug)
		{
			GLWRAP_GL_CALL(glDebugMessageCallback)(&context::debug_message_callback, this);
			GLWRAP_GL_CALL(glDebugMessageControl)(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);
			GLWRAP_GL_CALL(glEnable)(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			GLWRAP_GL_CALL(glEnable)(GL_DEBUG_OUTPUT);
		}
		else
		{
			detail::g_check_every_gl_call = true;
		}
	}

	void disable_debugging()
	{
		if (GL_KHR_debug)
		{
			GLWRAP_GL_CALL(glDisable)(GL_DEBUG_OUTPUT);
		}
		else
		{
			detail::g_check_every_gl_call = false;
		}
	}

	void enable_profiling()
	{
		detail::g_profile_every_gl_call = true;
	}

	void disable_profiling()
	{
		detail::g_profile_every_gl_call = false;
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

	// TODO: these will bind 0 on context destruction, that isn't really needed..
	detail::scoped_value<detail::parameter::draw_indirect_buffer> m_draw_indirect_buffer_binding;
	detail::scoped_value<detail::parameter::dispatch_indirect_buffer> m_dispatch_indirect_buffer_binding;
};

}
