
#pragma once

#include "native_handle.hpp"
#include "attribute.hpp"
#include "buffer.hpp"
#include "util.hpp"
#include "detail/attribute.hpp"

namespace gl
{

class context;

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

	template <typename T, typename B>
	typename std::enable_if<is_buffer_iterator<T, B>::value, void>::type
	bind_vertex_attribute(const attribute_location<T>& _location, const B& _comp)
	{
		auto const index = _location.get_index();

		glBindBuffer(GL_ARRAY_BUFFER, _comp.m_buffer);

		// TODO: make not needed
		detail::scoped_value<detail::parameter::vertex_array> binding(native_handle());
		
		glEnableVertexAttribArray(index);
		detail::vertex_attrib_pointer<T>::bind(index, _comp.stride(), _comp.m_offset);
	}

	template <typename T>
	void unbind_vertex_attribute(const attribute_location<T>& _location)
	{
		// TODO: make not needed
		detail::scoped_value<detail::parameter::vertex_array> binding(native_handle());
		
		glDisableVertexAttribArray(_location.get_index());
	}
};

}
