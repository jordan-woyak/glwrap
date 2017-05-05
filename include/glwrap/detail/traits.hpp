
#pragma once

namespace GLWRAP_NAMESPACE
{

namespace detail
{

template <typename T>
struct is_vec : std::false_type
{};

template <typename T, glm::precision P>
struct is_vec<glm::tvec2<T, P>> : std::true_type
{};

template <typename T, glm::precision P>
struct is_vec<glm::tvec3<T, P>> : std::true_type
{};

template <typename T, glm::precision P>
struct is_vec<glm::tvec4<T, P>> : std::true_type
{};


template <typename T>
struct is_mat : std::false_type
{};

template <typename T, glm::precision P>
struct is_mat<glm::tmat2x2<T, P>> : std::true_type
{};

template <typename T, glm::precision P>
struct is_mat<glm::tmat2x3<T, P>> : std::true_type
{};

template <typename T, glm::precision P>
struct is_mat<glm::tmat2x4<T, P>> : std::true_type
{};

template <typename T, glm::precision P>
struct is_mat<glm::tmat3x2<T, P>> : std::true_type
{};

template <typename T, glm::precision P>
struct is_mat<glm::tmat3x3<T, P>> : std::true_type
{};

template <typename T, glm::precision P>
struct is_mat<glm::tmat3x4<T, P>> : std::true_type
{};

template <typename T, glm::precision P>
struct is_mat<glm::tmat4x2<T, P>> : std::true_type
{};

template <typename T, glm::precision P>
struct is_mat<glm::tmat4x3<T, P>> : std::true_type
{};

template <typename T, glm::precision P>
struct is_mat<glm::tmat4x4<T, P>> : std::true_type
{};

}

}
