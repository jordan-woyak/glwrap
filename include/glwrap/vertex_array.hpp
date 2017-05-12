
#pragma once

#include "native_handle.hpp"
#include "attribute.hpp"
#include "buffer.hpp"
#include "util.hpp"
#include "detail/attribute.hpp"

namespace gl
{

class context;

// TODO: name?
template <typename T>
class vertex_buffer_binding
{
	friend class vertex_buffer_binding_enumerator;

public:
	int_t get_index() const
	{
		return m_index;
	}

private:
	vertex_buffer_binding(int_t _index)
		: m_index(_index)
	{}

	int_t m_index;
};

// TODO: name?
class vertex_buffer_binding_enumerator
{
public:
	// TODO: really need context?
	vertex_buffer_binding_enumerator(context& _context)
		: m_current_index()
	{
		detail::gl_get(GL_MAX_VERTEX_ATTRIB_BINDINGS, &m_max_vertex_attrib_bindings);
	}

	template <typename T>
	vertex_buffer_binding<T> get()
	{
		if (m_current_index == m_max_vertex_attrib_bindings)
			throw exception();

		return {m_current_index++};
	}

private:
	int_t m_current_index;
	int_t m_max_vertex_attrib_bindings;
};

template <typename T>
class vertex_buffer_binding_attribute
{
public:
	vertex_buffer_binding_attribute(vertex_buffer_binding<T> const& _binding)
		: m_index(_binding.get_index())
		, m_offset()
	{}	
	
	vertex_buffer_binding_attribute(int_t _index, uint_t _offset)
		: m_index(_index)
		, m_offset(_offset)
	{}

	int_t get_index() const
	{
		return m_index;
	}

	int_t get_offset() const
	{
		return m_offset;
	}

private:
	int_t m_index;
	uint_t m_offset;
};

template <typename T, typename M>
vertex_buffer_binding_attribute<M> operator|(vertex_buffer_binding<T> const& _binding, M T::*_member)
{
	// TODO: only do this for types that need aligning:
	//static_assert((detail::get_member_offset(_member) % sizeof(M)) == 0, "Member is not aligned.");
	
	return vertex_buffer_binding_attribute<M>(_binding.get_index(), detail::get_member_offset(_member));
}

// TODO: make this not needed
template <typename T, typename M>
vertex_buffer_binding_attribute<M> operator|(vertex_buffer_binding_attribute<T> const& _attr, M T::*_member)
{
	// TODO: only do this for types that need aligning:
	//static_assert((detail::get_member_offset(_member) % sizeof(M)) == 0, "Member is not aligned.");
	
	return vertex_buffer_binding_attribute<M>(_attr.get_index(), _attr.get_offset() + detail::get_member_offset(_member));
}

class vertex_array : public globject
{
public:
	explicit vertex_array(context& _context)
		: globject(gen_return(glGenVertexArrays))
	{}

	~vertex_array()
	{
		auto const nh = native_handle();
		glDeleteVertexArrays(1, &nh);
	}

/*
	template <typename T, typename A>
	void bind_vertex_attribute(const attribute_location<T>& _location, const buffer_iterator<T, A>& _iter)
	{
		auto const index = _location.get_index();

		// TODO: make temporary?
		glBindBuffer(GL_ARRAY_BUFFER, _iter.get_buffer());

		// TODO: make not needed
		detail::scoped_value<detail::parameter::vertex_array> binding(native_handle());

		// TODO: move out
		glEnableVertexAttribArray(index);
		
		detail::vertex_attrib_pointer<T>::bind(index, _iter.get_stride(), _iter.get_offset());
	}
*/

	// TODO: allow type conversion
	template <typename T>
	void bind_vertex_attribute(const attribute_location<T>& _attrib, const vertex_buffer_binding_attribute<T>& _binding)
	{
		if (GL_ARB_direct_state_access)
		{
			glVertexArrayAttribBinding(native_handle(), _attrib.get_index(), _binding.get_index());
			detail::gl_vertex_array_attrib_format<T, T, false>(native_handle(), _attrib.get_index(), _binding.get_offset());
		}
		else
		{
			detail::scoped_value<detail::parameter::vertex_array> binding(native_handle());

			glVertexAttribBinding(_attrib.get_index(), _binding.get_index());
			detail::gl_vertex_attrib_format<T, T, false>(_attrib.get_index(), _binding.get_offset());
		}
	}

	template <typename T, typename A>
	void bind_vertex_buffer(const vertex_buffer_binding<T>& _binding, const buffer_iterator<T, A>& _iter)
	{
		if (GL_ARB_direct_state_access)
		{
			glVertexArrayVertexBuffer(native_handle(), _binding.get_index(),
				_iter.get_buffer(), _iter.get_offset() - (ubyte_t*)0, _iter.get_stride());
		}
		else
		{
			detail::scoped_value<detail::parameter::vertex_array> binding(native_handle());
			
			glBindVertexBuffer(_binding.get_index(),
				_iter.get_buffer(), _iter.get_offset() - (ubyte_t*)0, _iter.get_stride());
		}
	}

	template <typename T>
	void enable_vertex_attribute(const attribute_location<T>& _location)
	{
		if (GL_ARB_direct_state_access)
		{
			glEnableVertexArrayAttrib(native_handle(), _location.get_index());
		}
		else
		{
			detail::scoped_value<detail::parameter::vertex_array> binding(native_handle());
		
			glEnableVertexAttribArray(_location.get_index());
		}
	}

	template <typename T>
	void disable_vertex_attribute(const attribute_location<T>& _location)
	{
		if (GL_ARB_direct_state_access)
		{
			glDisableVertexArrayAttrib(native_handle(), _location.get_index());
		}
		else
		{
			detail::scoped_value<detail::parameter::vertex_array> binding(native_handle());
		
			glDisableVertexAttribArray(_location.get_index());
		}
	}
};

}
