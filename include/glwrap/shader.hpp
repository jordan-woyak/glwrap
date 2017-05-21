#pragma once

#include "attribute.hpp"
#include "framebuffer.hpp"

#include "detail/variable.hpp"

namespace gl
{

class context;

namespace detail
{

template <shader_type T>
struct shader_obj
{
	static void create_objs(sizei_t _n, uint_t* _objs)
	{
		while (_n--)
		{
			*(_objs++) = GLWRAP_EC_CALL(glCreateShader)(static_cast<GLenum>(T));
		}
	}

	static void delete_objs(sizei_t _n, uint_t* _objs)
	{
		while (_n--)
		{
			GLWRAP_EC_CALL(glDeleteShader)(*(_objs++));
		}
	}
};

}

// TODO: de-templatify
template <shader_type T>
class basic_shader : public detail::globject<detail::shader_obj<T>>
{
public:
	explicit basic_shader(context&)
	{}

	void set_source(const std::string& _src)
	{
		std::array<const char*, 1> shad_full = {_src.c_str()};
		GLWRAP_EC_CALL(glShaderSource)(this->native_handle(), shad_full.size(), shad_full.data(), 0);
	}

	std::string get_source() const
	{
		return detail::get_shader_string(this->native_handle(),
			GLWRAP_EC_CALL(glGetShaderiv), GL_SHADER_SOURCE_LENGTH,
			GLWRAP_EC_CALL(glGetShaderSource));
	}

	void compile()
	{
		GLWRAP_EC_CALL(glCompileShader)(this->native_handle());
	}

	bool compile_status() const
	{
		int_t status = 0;
		GLWRAP_EC_CALL(glGetShaderiv)(this->native_handle(), GL_COMPILE_STATUS, &status);
		return (GL_TRUE == status);
	}

	std::string get_log() const
	{
		return detail::get_shader_string(this->native_handle(),
			GLWRAP_EC_CALL(glGetShaderiv), GL_INFO_LOG_LENGTH,
			GLWRAP_EC_CALL(glGetShaderInfoLog));
	}

private:
};

}
