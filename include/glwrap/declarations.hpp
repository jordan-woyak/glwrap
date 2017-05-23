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
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/type_trait.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <type_traits>
#include <array>

namespace GLWRAP_NAMESPACE
{

typedef GLvoid void_t;
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

template <typename T>
struct normalized
{
	static_assert(std::is_integral<T>::value, "normalized is only sane for integral types.");

	// TODO: allow normalized ivec/uivec as well
	
	normalized& operator=(float_t _val)
	{
		// This is valid for both signed and unsigned types.
		// OpenGL defines the range as [MIN + 1, MAX]
		val = T(_val * std::numeric_limits<T>::max());
		
		return *this;
	}
	
	T val;
};

typedef normalized<byte_t> norm_byte_t;
typedef normalized<ubyte_t> norm_ubyte_t;
typedef normalized<short_t> norm_short_t;
typedef normalized<ushort_t> norm_ushort_t;
typedef normalized<int_t> norm_int_t;
typedef normalized<uint_t> norm_uint_t;

// TODO: gl::array probably isn't needed
template <typename T, sizei_t N>
using array = std::array<T, N>;

class context;

namespace detail
{

// TODO: These globals are ugly!!
thread_local context* g_current_context = {};
thread_local bool g_check_every_gl_call = {};
	
static const bool_t IS_UNIFORM_MATRIX_TRANSPOSE_NEEDED = GL_FALSE;

/*
enum class buffer_target
{
	array = GL_ARRAY_BUFFER,
	atomic_counter = GL_ATOMIC_COUNTER_BUFFER,
	copy_read = GL_COPY_READ_BUFFER,
	copy_write = GL_COPY_WRITE_BUFFER,
	draw_indirect = GL_DRAW_INDIRECT_BUFFER,
	dispatch_indirect = GL_DISPATCH_INDIRECT_BUFFER,
	element_array = GL_ELEMENT_ARRAY_BUFFER,
	pixel_pack = GL_PIXEL_PACK_BUFFER,
	pixel_unpack = GL_PIXEL_UNPACK_BUFFER,
	shader_storage = GL_SHADER_STORAGE_BUFFER,
	transform_feedback = GL_TRANSFORM_FEEDBACK_BUFFER,
	uniform = GL_UNIFORM_BUFFER,
};
*/

}

class context;

enum class shader_type : GLenum
{
	compute = GL_COMPUTE_SHADER,
	vertex = GL_VERTEX_SHADER,
	//tess_control = GL_TESS_CONTROL_SHADER,
	//tess_evaluation = GL_TESS_EVALUATION_SHADER,
	//geometry = GL_GEOMETRY_SHADER,
	fragment = GL_FRAGMENT_SHADER,
};

// TODO: allow combinations:
enum class shader_stage : GLenum
{
	//compute = GL_COMPUTE_SHADER_BIT,
	vertex = GL_VERTEX_SHADER_BIT,
	//tess_control = GL_TESS_CONTROL_SHADER_BIT,
	//tess_evaluation = GL_TESS_EVALUATION_SHADER_BIT,
	//geometry = GL_GEOMETRY_SHADER_BIT,
	fragment = GL_FRAGMENT_SHADER_BIT,

	all = GL_ALL_SHADER_BITS,
};

template <shader_type T>
class basic_shader;

template <shader_type T>
class shader_builder;

typedef basic_shader<shader_type::compute> compute_shader;
typedef basic_shader<shader_type::vertex> vertex_shader;
typedef basic_shader<shader_type::fragment> fragment_shader;

typedef shader_builder<shader_type::compute> compute_shader_builder;
typedef shader_builder<shader_type::vertex> vertex_shader_builder;
typedef shader_builder<shader_type::fragment> fragment_shader_builder;

enum class texture_type : enum_t
{
	//texture_1d = GL_TEXTURE_1D, // not in ES
	texture_2d = GL_TEXTURE_2D,
	texture_3d = GL_TEXTURE_3D,

	//texture_1d_array = GL_TEXTURE_1D_ARRAY,
	texture_2d_array = GL_TEXTURE_2D_ARRAY,

	//texture_rectangle = GL_TEXTURE_RECTANGLE, // not in ES

	texture_cube_map = GL_TEXTURE_CUBE_MAP,
	//texture_cube_map_array = GL_TEXTURE_CUBE_MAP_ARRAY,

	//texture_buffer = GL_TEXTURE_BUFFER, // not in ES

	texture_2d_multisample = GL_TEXTURE_2D_MULTISAMPLE,
	//texture_2d_multisample_array = GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
};

// TODO: use this or actual types?
/*
enum class texture_data_type
{
	// also handles normalized integer formats
	floating,
	
	signed_integral,
	
	unisgned_integral,
};
*/

template <texture_type T, typename D>
class basic_texture;

typedef basic_texture<texture_type::texture_2d, float_t> texture_2d;
typedef basic_texture<texture_type::texture_3d, float_t> texture_3d;
typedef basic_texture<texture_type::texture_2d_array, float_t> texture_2d_array;
typedef basic_texture<texture_type::texture_cube_map, float_t> texture_cube_map;
typedef basic_texture<texture_type::texture_2d_multisample, float_t> texture_2d_multisample;

typedef basic_texture<texture_type::texture_2d, int_t> itexture_2d;
typedef basic_texture<texture_type::texture_3d, int_t> itexture_3d;
typedef basic_texture<texture_type::texture_2d_array, int_t> itexture_2d_array;
typedef basic_texture<texture_type::texture_cube_map, int_t> itexture_cube_map;
typedef basic_texture<texture_type::texture_2d_multisample, int_t> itexture_2d_multisample;

typedef basic_texture<texture_type::texture_2d, uint_t> utexture_2d;
typedef basic_texture<texture_type::texture_3d, uint_t> utexture_3d;
typedef basic_texture<texture_type::texture_2d_array, uint_t> utexture_2d_array;
typedef basic_texture<texture_type::texture_cube_map, uint_t> utexture_cube_map;
typedef basic_texture<texture_type::texture_2d_multisample, uint_t> utexture_2d_multisample;

struct draw_arrays_indirect_cmd
{
	uint_t count;
	uint_t instance_count;
	uint_t first;
	uint_t base_instance; // reserved on ES
};

struct draw_elements_indirect_cmd
{
	uint_t count;
	uint_t instance_count;
	uint_t first_index;
	uint_t base_vertex;
	uint_t base_instance; // reserved on ES
};

/*
// TODO: straight uvec3 or this, or a struct of 3 uint ?
// Should this just be a typedef of uvec3?
struct dispatch_indirect_cmd
{
	uvec3 num_groups;
};
*/

}
