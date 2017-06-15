#pragma once

namespace GLWRAP_NAMESPACE
{

enum class query_type : GLenum
{
	samples_passed = GL_SAMPLES_PASSED,
	//any_samples_passed = GL_ANY_SAMPLES_PASSED,
	any_samples_passed_conservative = GL_ANY_SAMPLES_PASSED_CONSERVATIVE,
	//primitives_generated = GL_PRIMITIVES_GENERATED,
	transform_feedback_primitives_written = GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN,
	//time_elapsed = GL_TIME_ELAPSED
};

namespace detail
{

struct query_obj
{
	static void create_objs(enum_t _type, sizei_t _n, uint_t* _objs)
	{
		if (is_extension_present(GL_ARB_direct_state_access))
		{
			GLWRAP_GL_CALL(glCreateQueries)(_type, _n, _objs);
		}
		else
		{
			GLWRAP_GL_CALL(glGenQueries)(_n, _objs);
		}
	}

	static void delete_objs(sizei_t _n, uint_t* _objs)
	{
		GLWRAP_GL_CALL(glDeleteQueries)(_n, _objs);
	}
};

}

class query : public detail::globject<detail::query_obj>
{
public:
	explicit query(context&, query_type _type)
		: detail::globject<detail::query_obj>(static_cast<enum_t>(_type))
		, m_type(_type)
	{}

	void start()
	{
		GLWRAP_GL_CALL(glBeginQuery)(static_cast<enum_t>(m_type), native_handle());
	}

	void stop()
	{
		// TODO: stupid stupid
		GLWRAP_GL_CALL(glEndQuery)(static_cast<enum_t>(m_type));
	}

	uint_t result() const
	{
		GLuint result = 0;
		GLWRAP_GL_CALL(glGetQueryObjectuiv)(native_handle(), GL_QUERY_RESULT, &result);
		return result;
	}

	bool result_available() const
	{
		GLuint result = 0;
		GLWRAP_GL_CALL(glGetQueryObjectuiv)(native_handle(), GL_QUERY_RESULT_AVAILABLE, &result);
		return GL_TRUE == result;
	}

private:
	// TODO: kill this state:
	query_type m_type;
};

}
