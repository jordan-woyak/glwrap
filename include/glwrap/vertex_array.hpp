
#pragma once

#include "native_handle.hpp"
#include "attribute.hpp"
#include "buffer.hpp"
#include "detail/attribute.hpp"

namespace gl
{

class context;

class vertex_array : public globject
{
	friend class context;

public:
	template <typename T>
	class indexed_iterator;

	class iterator
	{
		friend class vertex_array;
		friend class context;

	public:
		template <typename T>
		indexed_iterator<T> operator/(buffer_iterator<T> const& _indices)
		{
			static_assert(std::is_same<T, ubyte_t>::value
				|| std::is_same<T, ushort_t>::value
				|| std::is_same<T, uint_t>::value
				, "must be ubyte, uint, ushort");

			return {*this, _indices.m_buffer, 0};
		}

	private:
		iterator(GLuint _vao, GLuint _offset)
			: m_vao(_vao), m_offset(_offset)
		{}

		GLuint m_vao;
		GLuint m_offset;
	};

	template <typename T>
	class indexed_iterator
	{
		friend class iterator;
		friend class context;

	private:
		indexed_iterator(iterator const& _verts, GLuint _buffer, GLuint _offset)
			: m_verts(_verts), m_buffer(_buffer), m_offset(_offset)
		{}

		iterator m_verts;
		GLuint m_buffer;
		GLuint m_offset;
	};

	explicit vertex_array(context& _context)
		: globject(gen_return(glGenVertexArrays))
	{}

	~vertex_array()
	{
		auto const nh = native_handle();
		glDeleteVertexArrays(1, &nh);
	}

	iterator begin() const
	{
		return {native_handle(), 0};
	}

	template <typename T, typename B>
	typename std::enable_if<is_buffer_iterator<T, B>::value, void>::type
	bind_vertex_attribute(const attribute_location<T>& _location, const B& _comp)
	{
		auto const index = _location.get_index();

		bind();
		glEnableVertexAttribArray(index);

		glBindBuffer(GL_ARRAY_BUFFER, _comp.m_buffer);
		detail::vertex_attrib_pointer<T>::bind(index, _comp.stride(), _comp.m_offset);
	}

	template <typename T>
	void unbind_vertex_attribute(const attribute_location<T>& _location)
	{
		bind();
		glDisableVertexAttribArray(_location.get_index());
	}

private:
	void bind()
	{
		glBindVertexArray(native_handle());
	}
};

}
