#pragma once

namespace gl
{

// TODO: name?
template <typename T>
class transform_feedback_binding
{
	friend class transform_feedback_binding_alloter;

public:
	uint_t get_index() const
	{
		return m_index;
	}

private:
	transform_feedback_binding(uint_t _index)
		: m_index(_index)
	{}

	uint_t m_index;
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
		GLint max_bindings{};
		glGetIntegerv(GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS, &max_bindings);
		m_max_transform_feedback_separate_components = max_bindings;
	}

	template <typename T>
	transform_feedback_binding<T> allot()
	{
		if (m_current_index == m_max_transform_feedback_separate_components)
			throw exception();

		return {m_current_index++};
	}

private:
	uint_t m_current_index;
	uint_t m_max_transform_feedback_separate_components;
};

}
