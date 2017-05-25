
#pragma once

#include "native_handle.hpp"
#include "attribute.hpp"
#include "buffer.hpp"
#include "util.hpp"
#include "detail/attribute.hpp"

namespace GLWRAP_NAMESPACE
{

class context;

namespace detail
{

struct vertex_buffer_index
{};

}

// TODO: name? just vertex_binding?
template <typename T>
using vertex_buffer_binding = detail::buffer_index_attribute<detail::vertex_buffer_index, T>;

// TODO: ugly, kill one of these
template <typename T>
using vertex_buffer_binding_attribute = detail::buffer_index_attribute<detail::vertex_buffer_index, T>;

// TODO: name?
class vertex_buffer_binding_enumerator
{
public:
	// TODO: really need context?
	vertex_buffer_binding_enumerator(context& _context)
		: m_current_index()
	{
		// TODO: correct parameter?
		detail::gl_get(GL_MAX_VERTEX_ATTRIB_BINDINGS, &m_max_vertex_attrib_bindings);
	}

	template <typename T>
	vertex_buffer_binding<T> get()
	{
		if (m_current_index == m_max_vertex_attrib_bindings)
			throw exception(0);

		return vertex_buffer_binding<T>(m_current_index++);
	}

private:
	int_t m_current_index;
	int_t m_max_vertex_attrib_bindings;
};

namespace detail
{

struct vertex_array_obj
{
	static void create_objs(sizei_t _n, uint_t* _objs)
	{
		if (GL_ARB_direct_state_access)
		{
			GLWRAP_EC_CALL(glCreateVertexArrays)(_n, _objs);
		}
		else
		{
			GLWRAP_EC_CALL(glGenVertexArrays)(_n, _objs);
		}
	}

	static void delete_objs(sizei_t _n, uint_t* _objs)
	{
		GLWRAP_EC_CALL(glDeleteVertexArrays)(_n, _objs);
	}
};

}

class vertex_array : public detail::globject<detail::vertex_array_obj>
{
public:
	explicit vertex_array(context&)
	{}

/*
	// TODO: kill
	template <typename T, typename A>
	void bind_vertex_pointer(const attribute_location<T>& _attrib, const buffer_iterator<T, A>& _iter)
	{
		// TODO: make temporary?
		GLWRAP_EC_CALL(glBindBuffer)(GL_ARRAY_BUFFER, _iter.get_buffer());

		// TODO: make not needed
		detail::scoped_value<detail::parameter::vertex_array> binding(native_handle());
		
		detail::gl_vertex_attrib_pointer<T, T, false>(_attrib.get_index(), _iter.get_stride(), _iter.get_offset());
	}

	// TODO: kill
	template <typename T>
	void set_attrib_divisor(const attribute_location<T>& _attrib, uint_t _divisor)
	{
		detail::scoped_value<detail::parameter::vertex_array> binding(native_handle());
	
		GLWRAP_EC_CALL(glVertexAttribDivisor)(_attrib.get_index(), _divisor);
	}
*/

	// TODO: separate Bind and Format / rename this
	// TODO: templates are getting ugly
	template <typename ShaderType, typename InputType>
	void set_attribute_format(const attribute_location<ShaderType>& _attrib, const vertex_buffer_binding_attribute<InputType>& _binding)
	{
		// TODO: assert that Shader and Input type conversion is sensible
		//static_assert(std::is_same<ShaderType, InputType>::value, "Currently, attrib types must match exactly..");

		typedef detail::variable_traits<ShaderType> s_traits;
		typedef detail::variable_traits<InputType> i_traits;

		static_assert(s_traits::component_count == i_traits::component_count
			&& s_traits::attrib_index_count == i_traits::attrib_index_count,
			"Component and/or attribute index counts of input/attrib types does not match. Input is not sensible.");
		
		if (GL_ARB_direct_state_access)
		{
			detail::gl_vertex_array_attrib_binding<ShaderType>
				(native_handle(), _attrib.get_index(), _binding.get_index());
			detail::gl_vertex_array_attrib_format<ShaderType, InputType, false>
				(native_handle(), _attrib.get_index(), _binding.get_offset());
			detail::gl_enable_vertex_array_attrib<ShaderType>
				(native_handle(), _attrib.get_index());
		}
		else
		{
			detail::scoped_value<detail::parameter::vertex_array> binding(native_handle());

			detail::gl_vertex_attrib_binding<ShaderType>(_attrib.get_index(), _binding.get_index());
			detail::gl_vertex_attrib_format<ShaderType, InputType, false>(_attrib.get_index(), _binding.get_offset());
			detail::gl_enable_vertex_attrib_array<ShaderType>(_attrib.get_index());
		}
	}

	template <typename T, typename A>
	void set_buffer(const vertex_buffer_binding<T>& _binding, const buffer_iterator<T, A>& _iter)
	{
		if (GL_ARB_direct_state_access)
		{
			GLWRAP_EC_CALL(glVertexArrayVertexBuffer)(native_handle(), _binding.get_index(),
				_iter.get_buffer(), _iter.get_offset() - (ubyte_t*)0, _iter.get_stride());
		}
		else
		{
			detail::scoped_value<detail::parameter::vertex_array> binding(native_handle());
			
			GLWRAP_EC_CALL(glBindVertexBuffer)(_binding.get_index(),
				_iter.get_buffer(), _iter.get_offset() - (ubyte_t*)0, _iter.get_stride());
		}
	}

	// automatically enabled, currently
/*
	template <typename T>
	void disable_attribute(const attribute_location<T>& _attrib)
	{
		if (GL_ARB_direct_state_access)
		{
			detail::gl_enable_vertex_array_attrib<T>(native_handle(), index);
		}
		else
		{
			detail::scoped_value<detail::parameter::vertex_array> binding(native_handle());

			detail::gl_enable_vertex_attrib_array<T>(index);
		}
	}
*/

	template <typename T>
	void disable_attribute(const attribute_location<T>& _attrib)
	{
		if (GL_ARB_direct_state_access)
		{
			detail::gl_disable_vertex_array_attrib<T>(native_handle(), index);
		}
		else
		{
			detail::scoped_value<detail::parameter::vertex_array> binding(native_handle());

			detail::gl_disable_vertex_attrib_array<T>(index);
		}
	}

	template <typename T>
	void set_divisor(const vertex_buffer_binding<T>& _binding, uint_t _divisor)
	{
		if (GL_ARB_direct_state_access)
		{
			GLWRAP_EC_CALL(glVertexArrayBindingDivisor)(native_handle(), _binding.get_index(), _divisor);
		}
		else
		{
			detail::scoped_value<detail::parameter::vertex_array> binding(native_handle());
		
			GLWRAP_EC_CALL(glVertexBindingDivisor)(_binding.get_index(), _divisor);
		}
	}
};

class vertex_array_builder
{
public:
	explicit vertex_array_builder(context&)
	{}

	// TODO: implement
};

}
