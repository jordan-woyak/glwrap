#pragma once

//#include <GL/glut.h>
#include <GL/glew.h>
#include <GL/gl.h>

namespace gl
{

enum class shader_type : GLenum
{
	vertex = GL_VERTEX_SHADER,
	geometry = GL_GEOMETRY_SHADER,
	fragment = GL_FRAGMENT_SHADER,
};

template <shader_type T>
class shader;

typedef shader<shader_type::vertex> vertex_shader;
typedef shader<shader_type::geometry> geometry_shader;
typedef shader<shader_type::fragment> fragment_shader;

}
