#pragma once

namespace GLWRAP_NAMESPACE
{

template <typename T, typename A>
class mapped_buffer;

// TODO: move this into the buffer class?
template <typename T, typename A>
class buffer_iterator
{
	//template <typename, typename>
	//friend class buffer;

public:
	typedef T value_type;
	typedef A alignment_type;

	buffer_iterator& operator++()
	{
		return *this +=1;
	}

	buffer_iterator& operator+=(std::size_t _val)
	{
		m_offset += _val * get_stride();
		return *this;
	}

	friend buffer_iterator operator+(buffer_iterator _lhs, std::size_t _offset)
	{
		return _lhs += _offset;
	}

	// TODO: this should be private
	buffer_iterator(uint_t _buffer, ubyte_t* _offset, const alignment_type& _alignment)
		: m_offset(_offset)
		, m_alignment(_alignment)
		, m_buffer(_buffer)
	{}

	uint_t get_buffer() const
	{
		return m_buffer;
	}
	
	ubyte_t* get_offset() const
	{
		return m_offset;
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
	ubyte_t* m_offset;
	alignment_type m_alignment;
	uint_t m_buffer;
};

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
		return {buffer(), (ubyte_t*)0 + byte_offset(), alignment()};
	}

	iterator end()
	{
		return {buffer(), (ubyte_t*)0 + byte_offset() + byte_length(), alignment()};
	}

	uint_t size() const
	{
		return byte_length() / stride();
	}

	// TODO: do I want this?
	std::vector<value_type> get_range(uint_t _start, uint_t _count)
	{
		auto mbuf = map_buffer(*this, gl::map_access::read);
		
		// TODO: should I just return the mapped buffer?
		return std::vector<value_type>(mbuf.begin(), mbuf.end());
	}

	// TODO: parameter order?
	template <typename R>
	void assign_range(R&& _range, sizei_t _offset)
	{
		//auto& contig_range = detail::get_contiguous_range<element_type>(std::forward<R>(_range));
		auto& contig_range = _range;

		auto begin = std::begin(contig_range);
		auto const end = std::end(contig_range);

		static_assert(detail::is_contiguous<R>::value,
			"range must be contiguous");

		static_assert(std::is_trivially_assignable<value_type, decltype(*begin)>::value,
			"range must be binary compatible with value_type");

		auto const str = stride();

		// If alignment doesn't match we need to copy just one value at a time..
		// TODO: use a compute shader + shader storage to work around this.
		uint_t const batch_amt = (sizeof(value_type) == str) ? std::distance(begin, end) : 1;

		auto func = GLWRAP_GL_CALL(glNamedBufferSubData);
		uint_t target = buffer();

		if (!is_extension_present(GL_ARB_direct_state_access))
		{
			GLWRAP_GL_CALL(glBindBuffer)(GL_COPY_WRITE_BUFFER, target);
			
			func = GLWRAP_GL_CALL(glBufferSubData);
			target = GL_COPY_WRITE_BUFFER;
		}
			 
		while (begin != end)
		{
			func(target, byte_offset() + _offset * str, batch_amt * sizeof(value_type), &*begin);
			begin += batch_amt;
			_offset += batch_amt;
		}
	}

	void invalidate_range(uint_t _offset, uint_t _length)
	{
		// TODO: allow checking if supported
		if (is_extension_present(GL_ARB_invalidate_subdata))
		{
			auto const str = stride();
			
			GLWRAP_GL_CALL(glInvalidateBufferSubData)(buffer(), _offset * str, _length * str);
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
