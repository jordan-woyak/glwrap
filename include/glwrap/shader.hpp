#pragma once

namespace gl
{

class context;
class program;

enum class shader_type : GLenum
{
	vertex = GL_VERTEX_SHADER,
	geometry = GL_GEOMETRY_SHADER,
	fragment = GL_FRAGMENT_SHADER,
};

template <shader_type T>
class shader : globject
{
	friend class program;

public:
	shader(context& glc)
		: globject(glCreateShader(static_cast<GLenum>(T)))
	{}

	void set_source(std::string const& _src)
	{
		m_source = _src;
	}

	std::string const& get_source()
	{
		return m_source;
	}

	void create_input(std::string const& _name);
	void create_output(std::string const& _name);

private:
	std::string get_modified_source() const
	{
		// TODO: allow setting version
		std::string mod_src("#version 330\n");

		for (auto& var : m_inputs)
		{
			mod_src += (boost::format("in %s %s;\n")
				% var->get_type_name() % var->get_name()).str();
		}

		for (auto& var : m_outputs)
		{
			mod_src += (boost::format("out %s %s;\n")
				% var->get_type_name() % var->get_name()).str();
		}

		return mod_src += m_source;
	}

	std::string m_source;

	// TODO: kill pointers
	std::list<std::unique_ptr<detail::variable_base>> m_inputs;
	std::list<std::unique_ptr<detail::variable_base>> m_outputs;
};

typedef shader<shader_type::vertex> vertex_shader;
typedef shader<shader_type::geometry> geometry_shader;
typedef shader<shader_type::fragment> fragment_shader;

}
