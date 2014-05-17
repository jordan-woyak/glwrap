#pragma once

#pragma comment(lib, "glew32s.lib")
#define GLEW_STATIC
#include <GL/glew.h>
#define FREEGLUT_STATIC
#include <GL/glut.h>

namespace gl
{

class context;

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

enum class texture_type : GLenum
{
	// TODO: array 1d 2d 3d
	texture_1d = GL_TEXTURE_1D,
	texture_2d = GL_TEXTURE_2D,
	texture_3d = GL_TEXTURE_3D,

	texture_rectangle = GL_TEXTURE_RECTANGLE,

	// TODO: 2d multisample [array]

	texture_cube_map = GL_TEXTURE_CUBE_MAP,

	texture_buffer = GL_TEXTURE_BUFFER,
};

template <texture_type T>
class texture;

typedef texture<texture_type::texture_1d> texture_1d;
typedef texture<texture_type::texture_2d> texture_2d;
typedef texture<texture_type::texture_3d> texture_3d;
typedef texture<texture_type::texture_rectangle> texture_rectangle;
typedef texture<texture_type::texture_cube_map> texture_cube_map;
typedef texture<texture_type::texture_buffer> texture_buffer;


}
