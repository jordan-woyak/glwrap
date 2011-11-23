#pragma once

namespace gl
{

template <typename T>
class buffer : public globject
{
	friend class context;

public:
	typedef T element_type;

	~buffer()
	{
		auto const nh = native_handle();
		glDeleteBuffers(1, &nh);
	}

//	void resize(std::size_t _size)
//	{
//		glBufferData(GL_buffer, _size * sizeof(element_type), NULL, GL_STATIC_DRAW);
//	}

	vertex_attribute<T> attrib()
	{
		return {sizeof(T), 0, native_handle()};
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

	void assign(buffer const& _other)
	{
		glBindBuffer(GL_COPY_READ_BUFFER, _other.native_handle());
		glBindBuffer(GL_COPY_WRITE_BUFFER, native_handle());

		sizei_t size = 0;
		glGetBufferParameteriv(GL_COPY_READ_BUFFER, GL_BUFFER_SIZE, &size);

		glBufferData(GL_COPY_WRITE_BUFFER, size, nullptr, GL_STATIC_DRAW);
		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, size);
	}

	explicit buffer(context& _context)
		: globject(gen_return(glGenBuffers))
	{}
};

}
