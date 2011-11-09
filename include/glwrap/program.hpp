#pragma once

#include <list>

#include <boost/format.hpp>

#include "uniform.hpp"
#include "attribute.hpp"
#include "framebuffer.hpp"
#include "vertex_array.hpp"

namespace gl
{

enum class primitive : GLenum
{
	points = GL_POINTS,
	line_strip = GL_LINE_STRIP,
	line_loop = GL_LINE_LOOP,
	lines = GL_LINES,
	line_strip_adjacency = GL_LINE_STRIP_ADJACENCY,
	lines_adjacency = GL_LINES_ADJACENCY,
	triangle_strip = GL_TRIANGLE_STRIP,
	triangle_fan = GL_TRIANGLE_FAN,
	triangles = GL_TRIANGLES,
	triangle_strip_adjacency = GL_TRIANGLE_STRIP_ADJACENCY,
	triangles_adjacency = GL_TRIANGLES_ADJACENCY
};

class program : public native_handle_base<GLuint>
{
public:
	~program()
	{
		glDeleteProgram(native_handle());
	}

	void compile()
	{
		// TODO: error check
		GLuint vshad = glCreateShader(GL_VERTEX_SHADER);
		GLuint fshad = glCreateShader(GL_FRAGMENT_SHADER);

		std::string uniform_header;
		{
		//int cur_location = 0;
		for (auto& var : m_uniforms)
		{
			uniform_header += (boost::format("uniform %s %s;\n")
				% var->get_type_name() % var->get_name()).str();
			//cur_location += var->get_index_count();
		}
		}

		{
		std::string vshad_header("#version 330\n");

		//int cur_location = 0;
		for (auto& var : m_attributes)
		{
			vshad_header += (boost::format("in %s %s;\n")
				% var->get_type_name() % var->get_name()).str();
			//cur_location += var->get_index_count();
		}

		std::array<const char*, 3> vshad_src = {{vshad_header.c_str(), uniform_header.c_str(), m_vertex_src.c_str()}};
		glShaderSource(vshad, vshad_src.size(), vshad_src.data(), 0);
		}

		{
		std::string fshad_header("#version 330\n");

		int cur_location = 0;
		for (auto& var : m_fragdatas)
		{
			fshad_header += (boost::format("layout(location = %i) out %s %s;\n")
				% cur_location % var->get_type_name() % var->get_name()).str();
			var->set_location(cur_location);
			// TODO: correct?
			cur_location += 1;
		}

		std::array<const char*, 3> fshad_src = {{fshad_header.c_str(), uniform_header.c_str(), m_fragment_src.c_str()}};
		glShaderSource(fshad, fshad_src.size(), fshad_src.data(), 0);
		}

		glAttachShader(native_handle(), vshad);
		glAttachShader(native_handle(), fshad);

		// attached shaders will be deleted on program deletion
		glDeleteShader(vshad);
		glDeleteShader(fshad);

		glLinkProgram(native_handle());

		for (auto& var : m_uniforms)
			var->set_location(glGetUniformLocation(native_handle(), var->get_name().c_str()));

		for (auto& var : m_attributes)
			var->set_location(glGetAttribLocation(native_handle(), var->get_name().c_str()));
	}

	std::string get_log() const
	{
		GLint log_length;
		glGetProgramiv(native_handle(), GL_INFO_LOG_LENGTH, &log_length);

		std::string log;

		if (log_length)
		{
			std::vector<GLchar> log_buffer(log_length);
			glGetProgramInfoLog(native_handle(), log_length, NULL, log_buffer.data());

			log.assign(log_buffer.begin(), log_buffer.end());
		}

		return log;
	}

	bool is_good() const
	{
		GLint status;
		glGetProgramiv(native_handle(), GL_LINK_STATUS, &status);
		return (GL_TRUE == status);
	}

//	explicit operator bool() const
//	{
//		return is_good();
//	}

	// TODO: kill
	void set_vertex_shader_source(const std::string& _src)
	{
		m_vertex_src = _src;
	}

	void set_fragment_shader_source(const std::string& _src)
	{
		m_fragment_src = _src;
	}

	template <typename T>
	uniform<T> create_uniform(const std::string& _name)
	{
		m_uniforms.push_back(std::unique_ptr<variable_base>(new variable<T>(_name)));
		return uniform<T>(std::prev(m_uniforms.end()));
	}

	template <typename T>
	attribute<T> create_attribute(const std::string& _name)
	{
		m_attributes.push_back(std::unique_ptr<variable_base>(new variable<T>(_name)));
		return attribute<T>(std::prev(m_attributes.end()));
	}

	template <typename T>
	fragdata<T> create_fragdata(const std::string& _name)
	{
		m_fragdatas.push_back(std::unique_ptr<variable_base>(new variable<T>(_name)));
		return fragdata<T>(std::prev(m_fragdatas.end()));
	}

	template <typename T, typename T2>
	void set_uniform(uniform<T>& _uniform, const T2& _value)
	{
		glUseProgram(native_handle());
		_uniform.set_value(_value);
	}

	void draw_arrays(primitive mode, int_t first, int_t count)
	{
		glUseProgram(native_handle());
		glDrawArrays(static_cast<GLenum>(mode), first, count);
	}

	void draw_arrays(vertex_array& arr, primitive mode, int_t first, int_t count)
	{
		arr.bind();
		glUseProgram(native_handle());
		glDrawArrays(static_cast<GLenum>(mode), first, count);
	}

	explicit program(context& _context)
		: native_handle_base<GLuint>(glCreateProgram())
	{}

private:
	// TODO: kill, move to compile parameters
	std::string m_vertex_src;
	std::string m_fragment_src;

	// TODO: don't need pointer
	std::list<std::unique_ptr<variable_base>> m_attributes;
	std::list<std::unique_ptr<variable_base>> m_fragdatas;
	std::list<std::unique_ptr<variable_base>> m_uniforms;
};

}
