#pragma once

#include <map>
#include <list>
#include <memory>

#include "detail/context.hpp"

namespace GLWRAP_NAMESPACE
{

class program;

namespace detail
{
/*
class uniform_block_variable
{
	friend class gl::program;

public:
	std::string const& get_name() const
	{
		return m_name;
	}

private:
	uniform_block_variable(std::string const& _name)
		: m_name(_name)
		, m_location()
		, m_members()
	{}

	std::string m_name;
	uint_t m_location;
	// TODO: shared_ptr can be killed
	std::map<std::size_t, std::shared_ptr<variable_base>> m_members;
};
*/

struct uniform_block_index
{};

}

template <typename T>
using uniform_block_location = detail::buffer_index<detail::uniform_block_index, T>;

class uniform_block_location_enumerator
{
public:
	// TODO: really need context?
	uniform_block_location_enumerator(context& _context)
		: m_current_index()
		, m_max_uniform_buffer_bindings()
	{
		// TODO: correct parameter?
		detail::gl_get(GL_MAX_UNIFORM_BUFFER_BINDINGS, &m_max_uniform_buffer_bindings);
	}

	template <typename T>
	uniform_block_location<T> get()
	{
		if (m_current_index == m_max_uniform_buffer_bindings)
			throw exception(0);

		return {m_current_index++};
	}

private:
	int_t m_current_index;
	int_t m_max_uniform_buffer_bindings;
};

/*
template <typename T>
class uniform_block_definition
{
	friend class program;

public:
	template <typename A>
	uniform_block_definition& operator()(A T::*_member, const std::string& _name)
	{
		register_member(_member, _name);
		return *this;
	}

	template <typename A>
	void register_member(A T::*_member, const std::string& _name)
	{
		m_members[detail::get_member_offset(_member)] =
			std::unique_ptr<detail::variable_base>(new detail::variable<A>(_name));
	}

private:
	// TODO: shared_ptr can be killed
	std::map<std::size_t, std::shared_ptr<detail::variable_base>> m_members;
};
*/

}
