#pragma once

#include "attribute.hpp"
#include "framebuffer.hpp"

#include "detail/variable.hpp"

namespace GLWRAP_NAMESPACE
{

class context;

namespace detail
{

struct shader_obj
{
	static void create_objs(enum_t _target, sizei_t _n, uint_t* _objs)
	{
		while (_n--)
		{
			*(_objs++) = GLWRAP_GL_CALL(glCreateShader)(_target);
		}
	}

	static void delete_objs(sizei_t _n, uint_t* _objs)
	{
		while (_n--)
		{
			GLWRAP_GL_CALL(glDeleteShader)(*(_objs++));
		}
	}
};

}

// TODO: de-templatify
template <shader_type T>
class basic_shader : public detail::globject<detail::shader_obj>
{
public:
	explicit basic_shader(context&)
		: detail::globject<detail::shader_obj>(static_cast<enum_t>(T))
	{}

	void set_source(const std::string& _src)
	{
		std::array<const char*, 1> shad_full = {{_src.c_str()}};
		GLWRAP_GL_CALL(glShaderSource)(native_handle(), shad_full.size(), shad_full.data(), 0);
	}

	std::string get_source() const
	{
		return detail::get_shader_string(native_handle(),
			GLWRAP_GL_CALL(glGetShaderiv), GL_SHADER_SOURCE_LENGTH,
			GLWRAP_GL_CALL(glGetShaderSource));
	}

	void compile()
	{
		GLWRAP_GL_CALL(glCompileShader)(native_handle());
	}

	bool compile_status() const
	{
		int_t status = 0;
		GLWRAP_GL_CALL(glGetShaderiv)(native_handle(), GL_COMPILE_STATUS, &status);
		return (GL_TRUE == status);
	}

	std::string get_log() const
	{
		return detail::get_shader_string(native_handle(),
			GLWRAP_GL_CALL(glGetShaderiv), GL_INFO_LOG_LENGTH,
			GLWRAP_GL_CALL(glGetShaderInfoLog));
	}

private:
};

}
