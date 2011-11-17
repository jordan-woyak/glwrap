#pragma once

#include "native_handle.hpp"
#include "vector.hpp"
#include "util.hpp"
#include "detail/attribute.hpp"

#include <boost/format.hpp>

namespace gl
{

class context;

//template <typename T>
//class attribute;

template <typename T>
class array_buffer_component
{
	//friend class attribute<T>;

public:
	array_buffer_component(GLsizei _stride, const GLvoid* _offset, GLuint _buffer)
		: m_stride(_stride)
		, m_offset(_offset)
		, m_buffer(_buffer)
	{}

	void bind_to_attrib(uint_t _index) const
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
		detail::vertex_attrib_pointer<T>::bind(_index, m_stride, m_offset);
	}

private:
	typedef T value_type;

	GLsizei m_stride;
	const GLvoid* m_offset;
	GLuint m_buffer;
};

template <typename T>
class array_buffer : public globject
{
	friend class context;

public:
	typedef T element_type;

	~array_buffer()
	{
		auto const nh = native_handle();
		glDeleteBuffers(1, &nh);
	}

//	void resize(std::size_t _size)
//	{
//		glBufferData(GL_buffer, _size * sizeof(element_type), NULL, GL_STATIC_DRAW);
//	}

	template <typename C>
	array_buffer_component<C> get_component(C element_type::*_component)
	{
		const T* const null_obj = nullptr;

		GLsizei const stride = sizeof(T);
		const GLvoid* offset = &(null_obj->*_component);

		return array_buffer_component<C>(stride, offset, native_handle());
	}

	// TODO: only sane for funtypes
	array_buffer_component<T> get_component()
	{
		return array_buffer_component<T>(0, 0, native_handle());
	}

	template <typename R>
	void assign(R&& _range)
	{
		//auto& contig_range = detail::get_contiguous_range<element_type>(std::forward<R>(_range));
		auto& contig_range = _range;

		auto const begin = std::begin(contig_range);
		auto const size = std::distance(begin, std::end(contig_range));

		static_assert(detail::is_contiguous<R>::value,
			"range must be contiguous");

		static_assert(detail::is_same_ignore_reference_cv<decltype(*begin), element_type>::value,
			"range must contain element_type");

		glBindBuffer(GL_COPY_WRITE_BUFFER, native_handle());
		glBufferData(GL_COPY_WRITE_BUFFER, size * sizeof(element_type), &*begin, GL_STATIC_DRAW);
	}

	void assign(array_buffer const& _other)
	{
		glBindBuffer(GL_COPY_READ_BUFFER, _other.native_handle());
		glBindBuffer(GL_COPY_WRITE_BUFFER, native_handle());

		sizei_t size = 0;
		glGetBufferParameteriv(GL_COPY_READ_BUFFER, GL_BUFFER_SIZE, &size);

		glBufferData(GL_COPY_WRITE_BUFFER, size, nullptr, GL_STATIC_DRAW);
		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, size);
	}

	explicit array_buffer(context& _context)
		: globject(gen_return(glGenBuffers))
	{}
};

}

