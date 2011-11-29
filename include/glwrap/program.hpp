#pragma once

#include <list>

#include <boost/format.hpp>

#include "uniform.hpp"
#include "attribute.hpp"
#include "framebuffer.hpp"
#include "vertex_array.hpp"
#include "transform_feedback.hpp"
#include "uniform_block.hpp"

namespace gl
{

class context;

// TODO: rename/put elsewhere
template <typename T>
struct vertex_out_varying
{
	friend class program;

public:
	std::string const& get_name() const
	{
		return (*m_iter)->get_name();
	}

private:
	typedef std::list<std::unique_ptr<variable_base>>::iterator iter_t;

	vertex_out_varying(iter_t _iter)
		: m_iter(_iter)
	{}

	iter_t m_iter;
};

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

		// uniforms
		std::string uniform_header;
		for (auto& uni : m_uniforms)
		{
			uniform_header += (boost::format("uniform %s %s;\n")
				% uni.get_variable().get_type_name() % uni.get_variable().get_name()).str();
		}

		for (auto& block : m_uniform_blocks)
		{
			uniform_header += (boost::format("layout(std140) uniform %s {\n") % block.get_name()).str();

			// TODO: actually look at the offset value!!!
			// currently super broken, not handling padding for shit
			for (auto& var : block.m_members)
				uniform_header += (boost::format("uniform %s %s;\n")
					% var.second->get_type_name() % var.second->get_name()).str();

			// TODO: allow scoped block
			uniform_header += "};\n";
		}

		// vertex shader
		// TODO: lame
		if (!m_vertex_src.empty())
		{
			std::string vshad_header("#version 330\n");

			for (auto& var : m_vertex_out_varying)
			{
				vshad_header += (boost::format("out %s %s;\n")
					% var->get_type_name() % var->get_name()).str();
			}

			for (auto& var : m_attributes)
			{
				vshad_header += (boost::format("in %s %s;\n")
					% var->get_type_name() % var->get_name()).str();
			}

			std::array<const char*, 3> vshad_src = {{vshad_header.c_str(), uniform_header.c_str(), m_vertex_src.c_str()}};
			GLuint vshad = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vshad, vshad_src.size(), vshad_src.data(), 0);

			glAttachShader(native_handle(), vshad);
			glDeleteShader(vshad);
		}

		// fragment shader
		// TODO: lame
		if (!m_fragment_src.empty())
		{
			std::string fshad_header("#version 330\n");

			for (auto& var : m_fragdatas)
			{
				fshad_header += (boost::format("out %s %s;\n")
					% var->get_type_name() % var->get_name()).str();
			}

			std::array<const char*, 3> fshad_src = {{fshad_header.c_str(), uniform_header.c_str(), m_fragment_src.c_str()}};
			GLuint fshad = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fshad, fshad_src.size(), fshad_src.data(), 0);

			glAttachShader(native_handle(), fshad);
			glDeleteShader(fshad);
		}
	}

	void link()
	{
		// TODO: allow use of GL_SEPARATE_ATTRIBS
		// transform feedback bindings
		std::vector<const char*> varyings(m_feedback_varyings.size());
		std::transform(m_feedback_varyings.begin(), m_feedback_varyings.end(),
			varyings.begin(), std::mem_fun_ref(&std::string::c_str));
		glTransformFeedbackVaryings(native_handle(), varyings.size(), varyings.data(), GL_SEPARATE_ATTRIBS);

		glLinkProgram(native_handle());

		// update uniform locations
		for (auto& uni : m_uniforms)
			uni.set_location(glGetUniformLocation(native_handle(),
				uni.get_variable().get_name().c_str()));

		for (auto& block : m_uniform_blocks)
			block.m_location = glGetUniformBlockIndex(native_handle(), block.m_name.c_str());
	}

	std::string get_log() const
	{
		//glValidateProgram(native_handle());

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

	// TODO: change function name?
	template <typename T>
	vertex_out_varying<T> create_vertex_out_varying(const std::string& _name)
	{
		m_vertex_out_varying.push_back(std::unique_ptr<variable_base>(new variable<T>(_name)));
		return vertex_out_varying<T>(std::prev(m_vertex_out_varying.end()));
	}

	template <typename T>
	uniform_block<T> create_uniform_block(const std::string& _name, uniform_block_definition<T> const& _block_def)
	{
		m_uniform_blocks.push_back(detail::uniform_block_variable(_name));

		// TODO: silly
		m_uniform_blocks.back().m_members = _block_def.m_members;
		return uniform_block<T>(std::prev(m_uniform_blocks.end()));
	}

	template <typename T>
	void set_uniform(uniform<T>& _uniform, typename detail::uniform_value<T>::type const& _value)
	{
		bind();
		_uniform.set_value(_value);
	}

	template <typename T>
	void set_uniform_block(uniform_block<T>& _block, uniform_block_binding<T> const& _binding)
	{
		glUniformBlockBinding(native_handle(), _block.m_iter->m_location, _binding.get_index());
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

	template <typename T>
	void bind_transform_feedback(vertex_out_varying<T>& _varying, transform_feedback_binding<T> const& _binding)
	{
		//glBindFragDataLocation(native_handle(), _number.get_index(), _fragdata.get_name().c_str());

		auto const index = _binding.get_index();

		if (index >= m_feedback_varyings.size())
			m_feedback_varyings.resize(index + 1);

		m_feedback_varyings[index] = _varying.get_name();
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
	std::list<std::unique_ptr<variable_base>> m_vertex_out_varying;

	std::vector<std::string> m_feedback_varyings;

	// TODO: store ptr or copy?
	std::list<detail::uniform_block_variable> m_uniform_blocks;

	std::list<uniform_variable> m_uniforms;
};

}
