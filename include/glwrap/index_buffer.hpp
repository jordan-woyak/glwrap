#pragma once

#include "native_handle.hpp"
#include "vector.hpp"

namespace gl
{

class context;

template <typename T>
class index_buffer : public globject
{
	static_assert(std::is_same<T, ubyte_t>::value
		|| std::is_same<T, ushort_t>::value
		|| std::is_same<T, uint_t>::value
		, "must be ubyte, uint, ushort");

	friend class context;

public:
	typedef T element_type;

	~index_buffer()
	{
		auto const nh = native_handle();
		glDeleteBuffers(1, &nh);
	}

	template <typename R>
	typename std::enable_if<!detail::is_contiguous<R>::value, void>::type
	assign(R&& _range)
	{
		assign(std::vector<T>(_range.begin(), _range.end()));
	}

	template <typename R>
	typename std::enable_if<detail::is_contiguous<R>::value, void>::type
	assign(R&& _range)
	{
		static_assert(std::is_same<typename R::value_type, element_type>::value,
			"range must contain element_type");

		glBindBuffer(GL_COPY_WRITE_BUFFER, native_handle());
		glBufferData(GL_COPY_WRITE_BUFFER, _range.size() * sizeof(element_type), _range.data(), GL_STATIC_DRAW);
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
