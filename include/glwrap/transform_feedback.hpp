#pragma once

#include "detail/context.hpp"

#include "native_handle.hpp"
#include "buffer.hpp"

namespace gl
{

// TODO: name? transform_feedback_buffer_binding ?
template <typename T>
class transform_feedback_binding
{
	friend class transform_feedback_binding_enumerator;

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
class transform_feedback_binding_enumerator
{
public:
	// TODO: really need context?
	transform_feedback_binding_enumerator(context& _context)
		: m_current_index()
	{
		// TODO: correct parameter?
		detail::gl_get(GL_MAX_TRANSFORM_FEEDBACK_BUFFERS, &m_max_tf_buffers);
	}

	template <typename T>
	transform_feedback_binding<T> get()
	{
		if (m_current_index == m_max_tf_buffers)
			throw exception(0);

		return {m_current_index++};
	}

private:
	int_t m_current_index;
	int_t m_max_tf_buffers;
};

class context;

class transform_feedback : public globject
{
public:
	explicit transform_feedback(context& _context)
		: globject(detail::gen_return(glGenTransformFeedbacks))
	{}

	~transform_feedback()
	{
		auto const nh = native_handle();
		GLWRAP_EC_CALL(glDeleteTransformFeedbacks)(1, &nh);
	}

	template <typename T, sizei_t S>
	void bind_buffer(transform_feedback_binding<T> const& _binding, const static_buffer_iterator<T, S>& _iter, sizei_t _size)
	{
		// TODO: check strides for alignment ?

		// TODO: make not needed
		detail::scoped_value<detail::parameter::transform_feedback> binding(native_handle());

		GLWRAP_EC_CALL(glBindBufferRange)(GL_TRANSFORM_FEEDBACK_BUFFER, _binding.get_index(),
			_iter.get_buffer(), _iter.get_offset() - (ubyte_t*)0, _size * _iter.get_stride());
	}

private:
};

template <typename T>
using transform_feedback_binding_attribute = binding_attribute<transform_feedback_binding, T>;

// TODO: alignment
// TODO: name
class transform_feedback_descriptor
{
public:
	friend program;

	// TODO: fix this int-hax
	template <typename T>
	void bind_vertex_attribute(const variable_description<T, int>& _varying, const transform_feedback_binding_attribute<T>& _attr)
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
