#pragma once

#include <list>

//#include <boost/format.hpp>
#include <sstream>

#include "uniform.hpp"
#include "shader.hpp"
#include "vertex_array.hpp"
#include "transform_feedback.hpp"
#include "uniform_block.hpp"

namespace gl
{

class context;

class program : public globject
{
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
			uniform_header += "uniform ";
			uniform_header += uni.get_variable().get_glsl_definition();
			uniform_header += ";\n";
#if 0
			uniform_header += (boost::format("uniform %s %s;\n")
				% uni.get_variable().get_type_name() % uni.get_variable().get_name()).str();
#endif
		}

		for (auto& block : m_uniform_blocks)
		{
			uniform_header += "layout(std140) uniform ";
			uniform_header += block.get_name();
			uniform_header += " {\n";

			// TODO: actually look at the offset value!!!
			// currently super broken, not handling padding for shit
			for (auto& var : block.m_members)
			{
				uniform_header += "uniform ";
				uniform_header += var.second->get_glsl_definition();
				uniform_header += ";\n";
			}

			// TODO: allow scoped block
			uniform_header += "};\n";

#if 0
			uniform_header += (boost::format("layout(std140) uniform %s {\n") % block.get_name()).str();

			// TODO: actually look at the offset value!!!
			// currently super broken, not handling padding for shit
			for (auto& var : block.m_members)
				uniform_header += (boost::format("uniform %s %s;\n")
					% var.second->get_type_name() % var.second->get_name()).str();

			// TODO: allow scoped block
			uniform_header += "};\n";
#endif
		}

		// vertex shader
		if (m_vshad)
		{
			std::string const& shad_header = m_vshad->get_header();
			std::string const& shad_src = m_vshad->get_source();

			std::array<const char*, 3> shad_full = {{shad_header.c_str(), uniform_header.c_str(), shad_src.c_str()}};
			glShaderSource(m_vshad->native_handle(), shad_full.size(), shad_full.data(), 0);
			glCompileShader(m_vshad->native_handle());
		}

		// fragment shader
		if (m_fshad)
		{
			std::string const& shad_header = m_fshad->get_header();
			std::string const& shad_src = m_fshad->get_source();

			std::array<const char*, 3> shad_full = {{shad_header.c_str(), uniform_header.c_str(), shad_src.c_str()}};
			glShaderSource(m_fshad->native_handle(), shad_full.size(), shad_full.data(), 0);
			glCompileShader(m_fshad->native_handle());
		}
	}

	void link()
	{
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

	// TODO: rename
	bool is_valid() const
	{
		glValidateProgram(native_handle());
		GLint status;
		glGetProgramiv(native_handle(), GL_VALIDATE_STATUS, &status);
		return (GL_TRUE == status);
	}

//	explicit operator bool() const
//	{
//		return is_good();
//	}

	template <typename T>
	uniform<T> create_uniform(const std::string& _name)
	{
		static_assert(detail::glslvar::is_valid_glsl_type<T>::value, "Invalid Uniform type.");
		
		m_uniforms.emplace_back(detail::variable<T>(_name));
		return uniform<T>(std::prev(m_uniforms.end()));
	}

	template <typename T>
	uniform_block<T> create_uniform_block(const std::string& _name, uniform_block_definition<T> const& _block_def)
	{
		m_uniform_blocks.push_back(detail::uniform_block_variable(_name));

		// TODO: silly
		m_uniform_blocks.back().m_members = _block_def.m_members;
		return uniform_block<T>(std::prev(m_uniform_blocks.end()));
	}

	// TODO: array support
	template <typename T>
	void set_uniform(uniform<T>& _uniform, typename detail::uniform_value<T>::type const& _value)
	{
		detail::set_program_uniform<T>(native_handle(), _uniform.get_location(),
			detail::uniform_value<T>::convert_to_gl_type(_value));
	}

	template <typename T>
	void set_uniform_block(uniform_block<T>& _block, uniform_block_binding<T> const& _binding)
	{
		glUniformBlockBinding(native_handle(), _block.m_iter->m_location, _binding.get_index());
	}

	template <typename T>
	void bind_attribute(attribute<T>& _attrib, attribute_location<T> const& _location)
	{
		// Subsequent indices after begin_index are automatically handled by OpenGL
		glBindAttribLocation(native_handle(), _location.get_begin_index(), _attrib.get_name().c_str());
	}

	template <typename T>
	void bind_fragdata(fragdata<T>& _fragdata, color_number const& _number)
	{
		glBindFragDataLocation(native_handle(), _number.get_index(), _fragdata.get_name().c_str());
	}

	void use_transform_feedback_description(const transform_feedback_descriptor& _desc)
	{
		// TODO: detect the ability to use GL_SEPARATE_ATTRIBS?
		// Is it worth it? any more performant?
		
		auto const varyings = _desc.build_varyings_array();
		glTransformFeedbackVaryings(native_handle(), varyings.size(), varyings.data(), GL_INTERLEAVED_ATTRIBS);
	}

	// TODO: these 3 will break if repeated
	void attach(vertex_shader& _shad)
	{
		m_vshad = &_shad;
		glAttachShader(native_handle(), _shad.native_handle());
	}

	void attach(geometry_shader& _shad)
	{
		m_gshad = &_shad;
		glAttachShader(native_handle(), _shad.native_handle());
	}

	void attach(fragment_shader& _shad)
	{
		m_fshad = &_shad;
		glAttachShader(native_handle(), _shad.native_handle());
	}

	explicit program(context& _context)
		: globject(glCreateProgram())
		, m_vshad(), m_gshad(), m_fshad()
	{}

private:
	// TODO: don't store globject pointers like this, it will break on swap/move
	vertex_shader* m_vshad;
	geometry_shader* m_gshad;
	fragment_shader* m_fshad;

	// TODO: store ptr or copy?
	std::list<detail::uniform_block_variable> m_uniform_blocks;

	std::list<detail::uniform_variable> m_uniforms;
};

}
