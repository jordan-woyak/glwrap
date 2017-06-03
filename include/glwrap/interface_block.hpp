
#pragma once

namespace GLWRAP_NAMESPACE
{

//class vertex_shader_builder;

namespace detail
{

template <typename T>
struct struct_layout
{
	const std::string& get_name() const
	{
		return m_name;
	}
	
	void set_name(std::string _name)
	{
		m_name = std::move(_name);
	}
	
	template <typename M>
	void add_member(M T::*_member, const std::string& _name)
	{		
		member m;

		m.offset = get_member_offset(_member);
		m.name = _name;
		m.definition = detail::get_type_name<M>()
			+ " " + _name
			+ detail::glsl_var_suffix<M>::suffix();

		members.push_back(std::move(m));
	}
#if 0
	// TODO: member names
	template <typename... M>
	void validate_layout(context& _glc, M... _members)
	{
		vertex_shader_builder vshad(_glc);

		// TODO: just call them member1,2,3 instead of having an instance name

		std::string src =
R"(
uniform TestFoo
{)" +
	get_definitions_string() +
R"(};
void main() {}
)";

		std::vector<const char*> names;

		for (auto& m : members)
		{
			names.push_back(m.name.c_str());
		}

		//std::cout << "src: " << src << std::endl;

		vshad.set_source(src);

		auto prog = vshad.create_shader_program(_glc);

		//std::cout << "layout prog info: " << prog.get_log() << std::endl;
		// TODO: check for valid program

		std::vector<uint_t> indices(names.size());

		GLWRAP_GL_CALL(glGetUniformIndices)(prog.native_handle(), names.size(), names.data(), indices.data());

		std::vector<int_t> offsets(indices.size());
		GLWRAP_GL_CALL(glGetActiveUniformsiv)(prog.native_handle(), indices.size(), indices.data(), GL_UNIFORM_OFFSET, offsets.data());

		for (size_t i = 0; i != members.size(); ++i)
		{
			if (members[i].offset != offsets[i])
			{
				std::cout << "Error! unmatched member offset \"" << members[i].name
					<< "\". desired: " << members[i].offset << " got: " << offsets[i] << std::endl;

				throw std::exception();
			}
		}
	}
#endif
	std::string get_definitions_string() const
	{
		std::string ret;
		for (auto& m : members)
			ret += "\t" + m.definition + ";\n";

		return ret;
	}

// TODO: private

	struct member
	{
		std::string name;
		std::string definition;

		int_t offset;
	};

	std::string m_name;
	std::vector<member> members;
};

}

}
