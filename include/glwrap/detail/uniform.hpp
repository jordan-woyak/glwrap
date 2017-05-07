#pragma once

namespace gl
{

namespace detail
{

//
// gl_uniform_v / gl_program_uniform_v
//
// provides a single name for glUniform*v / glProgramUniform*v functions
// which accepts typed arguments like vec3 and mat4
//

#define GLWRAP_UNIFORM_V_DEF(u_name, v_type) \
GLWRAP_FUNC_DECL \
void \
gl_uniform_v(int_t _loc, sizei_t _count, const v_type* _value) \
{ \
	glUniform##u_name##v(_loc, _count, value_ptr(*_value)); \
} \
\
GLWRAP_FUNC_DECL \
void \
gl_program_uniform_v(uint_t _program, int_t _loc, sizei_t _count, const v_type* _value) \
{ \
	glProgramUniform##u_name##v(_program, _loc, _count, value_ptr(*_value)); \
}

GLWRAP_UNIFORM_V_DEF(1f, float_t)
GLWRAP_UNIFORM_V_DEF(2f, vec2)
GLWRAP_UNIFORM_V_DEF(3f, vec3)
GLWRAP_UNIFORM_V_DEF(4f, vec4)

GLWRAP_UNIFORM_V_DEF(1i, int_t)
GLWRAP_UNIFORM_V_DEF(2i, ivec2)
GLWRAP_UNIFORM_V_DEF(3i, ivec3)
GLWRAP_UNIFORM_V_DEF(4i, ivec4)

GLWRAP_UNIFORM_V_DEF(1ui, uint_t)
GLWRAP_UNIFORM_V_DEF(2ui, uvec2)
GLWRAP_UNIFORM_V_DEF(3ui, uvec3)
GLWRAP_UNIFORM_V_DEF(4ui, uvec4)

#undef GLWRAP_UNIFORM_V_DEF

#define GLWRAP_UNIFORM_MAT_V_DEF(u_name, v_type) \
GLWRAP_FUNC_DECL \
void \
gl_uniform_v(int_t _loc, sizei_t _count, const v_type* _value) \
{ \
	glUniformMatrix##u_name##v(_loc, _count, IS_UNIFORM_MATRIX_TRANSPOSE_NEEDED, value_ptr(*_value)); \
} \
\
GLWRAP_FUNC_DECL \
void \
gl_program_uniform_v(uint_t _program, int_t _loc, sizei_t _count, const v_type* _value) \
{ \
	glProgramUniformMatrix##u_name##v(_program, _loc, _count, IS_UNIFORM_MATRIX_TRANSPOSE_NEEDED, value_ptr(*_value)); \
}

GLWRAP_UNIFORM_MAT_V_DEF(2f, mat2)
GLWRAP_UNIFORM_MAT_V_DEF(2x3f, mat2x3)
GLWRAP_UNIFORM_MAT_V_DEF(2x4f, mat2x4)

GLWRAP_UNIFORM_MAT_V_DEF(3x2f, mat3x2)
GLWRAP_UNIFORM_MAT_V_DEF(3f, mat3)
GLWRAP_UNIFORM_MAT_V_DEF(3x4f, mat3x4)

GLWRAP_UNIFORM_MAT_V_DEF(4x2f, mat4x2)
GLWRAP_UNIFORM_MAT_V_DEF(4x3f, mat4x3)
GLWRAP_UNIFORM_MAT_V_DEF(4f, mat4)

#undef GLWRAP_UNIFORM_MAT_V_DEF

// TODO: implement not in terms of gl_uniform_v ?
template <typename T>
void gl_uniform(int_t _loc, const T& _value)
{
	gl_uniform_v(_loc, 1, &_value);
}

template <typename T>
void gl_program_uniform(uint_t _program, int_t _loc, const T& _value)
{
	gl_program_uniform_v(_program, _loc, 1, &_value);
}

// TODO: bool and bvec

template <typename T, typename Enable = void>
struct uniform_value
{
	typedef T type;
	typedef T gl_type;

	static gl_type convert_to_gl_type(const type& _val)
	{
		return _val;
	}
};

template <texture_type T>
struct uniform_value<texture<T>>
{
	typedef texture_unit<texture<T>> type;
	typedef int_t gl_type;

	static gl_type convert_to_gl_type(const type& _val)
	{
		return _val.get_index();
	}
};

template <typename T>
struct uniform_value<T, typename std::enable_if<std::extent<T>::value>::type>
{
	typedef std::array<typename std::remove_extent<T>::type, std::extent<T>::value> type;
};

template <typename T>
void set_program_uniform(uint_t _program, int_t _loc, typename uniform_value<T>::gl_type const& _value)
{
	// TODO: is the proper check?
	if (GL_ARB_separate_shader_objects)
	{
		gl_program_uniform(_program, _loc, _value);
	}
	else
	{
		// TODO: ugly
		uint_t const prev_program = detail::get_parameter<int_t>(GL_CURRENT_PROGRAM);

		if (prev_program != _program)
			glUseProgram(_program);
		
		gl_uniform(_loc, _value);

		if (prev_program != _program)
			glUseProgram(prev_program);
	}
}

}

}
