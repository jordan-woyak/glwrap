#pragma once

#include <set>

#include "buffer.hpp"

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
class sub_buffer
{
	friend buffer_pool;

	sub_buffer()
		: m_block()
		, m_size()
		, m_alignment(sizeof(T))
		, m_pool()
	{}
	
public:
	typedef buffer_iterator<T, A> iterator;
	typedef T value_type;
	typedef A alignment_type;

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

	~sub_buffer()
	{
		m_pool->release_block(m_block);
	}

	iterator begin()
	{
		return {m_block.buffer, (ubyte_t*)0 + m_block.start, m_alignment};
	}

	iterator end()
	{
		return {m_block.buffer, (ubyte_t*)0 + m_block.start + m_size, m_alignment};
	}

	uint_t size() const
	{
		return m_size / m_alignment.get_stride();
	}

	uint_t capacity() const
	{
		return m_block.length / m_alignment.get_stride();
	}

	// TODO: do I want this?
	std::vector<T> get_range(uint_t _start, uint_t _count) const
	{
		gl::mapped_buffer<T, A> mbuf(m_block.buffer, m_block.start, size(), m_alignment);
		
		// TODO: should I just return the mapped buffer?
		return std::vector<T>(mbuf.begin(), mbuf.end());
	}

	// TODO: duplicate code in buffer class
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

		auto const stride = m_alignment.get_stride();

		GLWRAP_GL_CALL(glBindBuffer)(GL_COPY_WRITE_BUFFER, m_block.buffer);
		GLWRAP_GL_CALL(glBufferSubData)(GL_COPY_WRITE_BUFFER, m_block.start + _offset * stride, size * stride, &*begin);
	}

private:
	// TODO: get alignment from buffer_pool
	sub_buffer(const detail::buffer_block& _block, uint_t _size, buffer_pool* _pool)
		: m_block(_block)
		, m_size(_size)
		, m_alignment(sizeof(value_type))
		, m_pool(_pool)
	{}

	detail::buffer_block m_block;

	// TODO: this is in byte.. kinda messy
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
	buffer_pool(context& _glc)
		: m_glc(_glc)
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
			// TODO: don't hardcode dynamic_draw
			buf.storage(new_buf_size, gl::buffer_usage::dynamic_draw);

			std::cout << "#" << buf.native_handle() << " size: " << new_buf_size << std::endl;
			
			m_blocks.insert({0, new_buf_size, buf.native_handle()});
			
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

	// TODO: A vector would be nicer..
	std::list<buffer_type> m_buffers;

	// TODO: use vector and std::make_heap ?
	std::multiset<block> m_blocks;
};

}
