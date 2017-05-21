#pragma once

#include <chrono>

namespace gl
{

namespace detail
{

struct sync_obj
{
	static void create_objs(sizei_t _n, GLsync* _objs)
	{
		// TODO: this existing isn't sensible
	}

	static void delete_objs(sizei_t _n, GLsync* _objs)
	{
		while (_n--)
		{
			GLWRAP_EC_CALL(glDeleteSync)(*(_objs++));
		}
	}
};

}

class context;

// TODO: name?
class sync : public detail::native_handle_base<GLsync, detail::sync_obj>
{
	friend class context;

public:
	// allow timeout for in the future ?
	void wait()
	{
		GLWRAP_EC_CALL(glWaitSync)(native_handle(), 0, GL_TIMEOUT_IGNORED);
	}

	// use the flags parameter ?
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
		: native_handle_base<GLsync, detail::sync_obj>(_handle)
	{}
};

}
