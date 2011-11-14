#pragma once

namespace gl
{

enum class query_type : GLenum
{
	samples_passed = GL_SAMPLES_PASSED,
	any_samples_passed = GL_ANY_SAMPLES_PASSED,
	primitives_generated = GL_PRIMITIVES_GENERATED,
	transform_feedback_primitives_written = GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN,
	time_elapsed = GL_TIME_ELAPSED
};

class query : public globject
{
public:
	query(context& _context)
		: globject(gen_return(glGenQueries))
	{}

	~query()
	{
		auto const nh = native_handle();
		glDeleteQueries(1, &nh);
	}

	void begin(query_type _type)
	{
		glBeginQuery(static_cast<GLenum>(_type), native_handle());
	}

	void end(query_type _type)
	{
		glBeginQuery(static_cast<GLenum>(_type), native_handle());
	}

	uint64_t result() const
	{
		uint64_t result = 0;
		glGetQueryObjectui64v(native_handle(), GL_QUERY_RESULT, &result);
		return result;
	}
};

}
