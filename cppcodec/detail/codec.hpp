/**
 *  Copyright (C) 2015 Topology LP
 *  All rights reserved.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

#ifndef CPPCODEC_DETAIL_CODEC
#define CPPCODEC_DETAIL_CODEC

#include <assert.h>
#include <stdint.h>
#include <string>
#include <vector>

#include "../data/access.hpp"
#include "../data/raw_result_buffer.hpp"
#include "cppcodec/parse_error.hpp"

namespace cppcodec {
namespace detail {

// SFINAE: Templates sometimes beat sensible overloads - make sure we don't call the wrong one.
template <typename T>
struct non_numeric : std::enable_if<!std::is_arithmetic<T>::value> { };

template <typename T>
struct non_error : std::enable_if<!std::is_same<T, error>::value> { };


/**
 * Public interface for all the codecs. For API documentation, see README.md.
 */
template <typename CodecImpl>
class codec
{
public:
    //
    // Encoding

    // Convenient version, returns an std::string.
    static std::string encode(const uint8_t* binary, size_t binary_size, error &error) noexcept;
    static std::string encode(const char* binary, size_t binary_size, error &error) noexcept;

    // Convenient version with templated result type.
    template <typename Result> static Result encode(const uint8_t* binary, size_t binary_size, error &error) noexcept;
    template <typename Result> static Result encode(const char* binary, size_t binary_size, error &error) noexcept;
    template <typename Result = std::string, typename T = std::vector<uint8_t>>
    static Result encode(const T& binary, error &error) noexcept; 

    // Reused result container version. Resizes encoded_result before writing to it.
    template <typename Result>
    [[nodiscard]] static error encode(Result& encoded_result, const uint8_t* binary, size_t binary_size) noexcept;
    template <typename Result>
    [[nodiscard]] static error encode(Result& encoded_result, const char* binary, size_t binary_size) noexcept;
    template <typename Result, typename T, typename non_numeric<T>::type* = nullptr, typename non_error<T>::type* = nullptr>
    [[nodiscard]] static error encode(Result& encoded_result, const T& binary) noexcept;

    // Raw pointer output, assumes pre-allocated memory with size > encoded_size(binary_size).
    [[nodiscard]] static error encode(
            char* encoded_result, size_t encoded_buffer_size,
            const uint8_t* binary, size_t binary_size,
            size_t &encoded_result_size) noexcept;
    [[nodiscard]] static error encode(
            char* encoded_result, size_t encoded_buffer_size,
            const char* binary, size_t binary_size,
            size_t &encoded_result_size) noexcept;
    template<typename T>
    [[nodiscard]] static error encode(
            char* encoded_result, size_t encoded_buffer_size,
            const T& binary,
            size_t &encoded_result_size) noexcept;

    // Calculate the exact length of the encoded string based on binary size.
    static constexpr size_t encoded_size(size_t binary_size) noexcept;

    //
    // Decoding

    // Convenient version, returns an std::vector<uint8_t>.
    static std::vector<uint8_t> decode(const char* encoded, size_t encoded_size, error &error) noexcept;

    // Convenient version with templated result type.
    template <typename Result> static Result decode(const char* encoded, size_t encoded_size, error &error) noexcept;
    template <typename Result = std::vector<uint8_t>, typename T = std::string>
    static Result decode(const T& encoded, error &error) noexcept;

    // Reused result container version. Resizes binary_result before writing to it.
    template <typename Result>
    [[nodiscard]] static error decode(Result& binary_result, const char* encoded, size_t encoded_size) noexcept;
    template <typename Result, typename T, typename non_numeric<T>::type* = nullptr, typename non_error<T>::type* = nullptr>
    [[nodiscard]] static error decode(Result& binary_result, const T& encoded) noexcept;

    // Raw pointer output, assumes pre-allocated memory with size > decoded_max_size(encoded_size).
    [[nodiscard]] static error decode(
            uint8_t* binary_result, size_t binary_buffer_size,
            const char* encoded, size_t encoded_size,
            size_t &binary_result_size) noexcept;
    [[nodiscard]] static error decode(
            char* binary_result, size_t binary_buffer_size,
            const char* encoded, size_t encoded_size,
            size_t &binary_result_size) noexcept;
    template<typename T, typename non_error<T>::type* = nullptr>
    [[nodiscard]] static error decode(
            uint8_t* binary_result, size_t binary_buffer_size, const T& encoded, size_t &binary_result_size) noexcept;
    template<typename T, typename non_error<T>::type* = nullptr>
    [[nodiscard]] static error decode(
            char* binary_result, size_t binary_buffer_size, const T& encoded, size_t &binary_result_size) noexcept;

    // Calculate the maximum size of the decoded binary buffer based on the encoded string length.
    static constexpr size_t decoded_max_size(size_t encoded_size) noexcept;
};


//
// Inline definitions of the above functions, using CRTP to call into CodecImpl
//

//
// Encoding

template <typename CodecImpl>
inline std::string codec<CodecImpl>::encode(const uint8_t* binary, size_t binary_size, error &error) noexcept
{
    return encode<std::string>(binary, binary_size, error);
}

template <typename CodecImpl>
inline std::string codec<CodecImpl>::encode(const char* binary, size_t binary_size, error &error) noexcept
{
    return encode<std::string>(reinterpret_cast<const uint8_t*>(binary), binary_size, error);
}

template <typename CodecImpl>
template <typename Result>
inline Result codec<CodecImpl>::encode(const uint8_t* binary, size_t binary_size, error &error) noexcept
{
    Result encoded_result;
    error = encode(encoded_result, binary, binary_size);
    return encoded_result;
}

template <typename CodecImpl>
template <typename Result>
inline Result codec<CodecImpl>::encode(const char* binary, size_t binary_size, error &error) noexcept
{
    return encode<Result>(reinterpret_cast<const uint8_t*>(binary), binary_size, error);
}

template <typename CodecImpl>
template <typename Result, typename T>
inline Result codec<CodecImpl>::encode(const T& binary, error &error) noexcept
{
    return encode<Result>(data::uchar_data(binary), data::size(binary), error);
}

template <typename CodecImpl>
template <typename Result>
inline error codec<CodecImpl>::encode(
    Result& encoded_result, const uint8_t* binary, size_t binary_size) noexcept
{
    // This overload is where we reserve buffer capacity and call into CodecImpl.
    size_t encoded_buffer_size = encoded_size(binary_size);
    auto state = data::create_state(encoded_result, data::specific_t());
    data::init(encoded_result, state, encoded_buffer_size);

    CodecImpl::encode(encoded_result, state, binary, binary_size);
    data::finish(encoded_result, state);
    assert(data::size(encoded_result) == encoded_buffer_size);
    return {};
}

template <typename CodecImpl>
template <typename Result>
inline error codec<CodecImpl>::encode(
    Result& encoded_result, const char* binary, size_t binary_size) noexcept
{
    return encode(encoded_result, reinterpret_cast<const uint8_t*>(binary), binary_size);
}

template <typename CodecImpl>
template <typename Result, typename T, typename non_numeric<T>::type*, typename non_error<T>::type*>
inline error codec<CodecImpl>::encode(Result& encoded_result, const T& binary) noexcept
{
    return encode(encoded_result, data::uchar_data(binary), data::size(binary));
}

template <typename CodecImpl>
inline error codec<CodecImpl>::encode(
        char* encoded_result, size_t encoded_buffer_size,
        const uint8_t* binary, size_t binary_size,
        size_t &encoded_size) noexcept
{
    // This overload is where we wrap the result pointer & size.
    data::raw_result_buffer encoded(encoded_result, encoded_buffer_size);
    encode(encoded, binary, binary_size);

    encoded_size = data::size(encoded);
    if (encoded_size < encoded_buffer_size) {
        encoded_result[encoded_size] = '\0';
    }
    return {};
}

template <typename CodecImpl>
inline error codec<CodecImpl>::encode(
        char* encoded_result, size_t encoded_buffer_size,
        const char* binary, size_t binary_size,
        size_t &encoded_size) noexcept
{
    // This overload is where we wrap the result pointer & size.
    return encode(encoded_result, encoded_buffer_size,
            reinterpret_cast<const uint8_t*>(binary), binary_size, encoded_size);
}

template <typename CodecImpl>
template <typename T>
inline error codec<CodecImpl>::encode(
        char* encoded_result, size_t encoded_buffer_size,
        const T& binary,
        size_t &encoded_size) noexcept
{
    return encode(encoded_result, encoded_buffer_size, data::uchar_data(binary), data::size(binary), encoded_size);
}

template <typename CodecImpl>
inline constexpr size_t codec<CodecImpl>::encoded_size(size_t binary_size) noexcept
{
    return CodecImpl::encoded_size(binary_size);
}


//
// Decoding

template <typename CodecImpl>
inline std::vector<uint8_t> codec<CodecImpl>::decode(const char* encoded, size_t encoded_size, error &error) noexcept
{
    return decode<std::vector<uint8_t>>(encoded, encoded_size, error);
}

template <typename CodecImpl>
template <typename Result>
inline Result codec<CodecImpl>::decode(const char* encoded, size_t encoded_size, error &error) noexcept
{
    Result result;
    error = decode(result, encoded, encoded_size);
    return result;
}

template <typename CodecImpl>
template <typename Result, typename T>
inline Result codec<CodecImpl>::decode(const T& encoded, error &error) noexcept
{
    return decode<Result>(data::char_data(encoded), data::size(encoded), error);
}

template <typename CodecImpl>
template <typename Result>
inline error codec<CodecImpl>::decode(Result& binary_result, const char* encoded, size_t encoded_size) noexcept
{
    // This overload is where we reserve buffer capacity and call into CodecImpl.
    size_t binary_buffer_size = decoded_max_size(encoded_size);
    auto state = data::create_state(binary_result, data::specific_t());
    data::init(binary_result, state, binary_buffer_size);

    auto error = CodecImpl::decode(binary_result, state, encoded, encoded_size);
    data::finish(binary_result, state);
    assert(data::size(binary_result) <= binary_buffer_size);
    return error;
}


template <typename CodecImpl>
template <typename Result, typename T, typename non_numeric<T>::type*, typename non_error<T>::type*>
inline error codec<CodecImpl>::decode(Result& binary_result, const T& encoded) noexcept
{
    return decode(binary_result, data::char_data(encoded), data::size(encoded));
}

template <typename CodecImpl>
inline error codec<CodecImpl>::decode(
        uint8_t* binary_result, size_t binary_buffer_size,
        const char* encoded, size_t encoded_size,
        size_t &binary_result_size) noexcept
{
    return decode(reinterpret_cast<char*>(binary_result), binary_buffer_size, encoded, encoded_size, binary_result_size);
}

template <typename CodecImpl>
inline error codec<CodecImpl>::decode(
        char* binary_result, size_t binary_buffer_size,
        const char* encoded, size_t encoded_size,
        size_t &binary_result_size) noexcept
{
    // This overload is where we wrap the result pointer & size.
    data::raw_result_buffer binary(binary_result, binary_buffer_size);
    auto error = decode(binary, encoded, encoded_size);
    binary_result_size = data::size(binary);
    return error;
}

template <typename CodecImpl>
template <typename T, typename non_error<T>::type*>
inline error codec<CodecImpl>::decode(
        uint8_t* binary_result, size_t binary_buffer_size, const T& encoded, size_t &binary_result_size) noexcept
{
    return decode(reinterpret_cast<char*>(binary_result), binary_buffer_size, encoded, binary_result_size);
}

template <typename CodecImpl>
template <typename T, typename non_error<T>::type*>
inline error codec<CodecImpl>::decode(char* binary_result, size_t binary_buffer_size, const T& encoded, size_t &binary_result_size) noexcept
{
    return decode(binary_result, binary_buffer_size, data::char_data(encoded), data::size(encoded), binary_result_size);
}

template <typename CodecImpl>
inline constexpr size_t codec<CodecImpl>::decoded_max_size(size_t encoded_size) noexcept
{
    return CodecImpl::decoded_max_size(encoded_size);
}


} // namespace detail
} // namespace cppcodec

#endif
