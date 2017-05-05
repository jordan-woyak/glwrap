
#pragma once

namespace GLWRAP_NAMESPACE
{

namespace detail
{

template <typename T>
struct is_vec
{
	static const bool value = false;
};

template <typename T, glm::precision P>
struct is_vec<glm::tvec2<T, P>>
{
	static const bool value = true;
};

template <typename T, glm::precision P>
struct is_vec<glm::tvec3<T, P>>
{
	static const bool value = true;
};

template <typename T, glm::precision P>
struct is_vec<glm::tvec4<T, P>>
{
	static const bool value = true;
};

}

}
