#pragma once

namespace gl
{

class context;

// TODO: better name
class technique
{
	friend class context;

public:
	explicit technique(context&)
		: m_program()
		, m_vertex_array()
		, m_element_array()
		, m_mode()
	{}

	void use_program(program& _prog)
	{
		m_program = _prog.native_handle();
	}

	void use_vertex_array(vertex_array& _vert)
	{
		m_vertex_array = _vert.native_handle();
	}

	template <typename T>
	void use_element_array(buffer<T>& _buff)
	{
		static_assert(std::is_same<T, ubyte_t>::value
			|| std::is_same<T, ushort_t>::value
			|| std::is_same<T, uint_t>::value
			, "must be ubyte, uint, ushort");

		m_element_array = _buff.native_handle();
		m_element_type = detail::data_type_enum<T>();
	}

	void use_primitive_mode(primitive _prim)
	{
		m_mode = static_cast<GLenum>(_prim);
	}

private:
	void bind()
	{
		glUseProgram(m_program);
		glBindVertexArray(m_vertex_array);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_element_array);
	}

	GLenum get_type() const
	{
		return m_element_type;
	}

	GLenum get_mode() const
	{
		return m_mode;
	}

	GLuint m_program;
	GLuint m_vertex_array;

	GLuint m_element_array;
	GLenum m_element_type;

	GLenum m_mode;
};

}
