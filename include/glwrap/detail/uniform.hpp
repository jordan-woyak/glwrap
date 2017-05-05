#pragma once

namespace gl
{

namespace detail
{

template <typename T, typename Enable = void>
struct uniform_value
{
	typedef T type;
};

template <texture_type T>
struct uniform_value<texture<T>>
{
	typedef texture_unit<texture<T>> type;
};

template <typename T>
struct uniform_value<T, typename std::enable_if<std::extent<T>::value>::type>
{
	typedef std::array<typename std::remove_extent<T>::type, std::extent<T>::value> type;
};

template <typename T, std::size_t Count = 1>
struct set_uniform
{};

template <typename T>
void set_uniform_value(GLuint _location, T const& _value)
{
	set_uniform<T>::set(_location, _value);
}

template <typename T, std::size_t S>
struct set_uniform<std::array<T, S>>
{
	static void set(GLuint _location, std::array<T, S> const& _value)
	{
		static_assert(sizeof(T) == 0, "array types in uniforms incomplete");
	}
};

template <texture_type T>
struct set_uniform<texture_unit<texture<T>>>
{
	static void set(GLuint _location, const texture_unit<texture<T>>& _value)
	{
		glUniform1i(_location, _value.get_index());
	}
};

// float
template <>
inline void set_uniform_value<float_t>(GLuint _location, const float_t& _value)
{
	glUniform1f(_location, _value);
}

// int
template <>
inline void set_uniform_value<int_t>(GLuint _location, const int_t& _value)
{
	glUniform1i(_location, _value);
}

// vec2
template <>
inline void set_uniform_value<vec2>(GLuint _location, const vec2& _value)
{
	glUniform2fv(_location, 1, glm::value_ptr(_value));
}

// vec3
template <>
inline void set_uniform_value<vec3>(GLuint _location, const vec3& _value)
{
	glUniform3fv(_location, 1, glm::value_ptr(_value));
}

// vec4
template <>
inline void set_uniform_value<vec4>(GLuint _location, const vec4& _value)
{
	glUniform4fv(_location, 1, glm::value_ptr(_value));
}

// matrix
template <>
inline void set_uniform_value<mat3>(GLuint _location, const mat3& _value)
{
	glUniformMatrix3fv(_location, 1, GL_FALSE, glm::value_ptr(_value));
}

template <>
inline void set_uniform_value<mat4>(GLuint _location, const mat4& _value)
{
	glUniformMatrix4fv(_location, 1, GL_FALSE, glm::value_ptr(_value));
}

}

}
