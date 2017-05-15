#pragma once

#include "attribute.hpp"
#include "framebuffer.hpp"

#include "detail/variable.hpp"

namespace gl
{

class context;
class program;

namespace detail
{

template <shader_type, typename>
struct shader_io_type;

// TODO: separate these for inputs/outputs
template <typename T>
struct shader_io_type<shader_type::vertex, T>
{
	static_assert(glslvar::is_valid_glsl_type<T>::value, "Invalid Vertex Input/Output type.");

	typedef attribute_location<T> input_location;
};

template <typename T>
struct shader_io_type<shader_type::fragment, T>
{
	// TODO: this should be more restrictive
	static_assert(glslvar::is_valid_glsl_type<T>::value, "Invalid Fragment Output type.");

	typedef fragdata_location<T> output_location;
};

}

// TODO: de-templatify
template <shader_type T>
class shader : public globject
{
	friend class program;

public:
	explicit shader(context& glc)
		: globject(GLWRAP_EC_CALL(glCreateShader)(static_cast<GLenum>(T)))
	{}

	void set_source(const std::string& _src)
	{
		std::array<const char*, 1> shad_full = {_src.c_str()};
		GLWRAP_EC_CALL(glShaderSource)(native_handle(), shad_full.size(), shad_full.data(), 0);
	}

	std::string get_source() const
	{
		//GLWRAP_EC_CALL(glValidateProgram)(native_handle());

		GLint src_length = 0;
		GLWRAP_EC_CALL(glGetShaderiv)(native_handle(), GL_SHADER_SOURCE_LENGTH, &src_length);

		std::string src;

		if (src_length)
		{
			std::vector<GLchar> src_buffer(src_length);
			GLWRAP_EC_CALL(glGetShaderSource)(native_handle(), src_length, nullptr, src_buffer.data());

			src.assign(src_buffer.begin(), src_buffer.end());
		}

		return src;
	}

	void compile()
	{
		GLWRAP_EC_CALL(glCompileShader)(native_handle());
	}

	bool compile_status() const
	{
		int_t status = 0;
		GLWRAP_EC_CALL(glGetShaderiv)(native_handle(), GL_COMPILE_STATUS, &status);
		return (GL_TRUE == status);
	}

	std::string get_log() const
	{
		//GLWRAP_EC_CALL(glValidateProgram)(native_handle());

		GLint log_length = 0;
		GLWRAP_EC_CALL(glGetShaderiv)(native_handle(), GL_INFO_LOG_LENGTH, &log_length);

		std::string log;

		if (log_length)
		{
			std::vector<GLchar> log_buffer(log_length);
			GLWRAP_EC_CALL(glGetShaderInfoLog)(native_handle(), log_length, nullptr, log_buffer.data());

			log.assign(log_buffer.begin(), log_buffer.end());
		}

		return log;
	}

private:
};

// TODO: de-templatify
template <shader_type T>
class shader_builder
{
public:
	// TODO: need context
	shader_builder(context&)
	{}
	
	void set_source(std::string const& _src)
	{
		m_source = _src;
	}

	std::string const& get_source()
	{
		return m_source;
	}

	// TODO: kill the input/output and base it off of the location type?

	template <typename P, typename L>
	typename detail::shader_io_type<T, P>::input_location
	create_input(const variable_description<P, L>& _desc)
	{
		// TODO: check for valid type
		
		m_header_lines.emplace_back(get_glsl_definition<P>("in", _desc));

		return _desc.get_location();
	}

	template <typename P, typename L>
	typename detail::shader_io_type<T, P>::output_location
	create_output(const variable_description<P, L>& _desc)
	{
		// TODO: check for valid type
		
		m_header_lines.emplace_back(get_glsl_definition<P>("out", _desc));

		return _desc.get_location();
	}

	// TODO: can probably eliminate L
	template <typename P, typename L>
	uniform_location<P> create_uniform(const variable_description<P, L>& _desc)
	{
		// TODO: ugly
		static_assert(
			detail::glslvar::is_valid_glsl_type<P>::value
			|| std::is_same<P, texture_2d>::value
			, "Invalid Uniform type.");
		
		m_header_lines.emplace_back(get_glsl_definition<P>("uniform", _desc));

		return _desc.get_location();
	}

/*
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
*/

	// TODO: should this just take and modify a shader?
	shader<T> create_shader(context& _glc) const
	{
		shader<T> result(_glc);

		std::string src = "#version 330\n";
		src += "#extension GL_ARB_explicit_uniform_location : require\n";
		src += "#extension GL_ARB_explicit_attrib_location : require\n";
		for (auto& line : m_header_lines)
			src += line;
		src += m_source;
		
		result.set_source(src);
		result.compile();
		return result;
	}

private:
	// TODO: move this elsewhere:
	template <typename P, typename L>
	std::string get_glsl_definition(const std::string& _prefix, const variable_description<P, L>& _desc) const
	{
		return
			"layout(location = " + std::to_string(_desc.get_location().get_index()) + ") "
			+ _prefix + " "
			+ detail::glslvar::get_type_name<P>()
			+ " " + _desc.get_name()
			+ detail::glslvar::glsl_var_suffix<P>::suffix()
			+ ";\n";
	}

	std::string m_source;
	std::vector<std::string> m_header_lines;
};

}
