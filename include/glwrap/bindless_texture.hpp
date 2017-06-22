
#pragma once

namespace GLWRAP_NAMESPACE
{

template <texture_type Type, typename DataType>
class basic_texture_handle
{
public:
  typedef uint64_t native_handle_type;

  explicit basic_texture_handle(basic_texture<Type, DataType>& _tex)
    : m_native_handle(GLWRAP_GL_CALL(glGetTextureHandleARB)(_tex.native_handle()))
  {}

  explicit basic_texture_handle(basic_texture<Type, DataType>& _tex, sampler& _samp)
    : m_native_handle(GLWRAP_GL_CALL(glGetTextureHandleARB)(_tex.native_handle(), _samp.native_handle()))
  {}

  void make_resident()
  {
    GLWRAP_GL_CALL(glMakeTextureHandleResidentARB)(native_handle());
  }

  void make_non_resident()
  {
    GLWRAP_GL_CALL(glMakeTextureHandleNonResidentARB)(native_handle());
  }

  bool is_resident() const
  {
    return GLWRAP_GL_CALL(glIsTextureHandleResidentARB)(native_handle());
  }

  // TODO: rename?
  native_handle_type native_handle() const
  {
    return m_native_handle;
  }

private:
  native_handle_type m_native_handle;
};

template <texture_type Type, typename DataType>
basic_texture_handle<Type, DataType> make_texture_handle(basic_texture<Type, DataType>& _tex)
{
  return basic_texture_handle<Type, DataType>{_tex};
}

template <texture_type Type, typename DataType>
basic_texture_handle<Type, DataType> make_texture_handle(basic_texture<Type, DataType>& _tex, sampler& _samp)
{
  return basic_texture_handle<Type, DataType>{_tex, _samp};
}

typedef basic_texture_handle<texture_type::texture_2d, float_t> texture_handle_2d;
typedef basic_texture_handle<texture_type::texture_3d, float_t> texture_handle_3d;
typedef basic_texture_handle<texture_type::texture_2d_array, float_t> texture_handle_2d_array;
typedef basic_texture_handle<texture_type::texture_cube_map, float_t> texture_handle_cube_map;
typedef basic_texture_handle<texture_type::texture_2d_multisample, float_t> texture_handle_2d_multisample;

typedef basic_texture_handle<texture_type::texture_2d, int_t> itexture_handle_2d;
typedef basic_texture_handle<texture_type::texture_3d, int_t> itexture_handle_3d;
typedef basic_texture_handle<texture_type::texture_2d_array, int_t> itexture_handle_2d_array;
typedef basic_texture_handle<texture_type::texture_cube_map, int_t> itexture_handle_cube_map;
typedef basic_texture_handle<texture_type::texture_2d_multisample, int_t> itexture_handle_2d_multisample;

typedef basic_texture_handle<texture_type::texture_2d, uint_t> utexture_handle_2d;
typedef basic_texture_handle<texture_type::texture_3d, uint_t> utexture_handle_3d;
typedef basic_texture_handle<texture_type::texture_2d_array, uint_t> utexture_handle_2d_array;
typedef basic_texture_handle<texture_type::texture_cube_map, uint_t> utexture_handle_cube_map;
typedef basic_texture_handle<texture_type::texture_2d_multisample, uint_t> utexture_handle_2d_multisample;

// TODO: image handles

}
