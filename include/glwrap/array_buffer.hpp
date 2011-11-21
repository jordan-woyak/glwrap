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
class vertex_attribute
{
	//friend class attribute<T>;

public:
	vertex_attribute(GLsizei _stride, const GLvoid* _offset, GLuint _buffer)
		: m_stride(_stride)
		, m_offset(_offset)
		, m_buffer(_buffer)
	{}

	// TODO: ugg
	void bind_to_attrib(uint_t _index) const
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
		detail::vertex_attrib_pointer<T>::bind(_index, m_stride, m_offset);
	}

	const GLvoid* get_offset() const
	{
		return m_offset;
	}

	GLsizei get_stride() const
	{
		return m_stride;
	}

	GLuint get_buffer() const
	{
		return m_buffer;
	}

private:
	typedef T value_type;

	GLsizei m_stride;
	const GLvoid* m_offset;
	GLuint m_buffer;
};

template <typename B, typename M>
vertex_attribute<M> operator|(vertex_attribute<B> const& _attrib, M B::*_member)
{
	const B* const null_obj = nullptr;
	auto const offset = reinterpret_cast<std::intptr_t>(&(null_obj->*_member));

	return {_attrib.get_stride(), (char*)_attrib.get_offset() + offset, _attrib.get_buffer()};
}

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

	vertex_attribute<T> attrib()
	{
		return {sizeof(T), 0, native_handle()};
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

