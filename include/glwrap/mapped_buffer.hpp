#pragma once

#include "buffer.hpp"

namespace gl
{

// TODO: differentt type for readable and writable mappings
// TODO: each buffer can only have one mapping, maybe it should be more integrated with the buffer class?

template <typename T, typename A = detail::tight_buffer_alignment<T>>
class mapped_buffer
{	
public:
	typedef T value_type;
	typedef A alignment_type;
	
	mapped_buffer(mapped_buffer const&) = delete;
	mapped_buffer& operator=(mapped_buffer const&) = delete;

	mapped_buffer(mapped_buffer&& _other)
		: m_ptr()
		, m_size()
		, m_buffer()
		, m_alignment(_other.m_alignment)
	{
		// TODO: ugly:
		swap(_other);
	}

	mapped_buffer& operator=(mapped_buffer&& _other)
	{
		// TODO: this is ugly:
		swap(_other);
		return *this;
	}

	void swap(mapped_buffer& _other)
	{
		std::swap(m_ptr, _other.m_ptr);
		std::swap(m_size, _other.m_size);
		std::swap(m_buffer, _other.m_buffer);
		std::swap(m_alignment, _other.m_alignment);
	}

// TODO: movable

	mapped_buffer(buffer<T, A>& _buffer)
		: m_ptr()
		, m_size(_buffer.size())
		, m_buffer(_buffer.native_handle())
		, m_alignment(_buffer.m_alignment)
	{
		// TODO: allow range
		// TODO: don't hardcode access mode

		bitfield_t const access_mode = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;
		
		if (GL_ARB_direct_state_access)
		{
			m_ptr = static_cast<ubyte_t*>(GLWRAP_EC_CALL(glMapNamedBufferRange)(
				m_buffer, 0, m_size * get_stride(), access_mode));
		}
		else
		{
			GLWRAP_EC_CALL(glBindBuffer)(GL_COPY_WRITE_BUFFER, m_buffer);

			m_ptr = static_cast<ubyte_t*>(GLWRAP_EC_CALL(glMapBufferRange)(
				GL_COPY_WRITE_BUFFER, 0, m_size * get_stride(), access_mode));
		}		
	}

	~mapped_buffer()
	{
		if (!m_buffer)
			return;
			
		if (GL_ARB_direct_state_access)
		{
			GLWRAP_EC_CALL(glUnmapNamedBuffer)(m_buffer);
		}
		else
		{
			GLWRAP_EC_CALL(glBindBuffer)(GL_COPY_WRITE_BUFFER, m_buffer);
			GLWRAP_EC_CALL(glUnmapBuffer)(GL_COPY_WRITE_BUFFER);
		}
	}

	void flush_range(sizei_t _start, sizei_t _length)
	{
		intptr_t const gl_offset = _start * get_stride();
		intptr_t const gl_length = _length * get_stride();
		
		if (GL_ARB_direct_state_access)
		{
			GLWRAP_EC_CALL(glFlushMappedBufferRange)(m_buffer, gl_offset, gl_length);
		}
		else
		{
			GLWRAP_EC_CALL(glBindBuffer)(GL_COPY_WRITE_BUFFER, m_buffer);
			GLWRAP_EC_CALL(glFlushMappedNamedBufferRange)(
				GL_COPY_WRITE_BUFFER, gl_offset, gl_length);
		}
	}

	// TODO: useful?
	void flush(sizei_t _start, sizei_t _length)
	{
		flush_range(0, m_size);
	}

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

		iterator& operator+=(sizei_t _offset)
		{
			m_ptr += _offset * m_alignment.get_stride();
			return *this;
		}

		friend iterator operator+(iterator _lhs, sizei_t _offset)
		{
			return _lhs += _offset;
		}

		value_type& operator*() const
		{
			return *reinterpret_cast<T*>(m_ptr);
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
		iterator(ubyte_t* _ptr, alignment_type _alignment)
			: m_ptr(_ptr)
			, m_alignment(_alignment)
		{}

		ubyte_t* m_ptr;
		alignment_type m_alignment;
	};

	iterator begin()
	{
		return {m_ptr, m_alignment};
	}

	iterator end()
	{
		return {m_ptr + m_size * get_stride(), m_alignment};
	}

private:
	sizei_t get_stride() const
	{
		return m_alignment.get_stride();
	}

	ubyte_t* m_ptr;
	sizei_t m_size;
	uint_t m_buffer;
	alignment_type m_alignment;
};

template <typename T, typename A>
mapped_buffer<T, A> map_buffer(buffer<T, A>& _buffer)
{
	return mapped_buffer<T, A>(_buffer);
}

}
