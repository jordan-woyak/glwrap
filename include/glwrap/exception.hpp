#pragma once

namespace gl
{

// derive from std::exception?
class exception
{
public:
	GLenum error_flag;
};

}
