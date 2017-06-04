#pragma once

#include "buffer.hpp"

namespace GLWRAP_NAMESPACE
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

	template <template <typename, typename> class B>
	explicit mapped_buffer(buffer_view<B, T, A>& _view)
		: m_ptr()
		, m_size(_view.size())
		, m_buffer(_view.buffer())
		, m_alignment(_view.alignment())
	{
		// TODO: don't hardcode access mode
		// TODO: allow for all the other flags: persistent and such.

		bitfield_t const access_mode = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;
		
		if (GL_ARB_direct_state_access)
		{
			m_ptr = static_cast<ubyte_t*>(GLWRAP_GL_CALL(glMapNamedBufferRange)(m_buffer,
				_view.byte_offset(), _view.byte_length(), access_mode));
		}
		else
		{
			GLWRAP_GL_CALL(glBindBuffer)(GL_COPY_WRITE_BUFFER, m_buffer);

			m_ptr = static_cast<ubyte_t*>(GLWRAP_GL_CALL(glMapBufferRange)(GL_COPY_WRITE_BUFFER,
				_view.byte_offset(), _view.byte_length(), access_mode));
		}		
	}

	// FYI, unmap flushes if GL_MAP_FLUSH_EXPLICIT_BIT is not set
	~mapped_buffer()
	{
		if (!m_buffer)
			return;
			
		if (GL_ARB_direct_state_access)
		{
			GLWRAP_GL_CALL(glUnmapNamedBuffer)(m_buffer);
		}
		else
		{
			GLWRAP_GL_CALL(glBindBuffer)(GL_COPY_WRITE_BUFFER, m_buffer);
			GLWRAP_GL_CALL(glUnmapBuffer)(GL_COPY_WRITE_BUFFER);
		}
	}

	void flush_range(sizei_t _start, sizei_t _length)
	{
		intptr_t const gl_offset = _start * get_stride();
		intptr_t const gl_length = _length * get_stride();
		
		if (GL_ARB_direct_state_access)
		{
			GLWRAP_GL_CALL(glFlushMappedBufferRange)(m_buffer, gl_offset, gl_length);
		}
		else
		{
			GLWRAP_GL_CALL(glBindBuffer)(GL_COPY_WRITE_BUFFER, m_buffer);
			GLWRAP_GL_CALL(glFlushMappedNamedBufferRange)(
				GL_COPY_WRITE_BUFFER, gl_offset, gl_length);
		}
	}

	// TODO: useful?
	void flush(sizei_t _start, sizei_t _length)
	{
		flush_range(0, m_size);
	}

private:
	struct iter_state
	{
		friend class indexing_iterator<iter_state>;

		typedef uint_t index_type;
		typedef T value_type;
	// TODO: make private:
	//private:
		index_type m_index;
		
		ubyte_t* m_ptr;
		uint_t m_stride;

		index_type& get_index()
		{
			return m_index;
		}

		const index_type& get_index() const
		{
			return m_index;
		}

		value_type& deref() const
		{
			return *reinterpret_cast<value_type*>(m_ptr + m_index * m_stride);
		}
	};

public:
	typedef indexing_iterator<iter_state> iterator;

	iterator begin()
	{
		return {iter_state{0, m_ptr, get_stride()}};
	}

	iterator end()
	{
		return {iter_state{m_size, m_ptr, get_stride()}};
	}

private:
	uint_t get_stride() const
	{
		return m_alignment.get_stride();
	}

	ubyte_t* m_ptr;
	uint_t m_size;
	uint_t m_buffer;
	alignment_type m_alignment;
};

template <template <typename, typename> class B, typename T, typename A>
mapped_buffer<T, A> map_buffer(buffer_view<B, T, A>& _view)
{
	return mapped_buffer<T, A>(_view);
}

}
