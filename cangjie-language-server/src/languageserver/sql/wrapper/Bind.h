// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#pragma once

#include "SQLiteAPI.h"
#include "Traits.h"

#include <array>
#include <cstddef>
#include <cstring>
#include <string>
#include <type_traits>
#include <vector>

#if __cplusplus > 201402L
#if defined(__has_include)
#if __has_include(<filesystem> )
#define SQLDB_HAS_FILESYSTEM
#include <filesystem>
#endif // __has_include(<filesystem>)
#endif // defined(__has_include)
#include <optional>
#endif // __cplusplus > 201402L

namespace sqldb {
namespace traits {

template <typename, typename = void>
struct Bind;

template <typename T>
int bind(sqlite3_stmt *S, int &I, const T &V)
{
    return Bind<traits::remove_cvref_t<T>>::call(S, I, V);
}

template <typename T>
int bind(sqlite3_stmt *S, int &I, const T &V, sqlite3_destructor_type D)
{
    return Bind<traits::remove_cvref_t<T>>::call(S, I, V, D);
}

template <typename T>
struct Bind<const T *> : public Bind<T *> {};

template <typename T>
struct Bind<T, std::enable_if_t<std::is_integral<T>::value>> {
    static int call(sqlite3_stmt *S, int I, T V) { return sqlite::bind_int64(S, I, V); }
};

template <typename T>
struct Bind<T, std::enable_if_t<std::is_enum<T>::value>> {
    static int call(sqlite3_stmt *S, int I, T V)
    {
        return traits::bind(S, I, static_cast<std::underlying_type_t<T>>(V));
    }
};

template <typename T>
struct Bind<T, std::enable_if_t<std::is_floating_point<T>::value>> {
    static int call(sqlite3_stmt *S, int I, T V) { return sqlite::bind_double(S, I, V); }
};

template <>
struct Bind<std::string> {
    static int call(sqlite3_stmt *S, int I, const std::string &V, sqlite3_destructor_type D = sqlite::Transient)
    {
        return sqlite::bind_text64(S, I, V.data(), V.size(), D, sqlite::UTF8);
    }
};

template <std::size_t N>
struct Bind<char[N]> {
    static int call(sqlite3_stmt *S, int I, const char (&V)[N], sqlite3_destructor_type D = sqlite::Transient)
    {
        return sqlite::bind_text64(S, I, V, std::strlen(V), D, sqlite::UTF8);
    }
};

template <>
struct Bind<char *> {
    static int call(sqlite3_stmt *S, int I, const char *V, sqlite3_destructor_type D = sqlite::Transient)
    {
        return sqlite::bind_text64(S, I, V, std::strlen(V), D, sqlite::UTF8);
    }
};

template <typename T, std::size_t N>
struct Bind<std::array<T, N>, std::enable_if_t<is_scalar<T>::value>> {
    static int call(sqlite3_stmt *S, int I, const std::array<T, N> &V, sqlite3_destructor_type D = sqlite::Transient)
    {
        return sqlite::bind_blob64(S, I, V.data(), V.size() * sizeof(T), D);
    }
};

template <typename T, std::size_t N>
struct Bind<T[N], std::enable_if_t<is_scalar<T>::value>> {
    static int call(sqlite3_stmt *S, int I, const T (&V)[N], sqlite3_destructor_type D = sqlite::Transient)
    {
        return sqlite::bind_blob64(S, I, V, N * sizeof(T), D);
    }
};

template <typename T>
struct Bind<std::vector<T>, std::enable_if_t<is_scalar<T>::value>> {
    static int call(sqlite3_stmt *S, int I, const std::vector<T> &V, sqlite3_destructor_type D = sqlite::Transient)
    {
        return sqlite::bind_blob64(S, I, V.data(), V.size() * sizeof(T), D);
    }
};

template <>
struct Bind<std::nullptr_t> {
    static int call(sqlite3_stmt *S, int I, std::nullptr_t) { return sqlite::bind_null(S, I); }
};

#if __cplusplus > 201402L

#ifdef SQLDB_HAS_FILESYSTEM

template <>
struct Bind<std::filesystem::path> {
    static int call(sqlite3_stmt *S, int I, const std::filesystem::path &V)
    {
        return traits::bind(S, I, V.generic_u8string(), sqlite::Transient);
    }
};

#endif // SQLDB_HAS_FILESYSTEM

template <>
struct Bind<std::string_view> {
    static int call(sqlite3_stmt *S, int I, std::string_view V, sqlite3_destructor_type D = sqlite::Transient)
    {
        return sqlite::bind_text64(S, I, V.data(), V.size(), D, sqlite::UTF8);
    }
};

template <>
struct Bind<std::nullopt_t> {
    static int call(sqlite3_stmt *S, int I, std::nullopt_t) { return sqlite::bind_null(S, I); }
};

template <typename T>
struct Bind<std::optional<T>> {
    static int call(sqlite3_stmt *S, int I, const std::optional<T> &V)
    {
        return V ? traits::bind(S, I, *V) : traits::bind(S, I, std::nullopt);
    }
};

#endif // __cplusplus > 201402L

#if __cplusplus > 201703L

#if defined(__cpp_lib_char8_t)

inline const char *as_chars(const char8_t *S) noexcept { return reinterpret_cast<const char *>(S); }

template <>
struct Bind<std::u8string> {
    static int call(sqlite3_stmt *S, int I, const std::u8string &V, sqlite3_destructor_type D = sqlite::Transient)
    {
        return sqlite::bind_text64(S, I, as_chars(V.data()), V.size(), D, sqlite::UTF8);
    }
};

template <>
struct Bind<std::u8string_view> {
    static int call(sqlite3_stmt *S, int I, std::u8string_view V, sqlite3_destructor_type D = sqlite::Transient)
    {
        return sqlite::bind_text64(S, I, as_chars(V.data()), V.size(), D, sqlite::UTF8);
    }
};

#endif // defined(__cpp_lib_char8_t)

#endif // __cplusplus > 201703L

} // namespace traits
} // namespace sqldb