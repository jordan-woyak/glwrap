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

class context;

class transform_feedback : public globject
{
public:
	explicit transform_feedback(context& _context)
		: globject(gen_return(glGenTransformFeedbacks))
	{}

	~transform_feedback()
	{
		auto const nh = native_handle();
		glDeleteTransformFeedbacks(1, &nh);
	}

	template <typename T, sizei_t S>
	void bind_buffer(transform_feedback_binding<T> const& _binding, const static_buffer_iterator<T, S>& _iter, sizei_t _size)
	{
		// TODO: check strides for alignment ?

		// TODO: make not needed
		detail::scoped_value<detail::parameter::transform_feedback> binding(native_handle());

		glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, _binding.get_index(),
			_iter.get_buffer(), _iter.get_offset() - (ubyte_t*)0, _size * _iter.get_stride());
	}

private:
};

template <typename T, typename M = T, uint_t Offset = 0>
class tf_vertex_attribute
{
public:
};

// TODO: fix
template <typename T, typename M1, typename M2, uint_t Offset>
constexpr auto operator|(tf_vertex_attribute<T, M1, Offset> const& _attrib, M2 T::*_member)
	-> tf_vertex_attribute<T, M2, Offset>
{
	return {};
}

template <typename T>
class transform_feedback_vertex
{
public:
	template <typename M>
	void attribute_binder()
	{
		
	}

private:
};

}
