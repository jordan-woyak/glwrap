#pragma once

#include "attribute.hpp"
#include "framebuffer.hpp"
#include "shader.hpp"
#include "program.hpp"

#include "detail/variable.hpp"

namespace gl
{

class context;

namespace detail
{

// TODO: allow generic inputs/outputs with generic location and other specifiers
// TODO: allow adding an input and output to two shader_builders at the same time

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
			|| std::is_same<P, sampler_2d>::value
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
		result.set_source(generate_full_source());
		result.compile();
		return result;
	}

	// TODO: rename? create a single stage program for program pipelines
	program create_shader_program(context& _glc) const
	{
		std::string const src = generate_full_source();
		std::array<const char*, 1> srcs = {src.c_str()};
		return program(GLWRAP_EC_CALL(glCreateShaderProgramv)(shader_type, srcs.size(), srcs.data()));
	}

private:
	static const enum_t shader_type = static_cast<enum_t>(T);

	// TODO: take advantage of OpenGL being able to take an array of strings
	// instead of building one big string
	std::string generate_full_source() const
	{
		std::string src = "#version 330\n";
		src += "#extension GL_ARB_explicit_uniform_location : require\n";
		src += "#extension GL_ARB_explicit_attrib_location : require\n";
		for (auto& line : m_header_lines)
			src += line;
		src += m_source;

		return std::move(src);
	}

	// TODO: move this elsewhere:
	template <typename P, typename L>
	std::string get_glsl_definition(const std::string& _prefix, const variable_description<P, L>& _desc) const
	{
		// TODO: support unspecified location
		// TODO: support dualsource: layout(location = 0, index = 1)
		// TODO: support fragdata: layout(xfb_buffer = 2, xfb_offset = 0)
		
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
