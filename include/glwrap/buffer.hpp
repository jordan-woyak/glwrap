#pragma once

#include "constants.hpp"
#include "uniform_block.hpp"

#include "detail/context.hpp"

namespace gl
{

class vertex_array;

namespace detail
{

// TODO: move these elsewhere?
struct uniform_buffer_alignment
{
	uniform_buffer_alignment(sizei_t _size)
	{
		// TODO: would be nice to not call this for every buffer.
		auto const element_size = _size;
		auto const ubo_alignment = detail::get_parameter<int_t>(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT);

		m_stride = ((element_size + ubo_alignment - 1) / ubo_alignment) * ubo_alignment;
	}

	sizei_t get_stride() const
	{
		return m_stride;
	}

private:
	sizei_t m_stride;
};

template <sizei_t Stride>
struct static_buffer_alignment
{
	// TODO: somewhat ugly..
	static_buffer_alignment(sizei_t)
	{}
	
	static constexpr sizei_t get_stride()
	{
		return Stride;
	}
};


template <typename T>
using tight_buffer_alignment = static_buffer_alignment<sizeof(T)>;

// TODO: sensible name? needed at all?
struct dynamic_buffer_alignment
{
	dynamic_buffer_alignment(sizei_t _size)
		: m_stride(_size)
	{}
	
	sizei_t get_stride() const
	{
		return m_stride;
	}

private:
	sizei_t m_stride;
};

}

template <typename T, typename A>
class buffer_iterator;

template <typename T>
using uniform_buffer_iterator = buffer_iterator<T, detail::uniform_buffer_alignment>;

template <typename T, sizei_t N>
using static_buffer_iterator = buffer_iterator<T, detail::static_buffer_alignment<N>>;

template <typename T>
using tight_buffer_iterator = buffer_iterator<T, detail::tight_buffer_alignment<T>>;

template <typename T>
using dynamic_buffer_iterator = buffer_iterator<T, detail::dynamic_buffer_alignment>;

template <typename T, typename A>
class buffer;

// TODO: move this into the buffer class?
template <typename T, typename A>
class buffer_iterator
{
	template <typename, typename>
	friend class buffer;

public:
	typedef T value_type;
	typedef A alignment_type;

	buffer_iterator& operator+=(std::size_t _val)
	{
		m_buffer_offset += _val * get_stride();
		return *this;
	}

	friend buffer_iterator operator+(const buffer_iterator& _lhs, std::size_t _offset)
	{
		return _lhs += _offset;
	}

	// TODO: this should be private
	buffer_iterator(uint_t _buffer, ubyte_t* _buffer_offset, const alignment_type& _alignment, uint_t _member_offset = {})
		: m_buffer_offset(_buffer_offset)
		, m_member_offset(_member_offset)
		, m_alignment(_alignment)
		, m_buffer(_buffer)
	{}

	uint_t get_buffer() const
	{
		return m_buffer;
	}
	
	ubyte_t* get_offset() const
	{
		return m_buffer_offset + m_member_offset;
	}

	// TODO: this is a confusing name compared to get_offset
	ubyte_t* get_buffer_offset() const
	{
		return m_buffer_offset;
	}

	uint_t get_member_offset() const
	{
		return m_member_offset;
	}

	sizei_t get_stride() const
	{
		return m_alignment.get_stride();
	}

	const alignment_type& get_alignment() const
	{
		return m_alignment;
	}

private:
	// Offsets are kept separately so I can match iterators
	// from the same buffer area for transform feedback
	ubyte_t* m_buffer_offset;
	uint_t m_member_offset;

	alignment_type m_alignment;
	uint_t m_buffer;
};


template <typename T, typename M, typename A>
buffer_iterator<M, A> operator|(buffer_iterator<T, A> const& _iter, M T::*_member)
{
	auto const member_offset = detail::get_member_offset(_member);
	return
		buffer_iterator<M, A>(_iter.get_buffer(), _iter.get_buffer_offset(),
		_iter.get_alignment(), _iter.get_member_offset() + member_offset);
}

template <typename T, typename A>
class mapped_buffer;

template <typename T, typename A = detail::tight_buffer_alignment<T>>
class buffer : public globject
{
	friend class context;

public:

	// TODO: require T is a POD-type

	typedef T value_type;
	typedef A alignment_type;

	typedef buffer_iterator<T, A> iterator;

	friend class mapped_buffer<T, A>;

	explicit buffer(context& _context)
		: globject(detail::gen_return(glGenBuffers))
		, m_alignment(sizeof(value_type))
	{}

	~buffer()
	{
		auto const nh = native_handle();
		GLWRAP_EC_CALL(glDeleteBuffers)(1, &nh);
	}
	
	void storage(std::size_t _size, buffer_usage _usage)
	{
		// TODO: call glBufferStorage directly if available
		GLWRAP_EC_CALL(glBindBuffer)(GL_COPY_WRITE_BUFFER, native_handle());
		GLWRAP_EC_CALL(glBufferData)(GL_COPY_WRITE_BUFFER, _size * get_stride(), nullptr, static_cast<enum_t>(_usage));
	}

	std::size_t size() const
	{
		sizei_t sz = 0;
		GLWRAP_EC_CALL(glBindBuffer)(GL_COPY_WRITE_BUFFER, native_handle());
		GLWRAP_EC_CALL(glGetBufferParameteriv)(GL_COPY_WRITE_BUFFER, GL_BUFFER_SIZE, &sz);

		return sz / get_stride();
	}

	iterator begin()
	{
		return iterator(native_handle(), 0, m_alignment);
	}

	// TODO: this is broken for untight-alignments
	template <typename R>
	void assign(R&& _range, buffer_usage _usage)
	{
		//auto& contig_range = detail::get_contiguous_range<element_type>(std::forward<R>(_range));
		auto& contig_range = _range;

		auto const begin = std::begin(contig_range);
		auto const size = std::distance(begin, std::end(contig_range));

		static_assert(detail::is_contiguous<R>::value,
			"range must be contiguous");

		static_assert(detail::is_same_ignore_reference_cv<decltype(*begin), value_type>::value,
			"range must contain value_type");

		GLWRAP_EC_CALL(glBindBuffer)(GL_COPY_WRITE_BUFFER, native_handle());
		GLWRAP_EC_CALL(glBufferData)(GL_COPY_WRITE_BUFFER, size * get_stride(), &*begin, static_cast<enum_t>(_usage));
	}
	
	void assign(buffer const& _other, buffer_usage _usage)
	{
		GLWRAP_EC_CALL(glBindBuffer)(GL_COPY_READ_BUFFER, _other.native_handle());
		GLWRAP_EC_CALL(glBindBuffer)(GL_COPY_WRITE_BUFFER, native_handle());

		sizei_t sz = 0;
		GLWRAP_EC_CALL(glGetBufferParameteriv)(GL_COPY_READ_BUFFER, GL_BUFFER_SIZE, &sz);

		GLWRAP_EC_CALL(glBufferData)(GL_COPY_WRITE_BUFFER, size, nullptr, static_cast<enum_t>(_usage));
		GLWRAP_EC_CALL(glCopyBufferSubData)(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, sz);
	}

private:
	sizei_t get_stride() const
	{
		return m_alignment.get_stride();
	}

	// TODO: generate the alignment on the fly?
	alignment_type m_alignment;
};

}
