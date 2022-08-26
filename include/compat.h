// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <stdbool.h>

/* attributes */
#if defined(__GNUC__) && !defined(__clang__)
#define ATTR_ARTIFICIAL __attribute__((__artificial__))
#else
#define ATTR_ARTIFICIAL
#endif

#if defined(_MSC_VER)
#include <basetsd.h>
#define ATTR_ALIGN(X) __declspec(align(X))
#define ATTR_NONNULL(...)
#define ATTR_NONNULL_IS_NULL(x) !(x)
#define ATTR_UNUSED
#define ATTR_VISIBILITY(...)
#define ATTR_WARN_UNUSED_RESULT _Check_return_
typedef SSIZE_T ssize_t;
#else
#define ATTR_ALIGN(X) __attribute__((__aligned__(X)))
#define ATTR_NONNULL(...) __attribute__((__nonnull__(__VA_ARGS__)))
#define ATTR_NONNULL_IS_NULL(x) false
#define ATTR_UNUSED __attribute__((__unused__))
#define ATTR_VISIBILITY(...) __attribute__((__visibility__(__VA_ARGS__)))
#define ATTR_WARN_UNUSED_RESULT __attribute__((__warn_unused_result__))
#endif

#ifdef _WIN32
#include <Windows.h>
#include <stdint.h>
#include <stdio.h>
#include <tchar.h>
#undef interface
typedef uint32_t mode_t;
#else
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
typedef char TCHAR;
#define _T
inline int _tcserror_s(TCHAR *buffer, size_t size, int errnum)
{
#if defined(__APPLE__) || ((_POSIX_C_SOURCE >= 200112L) && !_GNU_SOURCE)
    return strerror_r(errnum, buffer, size);
#else
    return strerror_r(errnum, buffer, size) == NULL;
#endif
}

#define _sntprintf_s(buffer, buffer_size, count, format, ...) snprintf(buffer, buffer_size, format, __VA_ARGS__)
#endif

#ifdef __cplusplus
#include <QString>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>

namespace std
{
typedef basic_string<TCHAR> tstring;

typedef basic_string_view<TCHAR> tstring_view;

typedef basic_ostream<TCHAR> tostream;
typedef basic_istream<TCHAR> tistream;
typedef basic_iostream<TCHAR> tiostream;

typedef basic_ifstream<TCHAR> tifstream;
typedef basic_ofstream<TCHAR> tofstream;
typedef basic_fstream<TCHAR> tfstream;

typedef basic_stringstream<TCHAR> tstringstream;

template <class Type>
inline tstring to_tstring(const Type &value)
{
#if defined(UNICODE) || defined(_UNICODE)
    return to_wstring(value);
#else
    return to_string(value);
#endif
}

}

#ifndef DEFINE_ENUM_FLAG_OPERATORS
#define DEFINE_ENUM_FLAG_OPERATORS(T)                                                                                                                      \
    inline T operator~(T a) { return static_cast<T>(~static_cast<std::underlying_type<T>::type>(a)); }                                                     \
    inline T operator|(T a, T b) { return static_cast<T>(static_cast<std::underlying_type<T>::type>(a) | static_cast<std::underlying_type<T>::type>(b)); } \
    inline T operator&(T a, T b) { return static_cast<T>(static_cast<std::underlying_type<T>::type>(a) & static_cast<std::underlying_type<T>::type>(b)); }
#endif

const int HEX_BASE = 16;

QT_BEGIN_NAMESPACE

inline std::tstring QStringToStdTString(const QString &string)
{
#if defined(UNICODE) || defined(_UNICODE)
    return string.toStdWString();
#else
    return string.toStdString();
#endif
}

inline QString QStringFromTCharArray(const TCHAR *string)
{
#if defined(UNICODE) || defined(_UNICODE)
    return QString::fromWCharArray(string);
#else
    return string;
#endif
}

inline QString QStringFromStdTString(const std::tstring &string)
{
#if defined(UNICODE) || defined(_UNICODE)
    return QString::fromStdWString(string);
#else
    return QString::fromStdString(string);
#endif
}

inline QString toHex(qulonglong number, int min_width = 0, const QString &prefix = "0x")
{
    return prefix + QString("%1").arg(number, min_width, HEX_BASE, QChar('0')).toUpper();
}

QT_END_NAMESPACE

inline bool isxnumber(const std::tstring_view &string)
{
#if defined(UNICODE) || defined(_UNICODE)
    return std::all_of(std::begin(string), std::end(string), [](char16_t chr)
        { return iswxdigit(chr); });
#else
    return std::all_of(std::begin(string), std::end(string), [](unsigned char chr)
        { return isxdigit(chr); });
#endif
}

template <class Container>
auto get_default(const Container &data, const typename Container::key_type &key, const typename Container::mapped_type &default_value) -> typename Container::mapped_type
{
    if(auto it = data.find(key); it != data.end())
        return it->second;

    return default_value;
}
#endif
