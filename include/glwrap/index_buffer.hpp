#pragma once

#include "native_handle.hpp"
#include "vector.hpp"

namespace gl
{

class context;

template <typename T>
class index_buffer : public globject
{
	friend class context;

public:
	typedef T element_type;

	~index_buffer()
	{
		auto const nh = native_handle();
		glDeleteBuffers(1, &nh);
	}

	template <typename R>
	void assign(R&& _range)
	{
		//auto& contig_range = detail::get_contiguous_range<element_type>(std::forward<R>(_range));
		auto& contig_range = _range;

		auto const begin = std::begin(contig_range);
		auto const size = std::distance(begin, std::end(contig_range));

		static_assert(detail::is_contiguous<R>::value,
			"range must be contiguous");

		static_assert(detail::is_same_ignore_reference_cv<decltype(*begin), element_type>::value,
			"range must contain element_type");

		glBindBuffer(GL_COPY_WRITE_BUFFER, native_handle());
		glBufferData(GL_COPY_WRITE_BUFFER, size * sizeof(element_type), &*begin, GL_STATIC_DRAW);
	}

	explicit index_buffer(context& _context)
		: globject(gen_return(glGenBuffers))
	{}

private:
	void bind()
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, native_handle());
	}
};

}
