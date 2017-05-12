#pragma once

#include <chrono>

namespace gl
{

class context;

// TODO: name?
class sync : public native_handle_base<GLsync>
{
	friend class context;

public:
	~sync()
	{
		GLWRAP_EC_CALL(glDeleteSync)(native_handle());
	}

	void wait()
	{
		GLWRAP_EC_CALL(glWaitSync)(native_handle(), 0, GL_TIMEOUT_IGNORED);
	}

	void client_wait_for(std::chrono::nanoseconds _ns)
	{
		// TODO: return something
		GLWRAP_EC_CALL(glClientWaitSync)(native_handle(), GL_SYNC_FLUSH_COMMANDS_BIT, _ns.count());
	}

	bool is_signaled() const
	{
		GLint result;
		GLWRAP_EC_CALL(glGetSynciv)(native_handle(), GL_SYNC_STATUS, 1, nullptr, &result);
		return GL_SIGNALED == result;
	}

private:
	sync(GLsync _handle)
		: native_handle_base<GLsync>(_handle)
	{}
};

}
