
#pragma once

#include <algorithm>

#include "variable.hpp"
#include "texture.hpp"
#include "renderbuffer.hpp"

#include "detail/framebuffer.hpp"

namespace GLWRAP_NAMESPACE
{

// TODO: kill?
struct color_attach_point : attach_point
{
	friend class context;

private:
	color_attach_point(GLenum _attachment)
		: attach_point(_attachment)
	{}
};

// TODO: rename
// front/back/stecil/depth attachments
class color_attachment
{
public:
	color_attachment(enum_t _index = GL_NONE)
		: m_index(_index)
	{}

	// TODO: rename
	enum_t get_index() const
	{
		return m_index;
	}

	enum_t m_index;
};

// rename color_buffer_enumerator?
class color_attachment_enumerator
{
public:	
	// TODO: really need context?
	color_attachment_enumerator(context& _context)
		: m_current_index()
		, m_max_locations()
	{
		detail::gl_get(GL_MAX_COLOR_ATTACHMENTS, &m_max_locations);
	}

	color_attachment get()
	{
		if (m_current_index >= m_max_locations)
			throw exception(0);
			
		color_attachment ind(GL_COLOR_ATTACHMENT0 + m_current_index);

		m_current_index += 1;

		return ind;
	}

private:
	int_t m_current_index;
	int_t m_max_locations;
};

// TODO: framebuffer parameters: default size and such

namespace detail
{

struct framebuffer_obj
{
	static void create_objs(sizei_t _n, uint_t* _objs)
	{
		if (is_extension_present(GL_ARB_direct_state_access))
		{
			GLWRAP_GL_CALL(glCreateFramebuffers)(_n, _objs);
		}
		else
		{
			GLWRAP_GL_CALL(glGenFramebuffers)(_n, _objs);
		}
	}

	static void delete_objs(sizei_t _n, uint_t* _objs)
	{
		GLWRAP_GL_CALL(glDeleteFramebuffers)(_n, _objs);
	}
};

}

enum class framebuffer_status : enum_t
{
	// "Additionally, if an error occurs, zero is returned."
	error = 0,
	undefined = GL_FRAMEBUFFER_UNDEFINED,
	incomplete_attachment = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
	incomplete_missing_attachment = GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT,
	incomplete_draw_buffer = GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER,
	incomplete_read_buffer = GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER,
	unsupported = GL_FRAMEBUFFER_UNSUPPORTED,
	incomplete_multisample = GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE,
	incomplete_layer_target = GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS,
};

class framebuffer : public detail::globject<detail::framebuffer_obj>
{
public:
	explicit framebuffer(context&)
	{}

	// TODO: glInvalidateFramebuffer

	// TODO: go back to the gl::renderbuffer_attachment, gl::texture_attachment impl
	// or a single overload with an internal functor

	// TODO: almost think the implementation of this belongs in renderbuffer
	// TODO: support depth and stencil attachments
	void bind_attachment(color_attachment const& _point, renderbuffer const& _attachment)
	{
		if (is_extension_present(GL_ARB_direct_state_access))
		{
			GLWRAP_GL_CALL(glNamedFramebufferRenderbuffer)(native_handle(), _point.get_index(), GL_RENDERBUFFER, _attachment.native_handle());
		}
		else
		{
			detail::scoped_value<detail::parameter::read_framebuffer> binding(native_handle());
			
			GLWRAP_GL_CALL(glFramebufferRenderbuffer)(GL_READ_FRAMEBUFFER, _point.get_index(), GL_RENDERBUFFER, _attachment.native_handle());
		}
	}

	// TODO: almost think the implementation of this belongs in texture
	// TODO: support depth and stencil attachments
	// TODO: support 3d and array textures
	// TODO: I don't like this level interface
	void bind_attachment(color_attachment const& _point, texture_2d const& _attachment, uint_t _level)
	{
		if (is_extension_present(GL_ARB_direct_state_access))
		{
			GLWRAP_GL_CALL(glNamedFramebufferTexture)(native_handle(), _point.get_index(), _attachment.native_handle(), _level);
		}
		else
		{
			detail::scoped_value<detail::parameter::read_framebuffer> binding(native_handle());
			
			GLWRAP_GL_CALL(glFramebufferTexture2D)(GL_READ_FRAMEBUFFER, _point.get_index(), _attachment.target, _attachment.native_handle(), _level);
		}
	}

	// TODO: support depth and stencil attachments
	void unbind_attachment(color_attachment const& _point)
	{
		if (is_extension_present(GL_ARB_direct_state_access))
		{
			GLWRAP_GL_CALL(glNamedFramebufferRenderbuffer)(native_handle(), _point.get_index(), GL_RENDERBUFFER, 0);
		}
		else
		{
			detail::scoped_value<detail::parameter::read_framebuffer> binding(native_handle());
			
			GLWRAP_GL_CALL(glFramebufferRenderbuffer)(GL_READ_FRAMEBUFFER, _point.get_index(), GL_RENDERBUFFER, 0);
		}
	}

	// TODO: support depth and stencil attachments
	// TODO: should I even expose this function directly? it doesn't reference the fragdata numbers..
	void bind_draw_buffers(const std::vector<color_attachment>& _attachments)
	{
		std::vector<enum_t> bufs(_attachments.size());
		std::transform(_attachments.begin(), _attachments.end(), bufs.begin(), std::mem_fn(&color_attachment::get_index));
		
		if (is_extension_present(GL_ARB_direct_state_access))
		{
			GLWRAP_GL_CALL(glNamedFramebufferDrawBuffers)(native_handle(), bufs.size(), bufs.data());
		}
		else
		{
			detail::scoped_value<detail::parameter::draw_framebuffer> binding(native_handle());
			
			GLWRAP_GL_CALL(glDrawBuffers)(bufs.size(), bufs.data());
		}
	}

	// TODO: support constants GL_BACK and GL_NONE
	void bind_read_buffer(color_attachment const& _attachment)
	{
		if (is_extension_present(GL_ARB_direct_state_access))
		{
			GLWRAP_GL_CALL(glNamedFramebufferReadBuffer)(native_handle(), _attachment.get_index());
		}
		else
		{
			detail::scoped_value<detail::parameter::read_framebuffer> binding(native_handle());
			
			GLWRAP_GL_CALL(glReadBuffer)(_attachment.get_index());
		}
	}

	framebuffer_status get_draw_status()
	{
		return get_status<detail::parameter::draw_framebuffer, GL_DRAW_FRAMEBUFFER>();
	}

	framebuffer_status get_read_status()
	{
		return get_status<detail::parameter::read_framebuffer, GL_READ_FRAMEBUFFER>();
	}

	void set_default_width(uint_t _val)
	{
		set_parameter_raw(GL_FRAMEBUFFER_DEFAULT_WIDTH, _val);
	}

	void set_default_height(uint_t _val)
	{
		set_parameter_raw(GL_FRAMEBUFFER_DEFAULT_HEIGHT, _val);
	}

	void set_default_samples(uint_t _val)
	{
		set_parameter_raw(GL_FRAMEBUFFER_DEFAULT_SAMPLES, _val);
	}

	void set_default_fixed_sample_locations(bool _val)
	{
		set_parameter_raw(GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS, _val);
	}

/*
	// Not in GL ES
	void set_default_layers(uint_t _val)
	{
		set_parameter_raw(GL_FRAMEBUFFER_DEFAULT_LAYERS, _val);
	}
*/

	// TODO: unsigned and signed versions:
	template <typename T>
	void clear_color_buffer(const fragdata_location<T>& _loc, vec4 _val)
	{
		clear_buffer(GL_COLOR, _loc.get_index(), _val);
	}

	// TODO: remove the location param?
	template <typename T>
	void clear_depth_buffer(const fragdata_location<T>& _loc, float_t _val)
	{
		clear_buffer(GL_DEPTH, _loc.get_index(), _val);
	}

	// TODO: remove the location param?
	template <typename T>
	void clear_stencil_buffer(const fragdata_location<T>& _loc, int_t _val)
	{
		clear_buffer(GL_DEPTH, _loc.get_index(), _val);
	}

	// TODO: remove the location param?
	template <typename T>
	void clear_depth_stencil_buffer(const fragdata_location<T>& _loc, float_t _depth, int_t _stencil)
	{
		if (is_extension_present(GL_ARB_direct_state_access))
		{
			GLWRAP_GL_CALL(glClearNamedFramebufferfi)(native_handle(), _loc.get_index(), _depth, _stencil);
		}
		else
		{
			detail::scoped_value<detail::parameter::draw_framebuffer> binding(native_handle());

			GLWRAP_GL_CALL(glClearBufferfi)(_loc.get_index(), _depth, _stencil);
		}
	}

	// TODO: rename
	void blit(framebuffer& _read_fb,
		ivec2 const& _src_begin, ivec2 const& _src_end,
		ivec2 const& _dst_begin, ivec2 const& _dst_end, buffer_mask _mask, filter _filter)
	{
		if (is_extension_present(GL_ARB_direct_state_access))
		{
			GLWRAP_GL_CALL(glBlitNamedFramebuffer)(
				_read_fb.native_handle(), native_handle(),
				_src_begin.x, _src_begin.y, _src_end.x, _src_end.y,
				_dst_begin.x, _dst_begin.y, _dst_end.x, _dst_end.y,
				static_cast<bitfield_t>(_mask), static_cast<enum_t>(_filter));
		}
		else
		{
			detail::scoped_value<detail::parameter::read_framebuffer> r_binding(_read_fb.native_handle());
			detail::scoped_value<detail::parameter::draw_framebuffer> d_binding(native_handle());
			
			GLWRAP_GL_CALL(glBlitFramebuffer)(
				_src_begin.x, _src_begin.y, _src_end.x, _src_end.y,
				_dst_begin.x, _dst_begin.y, _dst_end.x, _dst_end.y,
				static_cast<bitfield_t>(_mask), static_cast<enum_t>(_filter));
		}
	}

private:
	// TODO: Get out of this class..
	template <typename V>
	void clear_buffer(enum_t _buffer, int_t _index, const V& _val)
	{
		if (is_extension_present(GL_ARB_direct_state_access))
		{
			detail::gl_clear_named_framebuffer(native_handle(), _buffer, _index, value_ptr(_val));
		}
		else
		{
			detail::scoped_value<detail::parameter::draw_framebuffer> binding(native_handle());

			detail::gl_clear_buffer(_buffer, _index, value_ptr(_val));
		}
	}

	// TODO: Get out of this class..
	template <typename Param, enum_t Target>
	framebuffer_status get_status()
	{
		enum_t status = {};
		
		if (is_extension_present(GL_ARB_direct_state_access))
		{
			status = GLWRAP_GL_CALL(glCheckNamedFramebufferStatus)(native_handle(), Target);
		}
		else
		{
			detail::scoped_value<Param> binding(native_handle());

			status = GLWRAP_GL_CALL(glCheckFramebufferStatus)(Target);
		}

		return static_cast<framebuffer_status>(status);
	}

	void set_parameter_raw(GLenum _pname, int_t _val)
	{
		if (is_extension_present(GL_ARB_direct_state_access))
		{
			GLWRAP_GL_CALL(glNamedFramebufferParameteri)(native_handle(), _pname, _val);
		}
		else
		{
			detail::scoped_value<detail::parameter::read_framebuffer> binding(native_handle());

			GLWRAP_GL_CALL(glFramebufferParameteri)(GL_READ_FRAMEBUFFER, _pname, _val);
		}
	}
};

// TODO: this should take a color attachment enumerator and bind attachments to fragdatas
class framebuffer_builder
{
public:
	framebuffer_builder(context&)
	{}

	// TODO: allow constants: GL_BACK and such
	template <typename T>
	void bind_draw_buffer(const fragdata_location<T>& _loc, color_attachment const& _attachment)
	{
		uint_t const index = _loc.get_index();

		if (index >= m_draw_buffers.size())
			m_draw_buffers.resize(index + 1, color_attachment(GL_NONE));

		m_draw_buffers[index] = _attachment;
	}

	// TODO: needed?
	template <typename T>
	void unbind_draw_buffer(const fragdata_location<T>& _loc)
	{
		uint_t const index = _loc.get_index();

		if (index < m_draw_buffers.size())
			m_draw_buffers[index] = color_attachment(GL_NONE);

		// TODO: shrink vector
	}

	void bind_read_buffer(color_attachment const& _attachment)
	{
		m_read_buffer = _attachment;
	}

	// TODO: needed?
	void unbind_read_buffer(color_attachment const& _attachment)
	{
		m_read_buffer = color_attachment(GL_NONE);
	}

	framebuffer create_framebuffer(context& _glc) const
	{
		framebuffer result(_glc);

		result.bind_draw_buffers(m_draw_buffers);
		result.bind_read_buffer(m_read_buffer);
		
		return result;
	}

private:
	std::vector<color_attachment> m_draw_buffers;
	color_attachment m_read_buffer;
};

// TODO: generalize this for any zero-able binding
class framebuffer_reference
{
public:
	// TODO: nullptr is kinda ugly
	framebuffer_reference(std::nullptr_t)
		: m_fb{}
	{}

	framebuffer_reference(const framebuffer& _fb)
		: m_fb{_fb.native_handle()}
	{}

	uint_t native_handle() const
	{
		return m_fb;
	}

private:
	const uint_t m_fb;
};

}
