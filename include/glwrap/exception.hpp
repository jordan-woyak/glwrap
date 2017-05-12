#pragma once

namespace gl
{

// derive from std::exception?
class exception
{
public:
	exception(GLenum _err)
		: error_flag(_err)
	{}
	
	GLenum error_flag;

	std::string get_string() const
	{
		return get_string(error_flag);
	}

private:
	static std::string get_string(GLenum _err)
	{
		switch (_err)
		{
		case GL_NO_ERROR:
			return "GL_NO_ERROR";
		case GL_INVALID_ENUM:
			return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE:
			return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION:
			return "GL_INVALID_OPERATION";
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			return "GL_INVALID_OPERATION";
		case GL_OUT_OF_MEMORY:
			return "GL_OUT_OF_MEMORY";
		case GL_STACK_UNDERFLOW:
			return "GL_STACK_UNDERFLOW";
		case GL_STACK_OVERFLOW:
			return "GL_STACK_OVERFLOW";
		default:
			return "UNKNOWN";
		}
	}
};

}
