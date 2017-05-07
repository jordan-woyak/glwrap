#pragma once

namespace gl
{

// TODO: name?
template <typename T>
class transform_feedback_binding
{
	friend class transform_feedback_binding_alloter;

public:
	int_t get_index() const
	{
		return m_index;
	}

private:
	transform_feedback_binding(int_t _index)
		: m_index(_index)
	{}

	int_t m_index;
};

// TODO: name?
class transform_feedback_binding_alloter
{
public:
	// TODO: really need context?
	transform_feedback_binding_alloter(context& _context)
		: m_current_index()
	{
		// TODO: using correct "max"?
		detail::gl_get(GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS, &m_max_tf_separate_components);
	}

	template <typename T>
	transform_feedback_binding<T> allot()
	{
		if (m_current_index == m_max_tf_separate_components)
			throw exception();

		return {m_current_index++};
	}

private:
	int_t m_current_index;
	int_t m_max_tf_separate_components;
};

}
