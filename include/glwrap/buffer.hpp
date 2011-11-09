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
class buffer_component
{
	//friend class attribute<T>;

public:
	buffer_component(GLsizei _stride, const GLvoid* _offset, GLuint _buffer)
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
void buffer_component<float_t>::bind(uint_t _index) const
{
	glVertexAttribPointer(_index, 1, GL_FLOAT, GL_FALSE, m_stride, m_offset);
}

template <>
void buffer_component<fvec2>::bind(uint_t _index) const
{
	glVertexAttribPointer(_index, 2, GL_FLOAT, GL_FALSE, m_stride, m_offset);
}

template <>
void buffer_component<fvec3>::bind(uint_t _index) const
{
	glVertexAttribPointer(_index, 3, GL_FLOAT, GL_FALSE, m_stride, m_offset);
}

template <>
void buffer_component<fvec4>::bind(uint_t _index) const
{
	glVertexAttribPointer(_index, 4, GL_FLOAT, GL_FALSE, m_stride, m_offset);
}

template <typename T>
class buffer : public native_handle_base<GLuint>
{
	friend class context;

public:
	typedef T element_type;

	buffer(const buffer&) = delete;
	buffer& operator=(const buffer&) = delete;

	buffer(buffer&&) = default;
	buffer& operator=(buffer&&) = default;

	~buffer()
	{
		auto const nh = native_handle();
		glDeleteBuffers(1, &nh);
	}

//	void resize(std::size_t _size)
//	{
//		glBufferData(GL_buffer, _size * sizeof(element_type), NULL, GL_STATIC_DRAW);
//	}

	template <typename C>
	buffer_component<C> get_component(C element_type::*_component)
	{
		const T* const null_obj = nullptr;

		GLsizei const stride = sizeof(T);
		const GLvoid* offset = &(null_obj->*_component);

		return buffer_component<C>(stride, offset, native_handle());
	}

	// TODO: only sane for funtypes
	buffer_component<T> get_component()
	{
		return buffer_component<T>(0, 0, native_handle());
	}

	void assign(std::vector<T> const& _range)
	{
		glBindBuffer(GL_COPY_WRITE_BUFFER, native_handle());
		glBufferData(GL_COPY_WRITE_BUFFER, _range.size() * sizeof(element_type), _range.data(), GL_STATIC_DRAW);
	}

	template <std::size_t S>
	void assign(std::array<T, S> const& _range)
	{
		glBindBuffer(GL_COPY_WRITE_BUFFER, native_handle());
		glBufferData(GL_COPY_WRITE_BUFFER, _range.size() * sizeof(element_type), _range.data(), GL_STATIC_DRAW);
	}

	template <typename R>
	void assign(R const& _range)
	{
		std::vector<T> vec(_range.begin(), _range.end());
		assign(vec);
	}

	void assign(buffer const& _other)
	{
		glBindBuffer(GL_COPY_READ_BUFFER, _other.native_handle());
		glBindBuffer(GL_COPY_WRITE_BUFFER, native_handle());

		sizei_t size = 0;
		glGetBufferParameteriv(GL_COPY_READ_BUFFER, GL_BUFFER_SIZE, &size);

		glBufferData(GL_COPY_WRITE_BUFFER, size, nullptr, GL_STATIC_DRAW);
		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, size);
	}

	explicit buffer(context& _context)
		: native_handle_base<GLuint>(gen_return(glGenBuffers))
	{}
};

}

