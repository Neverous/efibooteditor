// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* casts */
#if defined(__cplusplus)
#define STATIC_CAST(type) static_cast<type>
#else
#define STATIC_CAST(type) (type)
#define nullptr NULL
#endif

/* attributes */
#if defined(__GNUC__) && !defined(__clang__)
#define ATTR_ARTIFICIAL __attribute__((__artificial__))
#else
#define ATTR_ARTIFICIAL
#endif

/* MSVC compatibility */
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

/* Windows compatibility */
#if defined(_WIN32)
#include <Windows.h>
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
#define ANYSIZE_ARRAY 1
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#pragma clang diagnostic ignored "-Wreserved-identifier"
#pragma clang diagnostic ignored "-Wreserved-macro-identifier"
#endif
#define _T
#define _TRUNCATE STATIC_CAST(size_t)(-1)
inline int _tcsncpy_s(TCHAR *buffer, size_t size, TCHAR *src, size_t _size)
{
    (void)_size;
    return !buffer || *strncpy(buffer, src, size - 1) == 0;
}

inline int _tcserror_s(TCHAR *buffer, size_t size, int errnum)
{
#if defined(__APPLE__) || ((_POSIX_C_SOURCE >= 200112L) && !defined(_GNU_SOURCE))
    return strerror_r(errnum, buffer, size);
#else
    TCHAR *msg = strerror_r(errnum, buffer, size);
    if(msg == nullptr)
        return 0;

    return _tcsncpy_s(buffer, size, msg, _TRUNCATE);
#endif
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif
#define _sntprintf_s(buffer, buffer_size, count, format, ...) snprintf(buffer, buffer_size, format, __VA_ARGS__)
#endif

#if defined(__cplusplus)
#include <QString>
#include <cctype>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
#include <QStringConverter>
#else
#include <QTextCodec>
#endif

/* string types */
using tstring = std::basic_string<TCHAR>;

using tstring_view = std::basic_string_view<TCHAR>;

using tostream = std::basic_ostream<TCHAR>;
using tistream = std::basic_istream<TCHAR>;
using tiostream = std::basic_iostream<TCHAR>;

using tifstream = std::basic_ifstream<TCHAR>;
using tofstream = std::basic_ofstream<TCHAR>;
using tfstream = std::basic_fstream<TCHAR>;

using tstringstream = std::basic_stringstream<TCHAR>;

template <class Type>
inline tstring to_tstring(const Type &value)
{
#if defined(UNICODE) || defined(_UNICODE)
    return std::to_wstring(value);
#else
    return std::to_string(value);
#endif
}

const int HEX_BASE = 16;

/* QString helpers */
inline tstring QStringToStdTString(const QString &string)
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

inline QString QStringFromStdTString(const tstring &string)
{
#if defined(UNICODE) || defined(_UNICODE)
    return QString::fromStdWString(string);
#else
    return QString::fromStdString(string);
#endif
}

/* additional helpers */
inline QString toHex(unsigned long long number, int min_width = 0, const QString &prefix = "0x")
{
    return prefix + QString("%1").arg(number, min_width, HEX_BASE, QChar('0')).toUpper();
}

inline bool isxnumber(const tstring_view &string)
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

template <class Container>
inline bool toUnicode(QString &output, const Container &input, const char *codec_name = "UTF-8")
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    QStringDecoder decoder{codec_name};
    output = decoder.decode(input);
    return !decoder.hasError();
#else
    auto codec = QTextCodec::codecForName(codec_name);
    QTextCodec::ConverterState state;
    output = codec->toUnicode(reinterpret_cast<const char *>(std::data(input)), static_cast<int>(std::size(input)), &state);
    return state.invalidChars == 0;
#endif
}

inline QByteArray fromUnicode(const QString &input, const char *codec_name = "UTF-8")
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    QStringEncoder encoder(codec_name);
    return encoder.encode(input);
#else
    std::unique_ptr<QTextEncoder> encoder{QTextCodec::codecForName(codec_name)->makeEncoder(QTextCodec::IgnoreHeader)};
    return encoder->fromUnicode(input);
#endif
}

template <class Type>
struct type_identity
{
    using type = Type;
};

// Like std::underlying_type but also works for non enums
template <class Type>
using underlying_type_t = typename std::conditional_t<std::is_enum_v<Type>, std::underlying_type<Type>, type_identity<Type>>::type;

// like std::add_const but forces const also for pointer types
template <class Type>
using add_const_t = typename std::conditional_t<std::is_pointer_v<Type>, std::add_pointer_t<std::add_const_t<std::remove_pointer_t<Type>>>, std::add_const_t<Type>>;
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#endif
inline const void *advance_bytes(const void *ptr, size_t bytes)
{
    return STATIC_CAST(const void *)(STATIC_CAST(const uint8_t *)(ptr) + bytes);
}
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
