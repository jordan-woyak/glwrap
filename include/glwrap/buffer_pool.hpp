#pragma once

#include <set>

#include "buffer.hpp"

namespace GLWRAP_NAMESPACE
{

class buffer_pool
{
	// TODO: make this configurable:
	enum : uint_t
	{
		min_buffer_size = 1024 * 1024,
		min_sub_buffer_size = 1024,
	};

	// TODO: change this to uint_t maybe:
	typedef ubyte_t buffer_value_type;
	typedef buffer<buffer_value_type> buffer_type;
	
public:
	buffer_pool(context& _glc)
		: m_glc(_glc)
	{}

	template <typename T, template<typename> typename A = detail::tight_buffer_alignment>
	void get(uint_t _length)
	{
		// TODO: don't ignore alignment
		uint_t const req_size = sizeof(T) * _length / sizeof(buffer_value_type);

		auto it = m_blocks.begin();

		if (m_blocks.end() != it && req_size <= it->length /* && block is aligned */)
		{
			auto blk = *it;
			m_blocks.erase(it);

			for (uint_t half_len = blk.length / 2; half_len >= min_sub_buffer_size && req_size <= half_len; half_len /= 2)
			{
				release_block({blk.start + half_len, half_len});
				
				blk.length = half_len;
			}

			std::cout << "got block: " << blk.start << " " << blk.length << std::endl;
		}
		else
		{
			std::cout << "no block available, allocating new buffer" << std::endl;

			// TODO: look for a block with different alignment?

			uint_t const next_pow2 = 1 << glm::log2(req_size);

			uint_t const new_buf_size = std::max(next_pow2, (uint_t)min_buffer_size);

			buffer_type buf(m_glc);

			// TODO: don't hardcode dynamic_draw
			buf.storage(new_buf_size, gl::buffer_usage::dynamic_draw);
			m_buffers.emplace_back(std::move(buf));
			
			m_blocks.insert({0, new_buf_size});

			return get<T, A>(_length);
		}
	}

private:
	struct block
	{
		uint_t start;
		uint_t length;

		bool operator<(const block& _rhs) const
		{
			return length > _rhs.length;
		}
	};

	void release_block(const block& blk)
	{
		// TODO: merge blocks, but keep strict alignment?

		m_blocks.insert(blk);
	}

	// TODO: kill this:
	context& m_glc;

	// TODO: A vector would be nicer..
	std::list<buffer_type> m_buffers;

	std::set<block> m_blocks;
};

}
