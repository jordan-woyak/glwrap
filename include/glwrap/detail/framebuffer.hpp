
#pragma once

namespace GLWRAP_NAMESPACE
{

namespace detail
{

template <typename T>
void gl_clear_buffer(enum_t _buffer, int_t _index, const T* _val);

template <>
void gl_clear_buffer<int_t>(enum_t _buffer, int_t _index, const int_t* _val)
{
	GLWRAP_GL_CALL(glClearBufferiv)(_buffer, _index, _val);
}

template <>
void gl_clear_buffer<uint_t>(enum_t _buffer, int_t _index, const uint_t* _val)
{
	GLWRAP_GL_CALL(glClearBufferuiv)(_buffer, _index, _val);
}

template <>
void gl_clear_buffer<float_t>(enum_t _buffer, int_t _index, const float_t* _val)
{
	GLWRAP_GL_CALL(glClearBufferfv)(_buffer, _index, _val);
}

template <typename T>
void gl_clear_named_framebuffer(uint_t _fb, enum_t _buffer, int_t _index, const T* _val);

template <>
void gl_clear_named_framebuffer<int_t>(uint_t _fb, enum_t _buffer, int_t _index, const int_t* _val)
{
	GLWRAP_GL_CALL(glClearNamedFramebufferiv)(_fb, _buffer, _index, _val);
}

template <>
void gl_clear_named_framebuffer<uint_t>(uint_t _fb, enum_t _buffer, int_t _index, const uint_t* _val)
{
	GLWRAP_GL_CALL(glClearNamedFramebufferuiv)(_fb, _buffer, _index, _val);
}

template <>
void gl_clear_named_framebuffer<float_t>(uint_t _fb, enum_t _buffer, int_t _index, const float_t* _val)
{
	// TODO: why the need for const_cast.. GLEW Bug?
	GLWRAP_GL_CALL(glClearNamedFramebufferfv)(_fb, _buffer, _index, const_cast<float_t*>(_val));
}

}

}
