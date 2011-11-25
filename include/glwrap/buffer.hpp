#pragma once

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

// TODO: make offset a sane type

// TODO: rename iterator
template <typename T>
class buffer_iterator
{
	friend class context;
	friend class buffer<T>;
	friend class vertex_array;

	template <typename P, typename M>
	friend strided_buffer_iterator<M> operator|(buffer_iterator<P> const& _attrib, M P::*_member);

public:

private:
	buffer_iterator(GLuint _buffer, GLvoid* _offset)
		: m_buffer(_buffer)
		, m_offset(_offset)
	{}

	GLsizei stride() const
	{
		return sizeof(T);
	}

	GLuint m_buffer;
	GLvoid* m_offset;
};

template <typename T>
class strided_buffer_iterator
{
	friend class vertex_array;

	template <typename P, typename M>
	friend strided_buffer_iterator<M> operator|(buffer_iterator<P> const& _attrib, M P::*_member);

public:

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
	const P* const null_obj = nullptr;
	auto const offset = reinterpret_cast<std::intptr_t>(&(null_obj->*_member));

	return {_attrib.m_buffer, static_cast<char*>(_attrib.m_offset) + offset, _attrib.stride()};
}

template <typename T>
class mapped_buffer
{
public:
	mapped_buffer(buffer<T>& _buffer)
		: m_ptr()
		, m_size()
		, m_buffer(_buffer.native_handle())
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

	T* begin()
	{
		return m_ptr;
	}

	T* end()
	{
		return m_ptr + m_size;
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
};

template <typename T>
class buffer : public globject
{
	friend class context;

public:
	typedef T element_type;

	~buffer()
	{
		auto const nh = native_handle();
		glDeleteBuffers(1, &nh);
	}

	// TODO: don't hardcode GL_STATIC_DRAW everywhere

	void storage(std::size_t _size)
	{
		glBindBuffer(GL_COPY_WRITE_BUFFER, native_handle());
		glBufferData(GL_COPY_WRITE_BUFFER, _size * sizeof(element_type), nullptr, GL_STATIC_DRAW);
	}

	std::size_t size() const
	{
		sizei_t sz = 0;
		glBindBuffer(GL_COPY_WRITE_BUFFER, native_handle());
		glGetBufferParameteriv(GL_COPY_WRITE_BUFFER, GL_BUFFER_SIZE, &sz);

		return sz / sizeof(element_type);
	}

	// TODO: rename begin
	buffer_iterator<T> begin()
	{
		return {native_handle(), 0};
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
	{}
};

}
