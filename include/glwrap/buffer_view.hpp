#pragma once

namespace GLWRAP_NAMESPACE
{

template <typename T, typename A>
class mapped_buffer;

template <template <typename, typename> class Underlying, typename T, typename A>
class buffer_view;

namespace detail
{

template <typename T, typename A>
struct buffer_iterator_state
{
	friend class indexing_iterator<buffer_iterator_state>;

	//template <template <typename, typename> class Underlying>
	//friend class buffer_view<Underlying, T, A>;
	
	typedef T value_type;
	typedef uint_t index_type;
	//friend class context;

	// TODO: these are ugly:
	uint_t get_buffer() const
	{
		return m_buffer;
	}

	// TODO: ptr == ugly
	ubyte_t* get_offset() const
	{
		return (ubyte_t*)0 + m_byte_offset + m_index * m_stride;
	}

	uint_t get_stride() const
	{
		return m_stride;
	}

// TODO: make private:
//private:
	index_type m_index;
	
	uint_t m_buffer;
	uint_t m_byte_offset;
	uint_t m_stride;

	index_type& get_index()
	{
		return m_index;
	}

	// TODO: fix
	value_type deref() const
	{
		//return *reinterpret_cast<value_type*>(m_ptr + _index * m_alignment.get_stride());
		return value_type();
	}
};

}

template <typename T, typename A>
using buffer_iterator = indexing_iterator<detail::buffer_iterator_state<T, A>>;

template <template <typename, typename> class Underlying, typename T, typename A>
class buffer_view
{
public:
	typedef T value_type;
	typedef A alignment_type;
	typedef buffer_iterator<value_type, alignment_type> iterator;

	friend class mapped_buffer<T, A>;

	iterator begin()
	{
		return {detail::buffer_iterator_state<T, A>{0, buffer(), byte_offset(), stride()}};
	}

	iterator end()
	{
		return {detail::buffer_iterator_state<T, A>{size(), buffer(), byte_offset(), stride()}};
	}

	uint_t size() const
	{
		return byte_length() / stride();
	}

	// TODO: do I want this?
	std::vector<value_type> get_range(uint_t _start, uint_t _count)
	{
		auto mbuf = map_buffer(*this);
		
		// TODO: should I just return the mapped buffer?
		return std::vector<value_type>(mbuf.begin(), mbuf.end());
	}

	// TODO: broken for non-tight alignment
	template <typename R>
	void assign_range(R&& _range, sizei_t _offset)
	{
		//auto& contig_range = detail::get_contiguous_range<element_type>(std::forward<R>(_range));
		auto& contig_range = _range;

		auto const begin = std::begin(contig_range);
		auto const size = std::distance(begin, std::end(contig_range));

		static_assert(detail::is_contiguous<R>::value,
			"range must be contiguous");

		static_assert(detail::is_same_ignore_reference_cv<decltype(*begin), value_type>::value,
			"range must contain value_type");

		// TODO: clamp size so data isn't written into other sub-buffers?

		auto const str = stride();

		if (GL_ARB_direct_state_access)
		{
			GLWRAP_GL_CALL(glNamedBufferSubData)(buffer(), byte_offset() + _offset * str, size * str, &*begin);
		}
		else
		{
			GLWRAP_GL_CALL(glBindBuffer)(GL_COPY_WRITE_BUFFER, buffer());
			GLWRAP_GL_CALL(glBufferSubData)(GL_COPY_WRITE_BUFFER, byte_offset() + _offset * str, size * str, &*begin);
		}
	}

private:
	// TODO: I don't like these function names
	// lacking "get_" is to not clash with the underlying function names
	const Underlying<T, A>& underlying() const
	{
		return *static_cast<const Underlying<T, A>*>(this);
	}

	Underlying<T, A>& underlying()
	{
		return *static_cast<Underlying<T, A>*>(this);
	}

	uint_t buffer() const
	{
		return underlying().get_buffer();
	}

	uint_t byte_length() const
	{
		return underlying().get_byte_length();
	}

	uint_t byte_offset() const
	{
		return underlying().get_byte_offset();
	}

	const alignment_type& alignment() const
	{
		return underlying().get_alignment();
	}

	uint_t stride() const
	{
		return alignment().get_stride();
	}
};

}
