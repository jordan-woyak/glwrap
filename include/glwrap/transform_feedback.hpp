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
	{}

	template <typename T>
	transform_feedback_binding<T> allot()
	{
		// TODO: throw when using > max?

		return {m_current_index++};
	}

private:
	uint_t m_current_index;
};

}
