#pragma once

#include <list>

//#include <boost/format.hpp>
#include <sstream>

#include "uniform.hpp"
#include "shader.hpp"
#include "vertex_array.hpp"
#include "transform_feedback.hpp"
#include "uniform_block.hpp"

namespace GLWRAP_NAMESPACE
{

class context;

namespace detail
{

struct program_obj
{
	static void create_objs(sizei_t _n, uint_t* _objs)
	{
		while (_n--)
		{
			*(_objs++) = GLWRAP_GL_CALL(glCreateProgram)();
		}
	}

	static void delete_objs(sizei_t _n, uint_t* _objs)
	{
		while (_n--)
		{
			GLWRAP_GL_CALL(glDeleteProgram)(*(_objs++));
		}
	}
};

}

class program : public detail::globject<detail::program_obj>
{
public:
	explicit program(context&)
	{}

	explicit program(native_handle_type _handle, adopt_handle_t _a)
		: detail::globject<detail::program_obj>(_handle, _a)
	{}

	// TODO: GL_PROGRAM_BINARY_RETRIEVABLE_HINT
	// TODO: GL_PROGRAM_SEPARABLE

	void link()
	{
		GLWRAP_GL_CALL(glLinkProgram)(native_handle());
	}

	std::string get_log() const
	{
		return detail::get_shader_string(native_handle(),
			GLWRAP_GL_CALL(glGetProgramiv), GL_INFO_LOG_LENGTH,
			GLWRAP_GL_CALL(glGetProgramInfoLog));
	}

	// TODO: rename
	bool is_linked() const
	{
		GLint status;
		GLWRAP_GL_CALL(glGetProgramiv)(native_handle(), GL_LINK_STATUS, &status);
		return (GL_TRUE == status);
	}

	// TODO: rename
	bool is_valid() const
	{
		GLWRAP_GL_CALL(glValidateProgram)(native_handle());

		GLint status;
		GLWRAP_GL_CALL(glGetProgramiv)(native_handle(), GL_VALIDATE_STATUS, &status);
		return (GL_TRUE == status);
	}

	// This overload takes a value of the proper type.
	// It allows for nice syntax: set_uniform(loc, {0, 0, 0});
	template <typename T>
	void set_uniform(uniform_location<T>& _uniform, typename detail::uniform_value<T>::type const& _value)
	{
		detail::set_program_uniform<T>(native_handle(), _uniform.get_index(), _value);
	}

	// This overload takes a value of any other type, sanity checks are done in the details
	// It allows for bool and bvec to be set from float/int/uvec and vec/ivec/uvec directly.
	template <typename T, typename V>
	typename std::enable_if<!std::is_same<V, typename detail::uniform_value<T>::type>::value>::type
	set_uniform(uniform_location<T>& _uniform, V const& _value)
	{
		detail::set_program_uniform<T>(native_handle(), _uniform.get_index(), _value);
	}

	// TODO: set arrays of locations and bindings in one go?
	// TODO: overload this function to work with texture and image uniforms?
	// and disallow set_uniform for those types?
	template <typename T>
	void set_binding(uniform_block_location<T>& _loc, uniform_block_binding<T> const& _binding)
	{
		GLWRAP_GL_CALL(glUniformBlockBinding)(native_handle(), _loc.get_index(), _binding.get_index());
	}

	template <typename T>
	void set_binding(shader_storage_location<T>& _loc, shader_storage_binding<T> const& _binding)
	{
		GLWRAP_GL_CALL(glShaderStorageBlockBinding)(native_handle(), _loc.get_index(), _binding.get_index());
	}

	// TODO: good name?
	void use_transform_feedback_description(const transform_feedback_descriptor& _desc)
	{
		// TODO: detect the ability to use GL_SEPARATE_ATTRIBS?
		// Is it worth it? any more performant?

		auto const varyings = _desc.build_varyings_array();
		GLWRAP_GL_CALL(glTransformFeedbackVaryings)(native_handle(), varyings.size(), varyings.data(), GL_INTERLEAVED_ATTRIBS);
	}

	// TODO: accept a shader_builder as well?
	// both shader_builder and shader could fulfil some sort of shader_source requirement
	template <shader_type T>
	void attach(const basic_shader<T>& _shad)
	{
		GLWRAP_GL_CALL(glAttachShader)(native_handle(), _shad.native_handle());
	}

	template <shader_type T>
	void detach(const basic_shader<T>& _shad)
	{
		GLWRAP_GL_CALL(glDetachShader)(native_handle(), _shad.native_handle());
	}

private:
};

namespace detail
{

struct program_pipeline_obj
{
	static void create_objs(sizei_t _n, uint_t* _objs)
	{
		if (is_extension_present(GL_ARB_direct_state_access))
		{
			GLWRAP_GL_CALL(glCreateProgramPipelines)(_n, _objs);
		}
		else
		{
			GLWRAP_GL_CALL(glGenProgramPipelines)(_n, _objs);
		}
	}

	static void delete_objs(sizei_t _n, uint_t* _objs)
	{
		GLWRAP_GL_CALL(glDeleteProgramPipelines)(_n, _objs);
	}
};

}

class program_pipeline : public detail::globject<detail::program_pipeline_obj>
{
public:
	explicit program_pipeline(context&)
	{}

	void use_stages(shader_stage _stages, const program& _prog)
	{
		GLWRAP_GL_CALL(glUseProgramStages)(native_handle(), static_cast<enum_t>(_stages), _prog.native_handle());
	}
/*
	void use_all_stages(const program& _prog)
	{
		use_stages(gl::shader_stage::all, _prog);
	}
*/
	std::string get_log() const
	{
		return detail::get_shader_string(native_handle(),
			GLWRAP_GL_CALL(glGetProgramPipelineiv), GL_INFO_LOG_LENGTH,
			GLWRAP_GL_CALL(glGetProgramPipelineInfoLog));
	}

	// TODO: rename
	bool is_valid() const
	{
		GLWRAP_GL_CALL(glValidateProgramPipeline)(native_handle());

		GLint status;
		GLWRAP_GL_CALL(glGetProgramPipelineiv)(native_handle(), GL_VALIDATE_STATUS, &status);
		return (GL_TRUE == status);
	}
private:
};

}
