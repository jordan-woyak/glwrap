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
		GLWRAP_EC_CALL(glDeleteProgram)(native_handle());
	}

	void link()
	{
		GLWRAP_EC_CALL(glLinkProgram)(native_handle());
	}

	std::string get_log() const
	{
		//GLWRAP_EC_CALL(glValidateProgram)(native_handle());

		GLint log_length;
		GLWRAP_EC_CALL(glGetProgramiv)(native_handle(), GL_INFO_LOG_LENGTH, &log_length);

		std::string log;

		if (log_length)
		{
			std::vector<GLchar> log_buffer(log_length);
			GLWRAP_EC_CALL(glGetProgramInfoLog)(native_handle(), log_length, NULL, log_buffer.data());

			log.assign(log_buffer.begin(), log_buffer.end());
		}

		return log;
	}

	// TODO: rename
	bool is_good() const
	{
		GLint status;
		GLWRAP_EC_CALL(glGetProgramiv)(native_handle(), GL_LINK_STATUS, &status);
		return (GL_TRUE == status);
	}

	// TODO: rename
	bool is_valid() const
	{
		GLWRAP_EC_CALL(glValidateProgram)(native_handle());
		GLint status;
		GLWRAP_EC_CALL(glGetProgramiv)(native_handle(), GL_VALIDATE_STATUS, &status);
		return (GL_TRUE == status);
	}
/*
	template <typename T>
	uniform_block_location<T> create_uniform_block(const std::string& _name, uniform_block_definition<T> const& _block_def)
	{
		m_uniform_blocks.push_back(detail::uniform_block_variable(_name));

		// TODO: silly
		m_uniform_blocks.back().m_members = _block_def.m_members;
		return uniform_block<T>(std::prev(m_uniform_blocks.end()));
	}
*/
	// TODO: array support
	template <typename T>
	void set_uniform(uniform_location<T>& _uniform, typename detail::uniform_value<T>::type const& _value)
	{
		detail::set_program_uniform<T>(native_handle(), _uniform.get_index(),
			detail::uniform_value<T>::convert_to_gl_type(_value));
	}

/*
	template <typename T>
	void set_uniform_block(uniform_block_location<T>& _block, uniform_block_binding<T> const& _binding)
	{
		GLWRAP_EC_CALL(glUniformBlockBinding)(native_handle(), _block.get_index(), _binding.get_index());
	}
*/
	// TODO: just ban dynamic shader variable bindings in general?
/*
	template <typename T>
	void bind_attribute(attribute<T>& _attrib, attribute_location<T> const& _location)
	{
		// Subsequent indices after begin_index are automatically handled by OpenGL
		GLWRAP_EC_CALL(glBindAttribLocation)(native_handle(), _location.get_begin_index(), _attrib.get_name().c_str());
	}

	template <typename T>
	void bind_fragdata(fragdata<T>& _fragdata, color_number const& _number)
	{
		GLWRAP_EC_CALL(glBindFragDataLocation)(native_handle(), _number.get_index(), _fragdata.get_name().c_str());
	}
*/

	// TODO: good name?
	void use_transform_feedback_description(const transform_feedback_descriptor& _desc)
	{
		// TODO: detect the ability to use GL_SEPARATE_ATTRIBS?
		// Is it worth it? any more performant?
		
		auto const varyings = _desc.build_varyings_array();
		GLWRAP_EC_CALL(glTransformFeedbackVaryings)(native_handle(), varyings.size(), varyings.data(), GL_INTERLEAVED_ATTRIBS);
	}

	// TODO: accept a shader_builder as well?
	template <shader_type T>
	void attach(const shader<T>& _shad)
	{
		GLWRAP_EC_CALL(glAttachShader)(native_handle(), _shad.native_handle());
	}

	explicit program(context& _context)
		: globject(GLWRAP_EC_CALL(glCreateProgram)())
	{}

private:
};

}
