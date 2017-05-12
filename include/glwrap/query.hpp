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
	query(context& _context, query_type _type)
		: globject(detail::gen_return(glGenQueries))
		, m_type(_type)
	{}

	~query()
	{
		auto const nh = native_handle();
		GLWRAP_EC_CALL(glDeleteQueries)(1, &nh);
	}

	void start()
	{
		GLWRAP_EC_CALL(glBeginQuery)(static_cast<GLenum>(m_type), native_handle());
	}

	void stop()
	{
		// TODO: stupid stupid
		GLWRAP_EC_CALL(glEndQuery)(static_cast<GLenum>(m_type));
	}

	uint64_t result() const
	{
		GLuint64 result = 0;
		GLWRAP_EC_CALL(glGetQueryObjectui64v)(native_handle(), GL_QUERY_RESULT, &result);
		return result;
	}

private:
	query_type m_type;
};

}
