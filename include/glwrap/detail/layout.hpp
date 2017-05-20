
#pragma once

#include <type_traits>

#include "traits.hpp"
#include "../vector.hpp"

// TODO: move this out of detail/ ?

namespace gl
{

namespace layout
{

template <typename T, typename Enable = void>
struct std140;

// "If the member is a scalar consuming N basic machine units, the base alignment is N."
template <typename T>
struct alignas(sizeof(T)) std140<T,
	typename std::enable_if<std::is_scalar<T>::value>::type>
{
	T value;
};

// "If the member is a two- or four-component vector
// with components consuming N basic machine units,
// the base alignment is 2N or 4N, respectively."
template <typename T>
struct alignas(sizeof(T)) std140<T,
	typename std::enable_if<detail::is_vec<T>::value
	&& (2 == detail::vec_traits<T>::dimensions || 4 == detail::vec_traits<T>::dimensions)>::type>
{
	T value;
};

// "If the member is a three-component vector
// with components consuming N basic machine units, the base alignment is 4N."
template <typename T>
struct alignas(sizeof(T) / 3 * 4) std140<T,
	typename std::enable_if<detail::is_vec<T>::value
	&& (3 == detail::vec_traits<T>::dimensions)>::type>
{
	T value;
};

}

}