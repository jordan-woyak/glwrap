#pragma once

#include "constants.hpp"
#include "uniform_block.hpp"
#include "buffer_view.hpp"

#include "detail/context.hpp"

namespace GLWRAP_NAMESPACE
{

template <typename T, typename A>
class mapped_buffer;

namespace detail
{

// TODO: move these elsewhere, they don't make sense in detail namespace

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

template <typename T, typename A = detail::tight_buffer_alignment<T>>
class buffer : public detail::globject<detail::buffer_obj>, public buffer_view<buffer, T, A>
{
public:
	static_assert(std::is_trivially_copyable<T>::value, "buffer must be of trivially copyable type.");

	typedef T value_type;
	typedef A alignment_type;
	typedef buffer_view<::GLWRAP_NAMESPACE::buffer, T, A> view_type;

	friend view_type;

	// TODO: allow creation of a buffer directly from a vector/array

	explicit buffer(context&)
		: m_alignment(sizeof(value_type))
	{}

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

	// TODO: discourage use of mutable buffers
	// TODO: rename
	template <typename R>
	void assign(R&& _range, buffer_usage _usage)
	{
		auto const size = std::distance(std::begin(_range), std::end(_range));

		storage(size, _usage);
		view_type::assign_range(std::forward<R>(_range), 0);
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
	uint_t get_buffer() const
	{
		return native_handle();
	}

	uint_t get_byte_offset() const
	{
		return 0;
	}

	uint_t get_byte_length() const
	{
		// TODO: cache this?
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

	const alignment_type& get_alignment() const
	{
		return m_alignment;
	}
	
	sizei_t get_stride() const
	{
		return m_alignment.get_stride();
	}

	// TODO: generate the alignment on the fly?
	alignment_type m_alignment;
};

}
