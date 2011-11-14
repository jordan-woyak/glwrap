
#pragma once

#include "native_handle.hpp"
#include "attribute.hpp"

namespace gl
{

class context;

class vertex_array : public native_handle_base<GLuint>
{
public:
	explicit vertex_array(context& _context)
		: native_handle_base<GLuint>(gen_return(glGenVertexArrays))
	{}

	~vertex_array()
	{
		auto const nh = native_handle();
		glDeleteVertexArrays(1, &nh);
	}

	template <typename T>
	void bind_vertex_attribute(const attribute_location<T>& _location, const array_buffer_component<T>& _comp)
	{
		auto const index = _location.get_index();

		bind();
		glEnableVertexAttribArray(index);
		_comp.bind_to_attrib(index);
	}

	template <typename T>
	void disable_vertex_attribute(const attribute_location<T>& _location)
	{
		bind();
		glDisableVertexAttribArray(_location.get_index());
	}

//private:
	void bind()
	{
		glBindVertexArray(native_handle());
	}
};

}
