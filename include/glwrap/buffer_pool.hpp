#pragma once

#include <set>

#include "buffer.hpp"
#include "buffer_view.hpp"

namespace GLWRAP_NAMESPACE
{

namespace detail
{

struct buffer_block
{
	uint_t start;
	uint_t length;
	uint_t buffer;
};

}

class buffer_pool;

// TODO: rename
// TODO: pass the pool to the sub buffer ctor to create it
template <typename T, typename A = detail::tight_buffer_alignment<T>>
class sub_buffer : public buffer_view<sub_buffer, T, A>
{
public:
	typedef buffer_iterator<T, A> iterator;
	typedef T value_type;
	typedef A alignment_type;
	typedef buffer_view<::GLWRAP_NAMESPACE::sub_buffer, T, A> view_type;

	friend view_type;
	friend buffer_pool;

	sub_buffer(const sub_buffer&) = delete;
	sub_buffer& operator=(const sub_buffer&) = delete;

	sub_buffer(sub_buffer&& _other)
		: sub_buffer()
	{
		swap(_other);
	}

	sub_buffer& operator=(sub_buffer&& _other)
	{
		sub_buffer().swap(*this);
		swap(_other);
	}

	void swap(sub_buffer& _other)
	{
		std::swap(m_block, _other.m_block);
		std::swap(m_size, _other.m_size);
		std::swap(m_alignment, _other.m_alignment);
		std::swap(m_pool, _other.m_pool);
	}

	~sub_buffer();

	uint_t capacity() const
	{
		return m_block.length / m_alignment.get_stride();
	}

private:
	sub_buffer()
		: m_block()
		, m_size()
		, m_alignment(sizeof(T))
		, m_pool()
	{}

	// TODO: get alignment from buffer_pool
	sub_buffer(const detail::buffer_block& _block, uint_t _size, buffer_pool* _pool)
		: m_block(_block)
		, m_size(_size)
		, m_alignment(sizeof(value_type))
		, m_pool(_pool)
	{}

	uint_t get_buffer() const
	{
		return m_block.buffer;
	}

	uint_t get_byte_offset() const
	{
		return m_block.start;
	}

	uint_t get_byte_length() const
	{
		return m_block.length;
	}

	const alignment_type& get_alignment() const
	{
		return m_alignment;
	}

	detail::buffer_block m_block;

	// TODO: this is in bytes.. kinda messy
	uint_t m_size;

	alignment_type m_alignment;

	buffer_pool* m_pool;
};

// TODO: when to delete buffers and when to downsize blocks
class buffer_pool
{
	template <typename T, typename A>
	friend class sub_buffer;
	
	// TODO: make this configurable:
	enum : uint_t
	{
		min_buffer_size = 1024 * 1024,
		min_sub_buffer_size = 64,
	};

	typedef buffer<ubyte_t> buffer_type;

	typedef typename buffer_type::native_handle_type buffer_handle_type;
	
public:
	buffer_pool(context& _glc, buffer_access _access)
		: m_glc(_glc)
		, m_access(_access)
	{}

	template <typename T, typename A = detail::tight_buffer_alignment<T>>
	sub_buffer<T, A> get(uint_t _length)
	{
		// TODO: allow for probability of a sub buffer growing, sort blocks by this hint
		// TODO: allow for pre-allocated space to be larger than initial requested size
		
		A alignment(sizeof(T));
		uint_t const req_size = alignment.get_stride() * _length;

		auto it = m_blocks.begin();

		// TODO: make sure block is aligned!
		if (m_blocks.end() != it && req_size <= it->data.length /* && block is aligned */)
		{
			auto blk = it->data;
			m_blocks.erase(it);

			for (uint_t half_len = blk.length / 2; half_len >= min_sub_buffer_size && req_size <= half_len; half_len /= 2)
			{
				blk.length = half_len;
				
				auto half_blk = blk;
				half_blk.start += half_len;
				release_block(half_blk);
			}

			std::cout << "got block: " << blk.start << " " << blk.length << " #" << blk.buffer << std::endl;

			return sub_buffer<T, A>(blk, req_size, this);
		}
		else
		{
			std::cout << "no block available, allocating new buffer" << std::endl;

			// TODO: look for a block with different alignment?

			// TODO: does this round up?
			uint_t const next_pow2 = 1 << glm::log2(req_size);

			uint_t const new_buf_size = std::max(next_pow2, (uint_t)min_buffer_size);
			
			buffer_type buf(m_glc);
			buf.storage(new_buf_size, m_access);

			std::cout << "#" << buf.native_handle() << " size: " << new_buf_size << std::endl;
			
			m_blocks.insert({{{0, new_buf_size, buf.native_handle()}}});
			
			m_buffers.emplace_back(std::move(buf));

			return get<T, A>(_length);
		}
	}

	void debug_stats()
	{
		std::cout << "buffer count: " << m_buffers.size() << std::endl;

		std::cout << "block count: " << m_blocks.size() << std::endl;
		for (auto& b : m_blocks)
			std::cout << "#" << b.data.buffer << " size: " << b.data.length << " ";

		std::cout << std::endl;
	}

private:
	struct block
	{
		detail::buffer_block data;

		bool operator<(const block& _rhs) const
		{
			return data.length > _rhs.data.length;
		}
	};

	void release_block(const detail::buffer_block& blk)
	{
		// TODO: merge blocks, but keep strict alignment?

		//std::cout << "block released." << std::endl;

		m_blocks.insert({blk});
	}

	// TODO: kill this:
	context& m_glc;

	std::vector<buffer_type> m_buffers;

	// TODO: use vector and std::make_heap ?
	std::multiset<block> m_blocks;

	buffer_access m_access;
};

// TODO: move this back into the class def:
template <typename T, typename A>
sub_buffer<T, A>::~sub_buffer()
{
	m_pool->release_block(m_block);
}

}
