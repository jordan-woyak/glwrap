#pragma once

#include "uniform_block.hpp"

namespace gl
{

template <typename T>
class buffer;

template <typename T>
class buffer_iterator;

template <typename T>
class strided_buffer_iterator;

class vertex_array;

template <typename T, typename B, typename Enable = void>
struct is_buffer_iterator : std::false_type {};

template <typename T, typename B>
struct is_buffer_iterator<T, B, typename std::enable_if<
	std::is_same<buffer_iterator<T>, B>::value ||
	std::is_same<strided_buffer_iterator<T>, B>::value
	>::type> : std::true_type {};

namespace detail
{

// silly silly stuff
template <typename T>
struct actual_buffer_type
{
	typedef T type;
};

template <>
template <typename T>
struct actual_buffer_type<gl::uniform_block_align<T>>
{
	typedef T type;
};

// silly silly stuff
template <typename T>
struct actual_element_size
{
	static std::size_t value()
	{
		return sizeof(T);
	}
};

template <>
template <typename T>
struct actual_element_size<gl::uniform_block_align<T>>
{
	static std::size_t value()
	{
		// TODO: fails if T is bigger than alignment value!

		GLint offset{};
		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &offset);
		return offset;
	}
};

}

// TODO: rename iterator
template <typename T>
class buffer_iterator
{
	friend class context;

	friend class buffer<T>;
	// silly
	friend class buffer<gl::uniform_block_align<T>>;

	friend class vertex_array;

	template <typename P, typename M>
	friend strided_buffer_iterator<M> operator|(buffer_iterator<P> const& _attrib, M P::*_member);

public:
	buffer_iterator& operator+=(std::size_t _offset)
	{
		m_offset = static_cast<char*>(m_offset) + stride() * _offset;
		return *this;
	}

private:
	buffer_iterator(GLuint _buffer, GLvoid* _offset, GLsizei _stride)
		: m_buffer(_buffer)
		, m_offset(_offset)
		, m_stride(_stride)
	{}

	GLsizei stride() const
	{
		return m_stride;
	}

	GLuint m_buffer;
	GLvoid* m_offset;
	GLsizei m_stride;
};

template <typename T>
class strided_buffer_iterator
{
	friend class vertex_array;

	template <typename P, typename M>
	friend strided_buffer_iterator<M> operator|(buffer_iterator<P> const& _attrib, M P::*_member);

public:
	strided_buffer_iterator& operator+=(std::size_t _offset)
	{
		m_offset = static_cast<char*>(m_offset) + stride() * _offset;
		return *this;
	}

private:
	strided_buffer_iterator(GLuint _buffer, GLvoid* _offset, GLsizei _stride)
		: m_buffer(_buffer)
		, m_offset(_offset)
		, m_stride(_stride)
	{}

	GLsizei stride() const
	{
		return m_stride;
	}

	GLuint m_buffer;
	GLvoid* m_offset;
	GLsizei m_stride;
};

template <typename P, typename M>
strided_buffer_iterator<M> operator|(buffer_iterator<P> const& _attrib, M P::*_member)
{
	auto const offset = detail::get_member_offset(_member);
	return {_attrib.m_buffer, static_cast<char*>(_attrib.m_offset) + offset, _attrib.stride()};
}

template <typename T>
strided_buffer_iterator<T> operator+(strided_buffer_iterator<T> _lhs, std::size_t _offset)
{
	return _lhs += _offset;
}

template <typename T>
buffer_iterator<T> operator+(buffer_iterator<T> _lhs, std::size_t _offset)
{
	return _lhs += _offset;
}

// TODO: fix for uniform_block_align<> :/
template <typename T>
class mapped_buffer
{
public:
	template <typename B, typename Enable =
		typename std::enable_if<std::is_same<T, typename detail::actual_buffer_type<B>::type>::value>::type>
	mapped_buffer(buffer<B>& _buffer)
		: m_ptr()
		, m_size()
		, m_buffer(_buffer.native_handle())
		, m_stride(_buffer.m_element_size)
	{
		glBindBuffer(GL_COPY_WRITE_BUFFER, m_buffer);

		// TODO: allow range
		// TODO: don't hardcode access mode
		m_ptr = static_cast<T*>(glMapBuffer(GL_COPY_WRITE_BUFFER, GL_READ_WRITE));
		m_size = _buffer.size();

		// TODO: check for error

		// TODO: use this guy
		//glMapBufferRange(GL_COPY_WRITE_BUFFER, 0, length, access);
	}

	mapped_buffer(mapped_buffer const&) = delete;
	mapped_buffer& operator=(mapped_buffer const&) = delete;

	// TODO: movable

	// TODO: flush function

	class iterator : public std::iterator<std::random_access_iterator_tag, T>
	{
		friend class mapped_buffer;

	public:
		// TODO: silly
		typedef T value_type;

		iterator(iterator const& _rhs) = default;
		iterator& operator=(iterator const& _rhs) = default;

		iterator& operator++()
		{
			return *this += 1;
		}

		iterator& operator+=(std::size_t _offset)
		{
			m_ptr = reinterpret_cast<T*>(reinterpret_cast<char*>(m_ptr) + _offset * m_stride);
			return *this;
		}

		value_type& operator*() const
		{
			return *m_ptr;
		}

		value_type* operator->() const
		{
			return m_ptr;
		}

		bool operator==(iterator const& _rhs) const
		{
			return m_ptr == _rhs.m_ptr;
		}

		bool operator!=(iterator const& _rhs) const
		{
			return !(*this == _rhs);
		}

	private:
		iterator(T* _ptr, std::size_t _stride)
			: m_ptr(_ptr)
			, m_stride(_stride)
		{}

		T* m_ptr;
		std::size_t m_stride;
	};

	iterator begin()
	{
		return {m_ptr, m_stride};
	}

	iterator end()
	{
		return {reinterpret_cast<T*>(reinterpret_cast<char*>(m_ptr) + m_size * m_stride), m_stride};
	}

	~mapped_buffer()
	{
		glBindBuffer(GL_COPY_WRITE_BUFFER, m_buffer);
		glUnmapBuffer(GL_COPY_WRITE_BUFFER);
	}

private:
	T* m_ptr;
	std::size_t m_size;
	GLuint m_buffer;
	std::size_t m_stride;
};

template <typename T>
class buffer : public globject
{
	friend class context;

public:
	typedef typename detail::actual_buffer_type<T>::type element_type;

	friend class mapped_buffer<element_type>;

	~buffer()
	{
		auto const nh = native_handle();
		glDeleteBuffers(1, &nh);
	}

	// TODO: don't hardcode GL_STATIC_DRAW everywhere

	void storage(std::size_t _size)
	{
		glBindBuffer(GL_COPY_WRITE_BUFFER, native_handle());
		glBufferData(GL_COPY_WRITE_BUFFER, _size * m_element_size, nullptr, GL_STATIC_DRAW);
	}

	std::size_t size() const
	{
		sizei_t sz = 0;
		glBindBuffer(GL_COPY_WRITE_BUFFER, native_handle());
		glGetBufferParameteriv(GL_COPY_WRITE_BUFFER, GL_BUFFER_SIZE, &sz);

		return sz / m_element_size;
	}

	// TODO: read capability

	buffer_iterator<element_type> begin()
	{
		return buffer_iterator<element_type>(native_handle(), 0, m_element_size);
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
		//glBufferData(GL_COPY_WRITE_BUFFER, size * m_element_size, &*begin, GL_STATIC_DRAW);

		// TODO: unoptimized to work with uniform_block_align<>
		glBufferData(GL_COPY_WRITE_BUFFER, size * m_element_size, nullptr, GL_STATIC_DRAW);
		mapped_buffer<element_type> mbuf(*this);
		std::copy_n(begin, size, mbuf.begin());
	}

	void assign(buffer const& _other)
	{
		glBindBuffer(GL_COPY_READ_BUFFER, _other.native_handle());
		glBindBuffer(GL_COPY_WRITE_BUFFER, native_handle());

		sizei_t sz = 0;
		glGetBufferParameteriv(GL_COPY_READ_BUFFER, GL_BUFFER_SIZE, &sz);

		glBufferData(GL_COPY_WRITE_BUFFER, size, nullptr, GL_STATIC_DRAW);
		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, sz);
	}

	explicit buffer(context& _context)
		: globject(gen_return(glGenBuffers))
		, m_element_size(detail::actual_element_size<T>::value())
	{}

private:
	std::size_t m_element_size;
};

}
