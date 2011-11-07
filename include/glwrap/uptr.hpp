#pragma once

#include <memory>

// TODO: stupid

#define uptr std::unique_ptr

template <typename T>
typename std::enable_if<std::is_array<T>::value, uptr<T>>::type unew(std::size_t _size)
{
	return uptr<T>(new typename uptr<T>::element_type[_size]());
}

template <typename T, typename... Args>
typename std::enable_if<!std::is_array<T>::value, uptr<T>>::type unew(Args... _args)
{
	return uptr<T>(new T(_args...));
}

