#pragma once

#include "uniform_block.hpp"

#include "detail/context.hpp"

namespace gl
{

class vertex_array;

namespace detail
{

// TODO: move these elsewhere?
template <typename T>
struct tight_buffer_alignment
{
	static std::size_t get_alignment()
	{
		return sizeof(T);
	}
};

template <typename T>
struct uniform_buffer_alignment
{
	static std::size_t get_alignment()
	{
		auto const element_size = sizeof(T);
		auto const ubo_alignment = detail::get_parameter<int_t>(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT);

		return ((element_size + ubo_alignment - 1) / ubo_alignment) * ubo_alignment;
	}
};

// TODO: make a version of this where the alignment is known at compile-time ?
template <typename T>
struct strided_buffer_alignment
{
	strided_buffer_alignment(std::size_t _alignment)
		: m_alignment(_alignment)
	{}
	
	std::size_t get_alignment()
	{
		return m_alignment;
	}

private:
	std::size_t m_alignment;
};

}

template <typename T, template <typename> typename A>
class buffer_iterator;

template <typename T>
using tight_buffer_iterator = buffer_iterator<T, detail::tight_buffer_alignment>;

template <typename T>
using uniform_buffer_iterator = buffer_iterator<T, detail::uniform_buffer_alignment>;

template <typename T>
using strided_buffer_iterator = buffer_iterator<T, detail::strided_buffer_alignment>;

template <typename T, template <typename> typename A>
class buffer;

template <typename T, template <typename> typename A>
class buffer_iterator
{
	friend class context;

	template <typename, template <typename> typename>
	friend class buffer;
	
	friend class vertex_array;

public:
	typedef T value_type;
	typedef A<T> alignment_type;

	buffer_iterator& operator+=(std::size_t _offset)
	{
		m_offset += m_stride * _offset;
		return *this;
	}

	template <typename M>
	friend strided_buffer_iterator<M> operator|(buffer_iterator const& _iter, M value_type::*_member)
	{
		auto const member_offset = detail::get_member_offset(_member);
		return
			strided_buffer_iterator<M>{_iter.m_buffer, _iter.m_offset + member_offset, _iter.m_stride};
	}

	friend buffer_iterator operator+(const buffer_iterator& _lhs, std::size_t _offset)
	{
		return _lhs += _offset;
	}

	// TODO: this should be private
	buffer_iterator(uint_t _buffer, ubyte_t* _offset, sizei_t _stride)
		: m_offset(_offset)
		, m_stride(_stride)
		, m_buffer(_buffer)
	{}

private:
	ubyte_t* m_offset;
	// TODO: store the alignment type instead of this?
	sizei_t m_stride;
	uint_t m_buffer;
};

template <typename T>
class mapped_buffer;

template <typename T, template <typename> typename A = detail::tight_buffer_alignment>
class buffer : public globject
{
	friend class context;

public:
	typedef T value_type;
	typedef A<T> alignment_type;

	typedef buffer_iterator<T, A> iterator;

	friend class mapped_buffer<value_type>;

	explicit buffer(context& _context)
		: globject(gen_return(glGenBuffers))
		, m_alignment()
	{}

	~buffer()
	{
		auto const nh = native_handle();
		glDeleteBuffers(1, &nh);
	}

	// TODO: don't hardcode GL_STATIC_DRAW everywhere

	void storage(std::size_t _size)
	{
		glBindBuffer(GL_COPY_WRITE_BUFFER, native_handle());
		glBufferData(GL_COPY_WRITE_BUFFER, _size * m_alignment.get_alignment(), nullptr, GL_STATIC_DRAW);
	}

	std::size_t size() const
	{
		sizei_t sz = 0;
		glBindBuffer(GL_COPY_WRITE_BUFFER, native_handle());
		glGetBufferParameteriv(GL_COPY_WRITE_BUFFER, GL_BUFFER_SIZE, &sz);

		return sz / m_alignment.get_alignment();
	}

	iterator begin()
	{
		return iterator(native_handle(), 0, m_alignment.get_alignment());
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

		static_assert(detail::is_same_ignore_reference_cv<decltype(*begin), value_type>::value,
			"range must contain value_type");

		glBindBuffer(GL_COPY_WRITE_BUFFER, native_handle());
		//glBufferData(GL_COPY_WRITE_BUFFER, size * m_element_size, &*begin, GL_STATIC_DRAW);

		// TODO: unoptimized to work with uniform_block_align<>
		glBufferData(GL_COPY_WRITE_BUFFER, size * m_alignment.get_alignment(), nullptr, GL_STATIC_DRAW);
		mapped_buffer<value_type> mbuf(*this);
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

private:
	// TODO: store size?

	// TODO: do I like this?
	alignment_type m_alignment;
};

// TODO: fix for uniform_block_align<> :/
template <typename T>
class mapped_buffer
{
public:
	template <typename B>
	mapped_buffer(buffer<B>& _buffer)
		: m_ptr()
		, m_size()
		, m_buffer(_buffer.native_handle())
		, m_stride(_buffer.m_alignment.get_alignment())
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

		friend iterator operator+(iterator _lhs, std::size_t _offset)
		{
			return _lhs += _offset;
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

}
