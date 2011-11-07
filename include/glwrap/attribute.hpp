#pragma once

#include "variable.hpp"
#include "vertex_buffer.hpp"
#include "uptr.hpp"

#include <list>

namespace gl
{

class program;

template <typename T>
class attribute
{
	friend class program;
	friend class vertex_array;

public:
	void bind(vertex_buffer_component<fvec2>& _comp)
	{
		_comp.bind_to_attrib((*m_iter)->get_location());
	}

	std::size_t get_location() const
	{
		return (*m_iter)->get_location();
	}

private:
	typedef std::list<uptr<variable_base>>::iterator iter_t;

	attribute(iter_t _iter)
		: m_iter(_iter)
	{}

	iter_t m_iter;
};

}

