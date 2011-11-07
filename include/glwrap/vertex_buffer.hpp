#pragma once

#include "native_handle.hpp"
#include "vector.hpp"
#include "util.hpp"

#include <boost/format.hpp>

namespace gl
{

class context;

//template <typename T>
//class attribute;

template <typename T>
class vertex_buffer_component
{
	//friend class attribute<T>;

public:
	vertex_buffer_component(GLsizei _stride, const GLvoid* _offset, GLuint _buffer)
		: m_stride(_stride)
		, m_offset(_offset)
		, m_buffer(_buffer)
	{}

	void bind_to_attrib(uint_t _index) const
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
		bind(_index);
	}

private:
	void bind(uint_t _index) const;

	GLsizei m_stride;
	const GLvoid* m_offset;
	GLuint m_buffer;
};

template <>
void vertex_buffer_component<float_t>::bind(uint_t _index) const
{
	glVertexAttribPointer(_index, 1, GL_FLOAT, GL_FALSE, m_stride, m_offset);
}

template <>
void vertex_buffer_component<fvec2>::bind(uint_t _index) const
{
	glVertexAttribPointer(_index, 2, GL_FLOAT, GL_FALSE, m_stride, m_offset);
}

template <>
void vertex_buffer_component<fvec3>::bind(uint_t _index) const
{
	glVertexAttribPointer(_index, 3, GL_FLOAT, GL_FALSE, m_stride, m_offset);
}

template <>
void vertex_buffer_component<fvec4>::bind(uint_t _index) const
{
	glVertexAttribPointer(_index, 4, GL_FLOAT, GL_FALSE, m_stride, m_offset);
}

template <typename T>
class vertex_buffer : public native_handle_base<GLuint>
{
	friend class context;

public:
	typedef T element_type;

	vertex_buffer(const vertex_buffer&) = delete;
	vertex_buffer& operator=(const vertex_buffer&) = delete;

	vertex_buffer(vertex_buffer&&) = default;
	vertex_buffer& operator=(vertex_buffer&&) = default;

	~vertex_buffer()
	{
		auto const nh = native_handle();
		glDeleteBuffers(1, &nh);
	}

//	void resize(std::size_t _size)
//	{
//		glBufferData(GL_vertex_buffer, _size * sizeof(element_type), NULL, GL_STATIC_DRAW);
//	}

	template <typename C>
	vertex_buffer_component<C> get_component(C element_type::*_component)
	{
		const T* const null_obj = nullptr;

		GLsizei const stride = sizeof(T);
		const GLvoid* offset = &(null_obj->*_component);

		return vertex_buffer_component<C>(stride, offset, native_handle());
	}

	// TODO: only sane for funtypes
	vertex_buffer_component<T> get_component()
	{
		return vertex_buffer_component<T>(0, 0, native_handle());
	}

	// TODO: fails hard for non-array/vector
	template <typename R>
	void assign(R& range)
	{
		auto const begin = std::begin(range), end = std::end(range);
		auto const size = end - begin;

		static_assert(std::is_same<typename std::remove_reference<decltype(*begin)>::type, element_type>::value,
			"range must contain element_type");

		glBindBuffer(GL_COPY_WRITE_BUFFER, native_handle());

		glBufferData(GL_COPY_WRITE_BUFFER, size * sizeof(element_type), &*begin, GL_STATIC_DRAW);
	}

	explicit vertex_buffer(context& _context)
		: native_handle_base<GLuint>(gen_return(glGenBuffers))
	{}
};

}

