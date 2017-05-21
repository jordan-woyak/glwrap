#pragma once

namespace gl
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
	static void create_objs(sizei_t _n, uint_t* _objs)
	{
		//if (GL_ARB_direct_state_access)
		//{
			// TODO: fix this, target..
			//GLWRAP_EC_CALL(glCreateQueries)(_n, _objs);
		//}
		//else
		{
			GLWRAP_EC_CALL(glGenQueries)(_n, _objs);
		}
	}

	static void delete_objs(sizei_t _n, uint_t* _objs)
	{
		GLWRAP_EC_CALL(glDeleteQueries)(_n, _objs);
	}
};

}

class query : public detail::globject<detail::query_obj>
{
public:
	explicit query(context&, query_type _type)
		: m_type(_type)
	{}

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
