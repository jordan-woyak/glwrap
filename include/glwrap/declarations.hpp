#pragma once

#define GLWRAP_NAMESPACE gl
#define GLWRAP_FUNC_DECL inline
#define GLWRAP_MEMBER_FUNC_DECL inline

#if defined(_MSC_VER)
#pragma comment(lib, "glew32s.lib")
#endif

#define GLEW_STATIC
#include <GL/glew.h>
#define FREEGLUT_STATIC
#include <GL/glut.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/type_trait.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <type_traits>

namespace GLWRAP_NAMESPACE
{

typedef GLboolean bool_t;
typedef GLbyte byte_t;
typedef GLubyte ubyte_t;
typedef GLshort short_t;
typedef GLushort ushort_t;
typedef GLint int_t;
typedef GLuint uint_t;
//typedef GLfixed fixed_t;
typedef GLint64 int64_t;
typedef GLuint64 uint64_t;
typedef GLsizei sizei_t;
typedef GLenum enum_t;
typedef GLintptr intptr_t;
typedef GLsizeiptr sizeiptr_t;
//typedef GLsync sync_t
typedef GLbitfield bitfield_t;
typedef GLhalf half_t;
typedef GLfloat float_t;
//typedef GLclampf clampf_t;
typedef GLdouble double_t;
//typedef GLclampd clampd_t;
// TODO: kill this
typedef double_t depth_t;

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
