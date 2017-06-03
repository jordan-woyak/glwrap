#pragma once

#include "constants.hpp"
#include "uniform_block.hpp"

#include "detail/context.hpp"

namespace GLWRAP_NAMESPACE
{

class vertex_array;

namespace detail
{

// TODO: move these elsewhere?
template <enum_t Param>
struct parameter_alignment
{
	parameter_alignment(sizei_t _size)
	{
		// TODO: would be nice to not call this for every buffer.
		auto const element_size = _size;
		auto const req_alignment = detail::get_parameter<int_t>(Param);

		m_stride = ((element_size + req_alignment - 1) / req_alignment) * req_alignment;
	}

	sizei_t get_stride() const
	{
		return m_stride;
	}

private:
	sizei_t m_stride;
};

typedef parameter_alignment<GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT> uniform_buffer_alignment;
typedef parameter_alignment<GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT> shader_storage_buffer_alignment;

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

template <typename T>
using shader_storage_buffer_iterator = buffer_iterator<T, detail::shader_storage_buffer_alignment>;

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

template <typename T, typename A>
class mapped_buffer;

namespace detail
{

struct buffer_obj
{
	static void create_objs(sizei_t _n, uint_t* _objs)
	{
		if (GL_ARB_direct_state_access)
		{
			GLWRAP_GL_CALL(glCreateBuffers)(_n, _objs);
		}
		else
		{
			GLWRAP_GL_CALL(glGenBuffers)(_n, _objs);
		}
	}

	static void delete_objs(sizei_t _n, uint_t* _objs)
	{
		GLWRAP_GL_CALL(glDeleteBuffers)(_n, _objs);
	}
};

}

// TODO: used gl*NamedBuffer* functions when available:

template <typename T, typename A = detail::tight_buffer_alignment<T>>
class buffer : public detail::globject<detail::buffer_obj>
{
	friend class context;

public:

	static_assert(std::is_trivially_copyable<T>::value, "buffer must be of trivially copyable type.");

	typedef T value_type;
	typedef A alignment_type;

	friend class mapped_buffer<T, A>;

	// TODO: allow creation of a buffer directly from a vector/array

	explicit buffer(context&)
		: m_alignment(sizeof(value_type))
	{}

	// TODO: rename, use glStorage?
	void storage(std::size_t _size, buffer_usage _usage)
	{
		// TODO: proper use of flags
		bitfield_t const flags =
			GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		
		if (GL_ARB_buffer_storage)
		{
			if (GL_ARB_direct_state_access)
			{
				GLWRAP_GL_CALL(glNamedBufferStorage)(native_handle(), _size * get_stride(), nullptr, flags);
			}
			else
			{
				GLWRAP_GL_CALL(glBindBuffer)(GL_COPY_WRITE_BUFFER, native_handle());				
				GLWRAP_GL_CALL(glBufferStorage)(GL_COPY_WRITE_BUFFER, _size * get_stride(), nullptr, flags);
			}
		}
		// Emulate buffer storage support:
		else
		{
			if (GL_ARB_direct_state_access)
			{
				GLWRAP_GL_CALL(glNamedBufferData)(native_handle(), _size * get_stride(), nullptr, static_cast<enum_t>(_usage));
			}
			else
			{
				GLWRAP_GL_CALL(glBindBuffer)(GL_COPY_WRITE_BUFFER, native_handle());
				GLWRAP_GL_CALL(glBufferData)(GL_COPY_WRITE_BUFFER, _size * get_stride(), nullptr, static_cast<enum_t>(_usage));
			}
		}
	}

	// TODO: cache this?
	std::size_t size() const
	{
		sizei_t sz = 0;
		
		if (GL_ARB_direct_state_access)
		{
			GLWRAP_GL_CALL(glGetNamedBufferParameteriv)(native_handle(), GL_BUFFER_SIZE, &sz);
		}
		else
		{
			GLWRAP_GL_CALL(glBindBuffer)(GL_COPY_WRITE_BUFFER, native_handle());
			GLWRAP_GL_CALL(glGetBufferParameteriv)(GL_COPY_WRITE_BUFFER, GL_BUFFER_SIZE, &sz);
		}
		
		return sz / get_stride();
	}

private:
	// TODO: use indexing_iterator
/*
	struct iter_state
	{
		typedef ubyte_t* index_type;
		typedef int value_type;
		
		//ubyte_t* m_offset;
		alignment_type m_alignment;
		uint_t m_buffer;

		value_type& deref(index_type _index) const
		{
			// TODO: implement

			throw exception();
		}
	};
*/

public:
	//typedef indexing_iterator<iter_state> iterator;
	typedef buffer_iterator<T, A> iterator;

	iterator begin()
	{
		//return {nullptr, iter_state{0, m_alignment, native_handle()}};
		return {native_handle(), 0, m_alignment};
	}

	// TODO: this is broken for untight-alignments
	// TODO: rename
	// TODO: parameter order?
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

		if (GL_ARB_direct_state_access)
		{
			GLWRAP_GL_CALL(glNamedBufferSubData)(native_handle(), _offset * get_stride(), size * get_stride(), &*begin);
		}
		else
		{
			GLWRAP_GL_CALL(glBindBuffer)(GL_COPY_WRITE_BUFFER, native_handle());
			GLWRAP_GL_CALL(glBufferSubData)(GL_COPY_WRITE_BUFFER, _offset * get_stride(), size * get_stride(), &*begin);
		}
	}

	// TODO: discourage use of mutable buffers
	// TODO: this is broken for untight-alignments
	// TODO: rename
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

		if (GL_ARB_direct_state_access)
		{
			GLWRAP_GL_CALL(glNamedBufferData)(native_handle(), size * get_stride(), &*begin, static_cast<enum_t>(_usage));
		}
		else
		{
			GLWRAP_GL_CALL(glBindBuffer)(GL_COPY_WRITE_BUFFER, native_handle());
			GLWRAP_GL_CALL(glBufferData)(GL_COPY_WRITE_BUFFER, size * get_stride(), &*begin, static_cast<enum_t>(_usage));
		}
	}

	// TODO: discourage use of mutable buffers
	void assign(buffer const& _other, buffer_usage _usage)
	{
		if (GL_ARB_direct_state_access)
		{
			sizei_t sz = 0;
			GLWRAP_GL_CALL(glGetNamedBufferParameteriv)(_other.native_handle(), GL_BUFFER_SIZE, &sz);
			
			GLWRAP_GL_CALL(glNamedBufferData)(native_handle(), sz, nullptr, static_cast<enum_t>(_usage));
			GLWRAP_GL_CALL(glCopyNamedBufferSubData)(_other.native_handle(), native_handle(), 0, 0, sz);
		}
		else
		{
			GLWRAP_GL_CALL(glBindBuffer)(GL_COPY_READ_BUFFER, _other.native_handle());
			GLWRAP_GL_CALL(glBindBuffer)(GL_COPY_WRITE_BUFFER, native_handle());

			sizei_t sz = 0;
			GLWRAP_GL_CALL(glGetBufferParameteriv)(GL_COPY_READ_BUFFER, GL_BUFFER_SIZE, &sz);
		
			GLWRAP_GL_CALL(glBufferData)(GL_COPY_WRITE_BUFFER, sz, nullptr, static_cast<enum_t>(_usage));
			GLWRAP_GL_CALL(glCopyBufferSubData)(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, sz);
		}
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
