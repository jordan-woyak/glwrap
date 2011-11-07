
#pragma once

namespace gl
{

typedef void(*glgenfunc)(GLsizei, GLuint*);
GLuint gen_return(glgenfunc f)
{
	GLuint name;
	f(1, &name);
	return name;
}

}
