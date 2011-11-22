#pragma once

namespace gl
{

// TODO: name?
template <typename T>
class uniform_binding
{
	friend class uniform_binding_alloter;

public:
	uint_t get_index() const
	{
		return m_index;
	}

private:
	uniform_binding(uint_t _index)
		: m_index(_index)
	{}

	uint_t m_index;
};

// TODO: name?
class uniform_binding_alloter
{
public:
	// TODO: really need context?
	uniform_binding_alloter(context& _context)
		: m_current_index()
		, m_max_uniform_buffer_bindings()
	{
		GLint max_bindings{};
		glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &max_bindings);
		m_max_uniform_buffer_bindings = max_bindings;
	}

	template <typename T>
	uniform_binding<T> allot()
	{
		if (m_current_index == m_max_uniform_buffer_bindings)
			throw exception();

		return {m_current_index++};
	}

private:
	uint_t m_current_index;
	uint_t m_max_uniform_buffer_bindings;
};

class uniform_buffer : globject
{
public:
	explicit uniform_buffer(context& _context)
		: globject(gen_return(glGenBuffers))
	{}

	~uniform_buffer()
	{
		auto const nh = native_handle();
		glDeleteBuffers(1, &nh);
	}
};

}
