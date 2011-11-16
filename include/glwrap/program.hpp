#pragma once

#include <list>

#include <boost/format.hpp>

#include "uniform.hpp"
#include "attribute.hpp"
#include "framebuffer.hpp"
#include "vertex_array.hpp"

namespace gl
{

class context;

class program : public globject
{
	friend class context;

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
		for (auto& uni : m_uniforms)
		{
			uniform_header += (boost::format("uniform %s %s;\n")
				% uni.get_variable().get_type_name() % uni.get_variable().get_name()).str();
		}
		}

		{
		std::string vshad_header("#version 330\n");

		for (auto& var : m_attributes)
		{
			vshad_header += (boost::format("in %s %s;\n")
				% var->get_type_name() % var->get_name()).str();
		}

		std::array<const char*, 3> vshad_src = {{vshad_header.c_str(), uniform_header.c_str(), m_vertex_src.c_str()}};
		glShaderSource(vshad, vshad_src.size(), vshad_src.data(), 0);
		}

		{
		std::string fshad_header("#version 330\n");

		for (auto& var : m_fragdatas)
		{
			fshad_header += (boost::format("out %s %s;\n")
				% var->get_type_name() % var->get_name()).str();
		}

		std::array<const char*, 3> fshad_src = {{fshad_header.c_str(), uniform_header.c_str(), m_fragment_src.c_str()}};
		glShaderSource(fshad, fshad_src.size(), fshad_src.data(), 0);
		}

		glAttachShader(native_handle(), vshad);
		glAttachShader(native_handle(), fshad);

		// attached shaders will be deleted on program deletion
		glDeleteShader(vshad);
		glDeleteShader(fshad);
	}

	void link()
	{
		glLinkProgram(native_handle());

		// TODO: is this really needed?
		for (auto& uni : m_uniforms)
			uni.set_location(glGetUniformLocation(native_handle(),
				uni.get_variable().get_name().c_str()));
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
		m_uniforms.push_back(uniform_variable(variable<T>(_name)));
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

	template <typename T>
	void set_uniform(uniform<T>& _uniform, typename detail::uniform_value<T>::type const& _value)
	{
		bind();
		_uniform.set_value(_value);
	}

	template <typename T>
	void bind_attribute(attribute<T>& _attrib, attribute_location<T> const& _location)
	{
		glBindAttribLocation(native_handle(), _location.get_index(), _attrib.get_name().c_str());
	}

	template <typename T>
	void bind_fragdata(fragdata<T>& _fragdata, color_number const& _number)
	{
		glBindFragDataLocation(native_handle(), _number.get_index(), _fragdata.get_name().c_str());
	}

	explicit program(context& _context)
		: globject(glCreateProgram())
	{}

private:
	void bind() const
	{
		glUseProgram(native_handle());
	};

	// TODO: kill, move to compile parameters, maybe
	std::string m_vertex_src;
	std::string m_fragment_src;

	// TODO: kill pointers
	std::list<std::unique_ptr<variable_base>> m_attributes;
	std::list<std::unique_ptr<variable_base>> m_fragdatas;
	std::list<uniform_variable> m_uniforms;
};

}
