
#pragma once

#include "vector.hpp"

namespace gl
{

typedef void(*glgenfunc)(GLsizei, GLuint*);
GLuint gen_return(glgenfunc f)
{
	GLuint name;
	f(1, &name);
	return name;
}

struct pixel_block
{
	basic_vec<int_t, 2> lower, upper;

	void bind(GLenum _target) const
	{

	}
};

}
