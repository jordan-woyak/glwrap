#pragma once

#include "attribute.hpp"
#include "framebuffer.hpp"

#include "detail/variable.hpp"

namespace gl
{

class context;

// TODO: de-templatify
template <shader_type T>
class basic_shader : public globject
{
public:
	explicit basic_shader(context& glc)
		: globject(GLWRAP_EC_CALL(glCreateShader)(static_cast<GLenum>(T)))
	{}

	basic_shader(basic_shader&&) = default;
	basic_shader& operator=(basic_shader&&) = default;

	~basic_shader()
	{
		GLWRAP_EC_CALL(glDeleteShader)(native_handle());
	}

	void set_source(const std::string& _src)
	{
		std::array<const char*, 1> shad_full = {_src.c_str()};
		GLWRAP_EC_CALL(glShaderSource)(native_handle(), shad_full.size(), shad_full.data(), 0);
	}

	std::string get_source() const
	{
		GLint src_length = 0;
		GLWRAP_EC_CALL(glGetShaderiv)(native_handle(), GL_SHADER_SOURCE_LENGTH, &src_length);

		std::string src;

		if (src_length)
		{
			// Don't need null termination
			std::vector<GLchar> buf(src_length - 1);
			
			GLWRAP_EC_CALL(glGetShaderSource)(native_handle(), buf.size(), nullptr, buf.data());
			
			src.assign(buf.begin(), buf.end());
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
			// Don't need null termination
			std::vector<GLchar> buf(log_length - 1);
			
			GLWRAP_EC_CALL(glGetShaderInfoLog)(native_handle(), buf.size(), nullptr, buf.data());

			log.assign(buf.begin(), buf.end());
		}

		return log;
	}

private:
};

}
