#pragma once

#include "native_handle.hpp"
#include "vector.hpp"

namespace gl
{

class context;

template <typename T>
class index_buffer : public native_handle_base<GLuint>
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
	void set_data(const R& range)
	{
		static_assert(std::is_same<typename R::value_type, element_type>::value,
			"range must contain element_type");

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, native_handle());
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, range.size() * sizeof(element_type), range.data(), GL_STATIC_DRAW);
	}

	explicit index_buffer(device& _context)
		: native_handle_base<GLuint>(gen_return(glGenBuffers))
	{}
};

}

