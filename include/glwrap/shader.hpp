#pragma once

#include "attribute.hpp"
#include "framebuffer.hpp"

#include "detail/variable.hpp"

namespace gl
{

class context;
class program;

// TODO: rename/put elsewhere
template <typename T>
struct vertex_out_varying
{
	friend class shader<shader_type::vertex>;

public:
	std::string const& get_name() const
	{
		return (*m_iter)->get_name();
	}

private:
	typedef std::list<std::unique_ptr<detail::variable_base>>::iterator iter_t;

	vertex_out_varying(iter_t _iter)
		: m_iter(_iter)
	{}

	iter_t m_iter;
};

namespace detail
{

template <shader_type, typename>
struct shader_io_type;

// TODO: separate these for inputs/outputs
template <typename T>
struct shader_io_type<shader_type::vertex, T>
{
	static_assert(glslvar::is_valid_glsl_type<T>::value, "Invalid Vertex Input/Output type.");
	
	typedef attribute<T> input_type;
	typedef vertex_out_varying<T> output_type;
};

template <typename T>
struct shader_io_type<shader_type::fragment, T>
{
	// TODO: this should be more restrictive
	static_assert(glslvar::is_valid_glsl_type<T>::value, "Invalid Fragment Output type.");
	
	//typedef ...<T> input_type;
	typedef fragdata<T> output_type;
};

}

template <shader_type T>
class shader : public globject
{
	friend class program;

public:
	explicit shader(context& glc)
		: globject(glCreateShader(static_cast<GLenum>(T)))
	{}

	void set_source(std::string const& _src)
	{
		m_source = _src;
	}

	std::string const& get_source()
	{
		return m_source;
	}

	std::string get_log() const
	{
		//glValidateProgram(native_handle());

		GLint log_length;
		glGetShaderiv(native_handle(), GL_INFO_LOG_LENGTH, &log_length);

		std::string log;

		if (log_length)
		{
			std::vector<GLchar> log_buffer(log_length);
			glGetShaderInfoLog(native_handle(), log_length, NULL, log_buffer.data());

			log.assign(log_buffer.begin(), log_buffer.end());
		}

		return log;
	}

	template <typename P>
	typename detail::shader_io_type<T, P>::input_type
	create_input(std::string const& _name)
	{
		m_inputs.emplace_back(new detail::variable<P>(_name));
		return std::prev(m_inputs.end());
	}

	template <typename P>
	typename detail::shader_io_type<T, P>::output_type
	create_output(std::string const& _name)
	{
		m_outputs.emplace_back(new detail::variable<P>(_name));
		return std::prev(m_outputs.end());
	}

	template <typename P>
	typename detail::shader_io_type<T, P>::input_type
	assume_input(std::string const& _name)
	{
		m_assumed_vars.emplace_back(new detail::variable<P>(_name));
		return std::prev(m_assumed_vars.end());
	}

	template <typename P>
	typename detail::shader_io_type<T, P>::output_type
	assume_output(std::string const& _name)
	{
		m_assumed_vars.emplace_back(new detail::variable<P>(_name));
		return std::prev(m_assumed_vars.end());
	}

private:
	std::string get_header() const
	{
		// TODO: allow setting version
		std::string header("#version 300 es\n");

		for (auto& var : m_inputs)
		{
#if 0
			header += (boost::format("in %s %s;\n")
				% var->get_type_name() % var->get_name()).str();
#else
			header += "in ";
			header += var->get_type_name();
			header += " ";
			header += var->get_name();
			header += ";\n";
#endif
		}

		for (auto& var : m_outputs)
		{
#if 0
			header += (boost::format("out %s %s;\n")
				% var->get_type_name() % var->get_name()).str();
#else
			header += "out ";
			header += var->get_type_name();
			header += " ";
			header += var->get_name();
			header += ";\n";
#endif
		}

		return header;
	}

	std::string m_source;

	// TODO: don't need to store all this here
	std::list<std::unique_ptr<detail::variable_base>> m_inputs;
	std::list<std::unique_ptr<detail::variable_base>> m_outputs;

	std::list<std::unique_ptr<detail::variable_base>> m_assumed_vars;
};

}
