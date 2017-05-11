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

template <typename T>
class transform_feedback_binding_attribute
{
public:
	transform_feedback_binding_attribute(transform_feedback_binding<T> const& _binding)
		: m_index(_binding.get_index())
		, m_offset()
	{}	
	
	transform_feedback_binding_attribute(int_t _index, uint_t _offset)
		: m_index(_index)
		, m_offset(_offset)
	{}

	int_t get_index() const
	{
		return m_index;
	}

	int_t get_offset() const
	{
		return m_offset;
	}

private:
	int_t m_index;
	uint_t m_offset;
};

template <typename T, typename M>
transform_feedback_binding_attribute<M> operator|(transform_feedback_binding<T> const& _binding, M T::*_member)
{
	// TODO: only do this for types that need aligning:
	//static_assert((detail::get_member_offset(_member) % sizeof(M)) == 0, "Member is not aligned.");
	
	return transform_feedback_binding_attribute<M>(_binding.get_index(), detail::get_member_offset(_member));
}

// TODO: make this not needed
template <typename T, typename M>
transform_feedback_binding_attribute<M> operator|(transform_feedback_binding_attribute<T> const& _attr, M T::*_member)
{
	// TODO: only do this for types that need aligning:
	//static_assert((detail::get_member_offset(_member) % sizeof(M)) == 0, "Member is not aligned.");
	
	return transform_feedback_binding_attribute<M>(_attr.get_index(), _attr.get_offset() + detail::get_member_offset(_member));
}

// TODO: alignment
// TODO: name
class transform_feedback_descriptor
{
public:
	friend program;

	template <typename T>
	void bind_vertex_attribute(const vertex_out_varying<T>& _varying, const transform_feedback_binding_attribute<T>& _attr)
	{
		// TODO: assert offsets are aligned
		
		m_bindings[_attr.get_index()][_attr.get_offset()] = _varying.get_name();
	}

private:
	std::vector<const char*> build_varyings_array() const
	{
		std::vector<const char*> result;

		int_t current_binding = 0;

		for (auto& binding_attribs : m_bindings)
		{
			for (int_t const b = binding_attribs.first; b != current_binding; ++current_binding)
			{
				result.emplace_back("gl_NextBuffer");
			}

			int_t current_offset = 0;

			for (auto& offset_names : binding_attribs.second)
			{
				for (int_t const o = offset_names.first; o != current_offset; current_offset += 4)
				{
					// TODO: use gl_SkipComponents1,2,3, or 4 also!!!
					result.emplace_back("gl_SkipComponents1");
				}

				result.emplace_back(offset_names.second.c_str());
			}
		}

		return std::move(result);
	}

	std::map<int_t, std::map<uint_t, std::string>> m_bindings;
};

}
