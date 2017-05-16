
#pragma once

#include "variable.hpp"
#include "texture.hpp"
#include "renderbuffer.hpp"

namespace gl
{

// TODO: rename this to drawbuffer ?
// TODO: does it make sense for this to have a type
template <typename T>
class fragdata_location
{
public:
	fragdata_location(int_t _index)
		: m_index(_index)
	{}


	int_t get_index() const
	{
		return m_index;
	}

	int_t m_index;
};

// TODO: rename this
class fragdata_location_enumerator
{
public:
	template <typename T>
	using location_type = fragdata_location<T>;
	
	// TODO: really need context?
	fragdata_location_enumerator(context& _context)
		: m_current_index()
		, m_max_locations()
	{
		detail::gl_get(GL_MAX_DRAW_BUFFERS, &m_max_locations);
	}

	template <typename T>
	fragdata_location<T> get()
	{
		if (m_current_index >= m_max_locations)
			throw exception(0);
			
		fragdata_location<T> ind(m_current_index);

		m_current_index += 1;

		return ind;
	}

private:
	int_t m_current_index;
	int_t m_max_locations;
};

class program;

struct attach_point
{
	friend class context;

public:
	GLenum get_value() const
	{
		return m_attachment;
	}

protected:
	attach_point(GLenum _attachment)
		: m_attachment(_attachment)
	{}

	GLenum m_attachment;
};

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

class framebuffer : public globject
{
	friend class context;

	framebuffer(void*)
		: globject()
	{}

public:
	framebuffer(framebuffer&&) = default;
	framebuffer& operator=(framebuffer&&) = default;

	framebuffer(context& _context)
		: globject(detail::gen_return(glGenFramebuffers))
	{
		// TODO: this is ugly, actually create the object
		detail::scoped_value<detail::parameter::read_framebuffer> binding(native_handle());
	}

	~framebuffer()
	{
		auto const nh = native_handle();
		GLWRAP_EC_CALL(glDeleteFramebuffers)(1, &nh);
	}

	// TODO: support textures and stuff too
	// TODO: support depth and stencil attachments
	void bind_attachment(color_attachment const& _point, renderbuffer const& _attachment)
	{
		if (GL_ARB_direct_state_access)
		{
			GLWRAP_EC_CALL(glNamedFramebufferRenderbuffer)(native_handle(), _point.get_index(), GL_RENDERBUFFER, _attachment.native_handle());
		}
		else
		{
			detail::scoped_value<detail::parameter::read_framebuffer> binding(native_handle());
			
			GLWRAP_EC_CALL(glFramebufferRenderbuffer)(GL_READ_FRAMEBUFFER, _point.get_index(), GL_RENDERBUFFER, _attachment.native_handle());
		}
	}

	// TODO: support depth and stencil attachments
	void unbind_attachment(color_attachment const& _point)
	{
		if (GL_ARB_direct_state_access)
		{
			GLWRAP_EC_CALL(glNamedFramebufferRenderbuffer)(native_handle(), _point.get_index(), GL_RENDERBUFFER, 0);
		}
		else
		{
			detail::scoped_value<detail::parameter::read_framebuffer> binding(native_handle());
			
			GLWRAP_EC_CALL(glFramebufferRenderbuffer)(GL_READ_FRAMEBUFFER, _point.get_index(), GL_RENDERBUFFER, 0);
		}
	}

	// TODO: support depth and stencil attachments
	// TODO: should I even expose this function directly? it doesn't reference the fragdata numbers..
	void bind_draw_buffers(const std::vector<color_attachment>& _attachments)
	{
		std::vector<enum_t> bufs(_attachments.size());
		std::transform(_attachments.begin(), _attachments.end(), bufs.begin(), std::mem_fn(&color_attachment::get_index));
		
		if (GL_ARB_direct_state_access)
		{
			GLWRAP_EC_CALL(glNamedFramebufferDrawBuffers)(native_handle(), bufs.size(), bufs.data());
		}
		else
		{
			detail::scoped_value<detail::parameter::draw_framebuffer> binding(native_handle());
			
			GLWRAP_EC_CALL(glDrawBuffers)(bufs.size(), bufs.data());
		}
	}

	// TODO: support depth and stencil attachments
	void bind_read_buffer(color_attachment const& _attachment)
	{
		if (GL_ARB_direct_state_access)
		{
			GLWRAP_EC_CALL(glNamedFramebufferReadBuffer)(native_handle(), _attachment.get_index());
		}
		else
		{
			detail::scoped_value<detail::parameter::read_framebuffer> binding(native_handle());
			
			GLWRAP_EC_CALL(glReadBuffer)(_attachment.get_index());
		}
	}

	// TODO: other statuses and read status too
	bool is_draw_complete()
	{
		enum_t status = GL_FRAMEBUFFER_UNDEFINED;
		
		if (GL_ARB_direct_state_access)
		{
			status = GLWRAP_EC_CALL(glCheckNamedFramebufferStatus)(native_handle(), GL_DRAW_FRAMEBUFFER);
		}
		else
		{
			detail::scoped_value<detail::parameter::draw_framebuffer> binding(native_handle());

			status = GLWRAP_EC_CALL(glCheckFramebufferStatus)(GL_DRAW_FRAMEBUFFER);
		}

		return (GL_FRAMEBUFFER_COMPLETE == status);
	}

private:
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
		
		return std::move(result);
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
	framebuffer_reference(nullptr_t)
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
