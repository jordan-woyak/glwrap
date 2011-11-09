
#pragma once

#include "native_handle.hpp"
#include "context.hpp"

namespace gl
{

class vertex_array : public native_handle_base<GLuint>
{
public:
	vertex_array(vertex_array const&) = delete;
	vertex_array& operator=(vertex_array const&) = delete;

	vertex_array(vertex_array&&) = default;
	vertex_array& operator=(vertex_array&&) = default;

	explicit vertex_array(context& _context)
		: native_handle_base<GLuint>(gen_return(glGenVertexArrays))
	{}

	~vertex_array()
	{
		auto const nh = native_handle();
		glDeleteVertexArrays(1, &nh);
	}

	template <typename T>
	void bind_vertex_attribute(const attribute<T>& _attrib, const buffer_component<T>& _comp)
	{
		auto const index = _attrib.get_location();

		bind();
		glEnableVertexAttribArray(index);
		_comp.bind_to_attrib(index);
	}

	template <typename T>
	void disable_vertex_attribute(const attribute<T>& _attrib)
	{
		bind();
		glDisableVertexAttribArray(_attrib.get_location());
	}

//private:
	void bind()
	{
		glBindVertexArray(native_handle());
	}
};

}
