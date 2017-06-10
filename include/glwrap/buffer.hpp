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

// TODO: move these into buffer_view.hpp ?
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
		if (is_extension_present(GL_ARB_direct_state_access))
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

// TODO: this function is a joke.. but so is the usage hint.
inline buffer_usage get_emulated_usage_from_access_flags(buffer_access _access)
{
	bitfield_t const flags = static_cast<bitfield_t>(_access);

	// Start out with dynamic copy
	buffer_usage usage = buffer_usage::dynamic_copy;

	// If user wants to BufferSubData or map write then give them dynamic draw
	if (flags & (GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT))
	{
		usage = buffer_usage::dynamic_draw;
	}
	// If they don't want either of those but want client storage or map read,
	// then give them dynamic read
	else if (flags & (GL_CLIENT_STORAGE_BIT | GL_MAP_READ_BIT))
	{
		usage = buffer_usage::dynamic_read;
	}

	return usage;
}

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

	// TODO: rename one of these two overloads
	void storage(sizei_t _size, buffer_access _access)
	{
		storage(_size, nullptr, _access);
	}

	template <typename R>
	typename std::enable_if<detail::is_range<R>::value>::type
	storage(const R& _range, buffer_access _access)
	{
		// TODO: work around these limitations with compute shaders
		static_assert(detail::is_contiguous<R>::value,
			"Range must be contiguous.");

		static_assert(std::is_trivially_assignable<value_type&, decltype(*std::begin(_range))>::value,
			"range must be binary compatible with value_type");

		static_assert(sizeof(R) && std::is_same<alignment_type, detail::tight_buffer_alignment<T>>::value,
			"glStorage with non-null ptr is only sensible with tight alignments.");
			
		auto const size = std::distance(std::begin(_range), std::end(_range));

		storage(size, &*std::begin(_range), _access);
	}

	// naming ?
	bool is_persistent_mapping_supported()
	{
		return is_extension_present(GL_ARB_buffer_storage);
	}

private:
	void storage(sizei_t _size, const value_type* _data, buffer_access _access)
	{
		if (is_extension_present(GL_ARB_buffer_storage))
		{
			bitfield_t const flags = static_cast<bitfield_t>(_access);
				
			if (is_extension_present(GL_ARB_direct_state_access))
			{
				GLWRAP_GL_CALL(glNamedBufferStorage)(native_handle(), _size * get_stride(), _data, flags);
			}
			else
			{
				GLWRAP_GL_CALL(glBindBuffer)(GL_COPY_WRITE_BUFFER, native_handle());				
				GLWRAP_GL_CALL(glBufferStorage)(GL_COPY_WRITE_BUFFER, _size * get_stride(), _data, flags);
			}
		}
		// Emulate buffer storage support:
		else
		{
			auto const usage = detail::get_emulated_usage_from_access_flags(_access);
			
			if (is_extension_present(GL_ARB_direct_state_access))
			{
				GLWRAP_GL_CALL(glNamedBufferData)(native_handle(), _size * get_stride(), _data, static_cast<enum_t>(usage));
			}
			else
			{
				GLWRAP_GL_CALL(glBindBuffer)(GL_COPY_WRITE_BUFFER, native_handle());
				GLWRAP_GL_CALL(glBufferData)(GL_COPY_WRITE_BUFFER, _size * get_stride(), _data, static_cast<enum_t>(usage));
			}
		}
	}

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
		
		if (is_extension_present(GL_ARB_direct_state_access))
		{
			GLWRAP_GL_CALL(glGetNamedBufferParameteriv)(native_handle(), GL_BUFFER_SIZE, &sz);
		}
		else
		{
			GLWRAP_GL_CALL(glBindBuffer)(GL_COPY_WRITE_BUFFER, native_handle());
			GLWRAP_GL_CALL(glGetBufferParameteriv)(GL_COPY_WRITE_BUFFER, GL_BUFFER_SIZE, &sz);
		}
		
		return sz;
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

template <typename R>
auto make_buffer(context& _glc, const R& _range, buffer_access _access) -> buffer<typename R::value_type>
{
	buffer<typename R::value_type> buf(_glc);
	buf.storage(_range, _access);

	return buf;
}

}
