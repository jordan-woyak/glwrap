#pragma once

#include <chrono>

namespace gl
{

namespace detail
{

struct sync_obj
{
	static void create_objs(enum_t _cond, bitfield_t _flags, sizei_t _n, GLsync* _objs)
	{
		while (_n--)
		{
			*(_objs++) = GLWRAP_EC_CALL(glFenceSync)(_cond, _flags);
		}
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
	explicit sync(sync_condition _cond, bitfield_t _flags)
		: native_handle_base<native_handle_type, detail::sync_obj>(static_cast<GLenum>(_cond), _flags)
	{}
};

}
