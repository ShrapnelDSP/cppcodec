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

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <cppcodec/base32_crockford.hpp>
#include <cppcodec/base32_hex.hpp>
#include <cppcodec/base32_rfc4648.hpp>
#include <cppcodec/base64_rfc4648.hpp>
#include <cppcodec/base64_url.hpp>
#include <cppcodec/base64_url_unpadded.hpp>
#include <cppcodec/hex_lower.hpp>
#include <cppcodec/hex_upper.hpp>
#include <stdint.h>
#include <string.h> // for memcmp()
#include <vector>

TEST_CASE("Douglas Crockford's base32", "[base32][crockford]") {
    using base32 = cppcodec::base32_crockford;

    SECTION("encoded size calculation") {
        REQUIRE(base32::encoded_size(0) == 0);
        REQUIRE(base32::encoded_size(1) == 2);
        REQUIRE(base32::encoded_size(2) == 4);
        REQUIRE(base32::encoded_size(3) == 5);
        REQUIRE(base32::encoded_size(4) == 7);
        REQUIRE(base32::encoded_size(5) == 8);
        REQUIRE(base32::encoded_size(6) == 10);
        REQUIRE(base32::encoded_size(10) == 16);
    }

    SECTION("maximum decoded size calculation") {
        REQUIRE(base32::decoded_max_size(0) == 0);
        REQUIRE(base32::decoded_max_size(1) == 0);
        REQUIRE(base32::decoded_max_size(2) == 1);
        REQUIRE(base32::decoded_max_size(3) == 1);
        REQUIRE(base32::decoded_max_size(4) == 2);
        REQUIRE(base32::decoded_max_size(5) == 3);
        REQUIRE(base32::decoded_max_size(6) == 3);
        REQUIRE(base32::decoded_max_size(7) == 4);
        REQUIRE(base32::decoded_max_size(8) == 5);
        REQUIRE(base32::decoded_max_size(9) == 5);
        REQUIRE(base32::decoded_max_size(10) == 6);
        REQUIRE(base32::decoded_max_size(16) == 10);
    }

    std::string hello = "Hello World";
    std::string hello_encoded = "91JPRV3F41BPYWKCCG";
    std::string hello_encoded_null = "91JPRV3F41BPYWKCCG00";

    const uint8_t* hello_uint_ptr = reinterpret_cast<const uint8_t*>(hello.data());
    const uint8_t* hello_uint_ptr_encoded = reinterpret_cast<const uint8_t*>(hello_encoded.data());

    std::vector<uint8_t> hello_uint_vector(hello_uint_ptr, hello_uint_ptr + hello.size());
    std::vector<char> hello_char_vector_encoded(
            hello_encoded.data(), hello_encoded.data() + hello_encoded.size());
    std::vector<uint8_t> hello_uint_vector_encoded(
            hello_uint_ptr_encoded, hello_uint_ptr_encoded + hello_encoded.size());

    SECTION("encoding data") {
        cppcodec::error error;
        REQUIRE(base32::encode(std::vector<uint8_t>(), error, 0) == "");
        REQUIRE(base32::encode(std::vector<uint8_t>({0}), error, 0) == "00");
        REQUIRE(base32::encode(std::vector<uint8_t>({0, 0}), error, 0) == "0000");
        REQUIRE(base32::encode(std::vector<uint8_t>({0, 0, 0}), error, 0) == "00000");
        REQUIRE(base32::encode(std::vector<uint8_t>({0, 0, 0, 0}), error, 0) == "0000000");
        REQUIRE(base32::encode(std::vector<uint8_t>({0, 0, 0, 0, 0}), error, 0) == "00000000");
        REQUIRE(base32::encode(std::vector<uint8_t>({0, 0, 0, 0, 0, 0}), error, 0) == "0000000000");

        // Constructing an std::string reduces the size of the char array by one (null terminator).
        // Therefore, the result for passing the string literal directly ends up encoding
        // one more character, which produces two more symbols in this particular case.
        REQUIRE(base32::encode(std::string("Hello World"), error, 0) == hello_encoded);
        REQUIRE(base32::encode("Hello World", error, 0) == hello_encoded_null);

        REQUIRE(base32::encode(std::string("foo"), error, 0) == "CSQPY");
        REQUIRE(base32::encode(std::string("lowercase UPPERCASE 1434567 !@#$%^&*"), error, 0)
                == "DHQQESBJCDGQ6S90AN850HAJ8D0N6H9064T36D1N6RVJ08A04CJ2AQH658");
        REQUIRE(base32::encode(std::string("Wow, it really works!"), error, 0) == "AXQQEB10D5T20WK5C5P6RY90EXQQ4TVK44");
    }

    SECTION("decoding data") {
        cppcodec::error error;
        REQUIRE(base32::decode("", error) == std::vector<uint8_t>());
        REQUIRE(base32::decode("00", error) == std::vector<uint8_t>({0}));
        REQUIRE(base32::decode("0000", error) == std::vector<uint8_t>({0, 0}));
        REQUIRE(base32::decode("00000", error) == std::vector<uint8_t>({0, 0, 0}));
        REQUIRE(base32::decode("0000000", error) == std::vector<uint8_t>({0, 0, 0, 0}));
        REQUIRE(base32::decode("00000000", error) == std::vector<uint8_t>({0, 0, 0, 0, 0}));
        REQUIRE(base32::decode("0000000000", error) == std::vector<uint8_t>({0, 0, 0, 0, 0, 0}));

        // For decoding data, the result should be the same whether or not there is
        // a null terminator at the end, because the input is a string (not binary array).
        REQUIRE(base32::decode(std::string("91JPRV3F41BPYWKCCG"), error) == hello_uint_vector);
        REQUIRE(base32::decode("91JPRV3F41BPYWKCCG", error) == hello_uint_vector);

        REQUIRE(base32::decode<std::string>("CSQPY", error) == "foo");
        REQUIRE(base32::decode<std::string>("DHQQESBJCDGQ6S90AN850HAJ8D0N6H9064T36D1N6RVJ08A04CJ2AQH658", error)
                == "lowercase UPPERCASE 1434567 !@#$%^&*");

        // Lowercase should decode just as well as uppercase.
        REQUIRE(base32::decode<std::string>("AXQQEB10D5T20WK5C5P6RY90EXQQ4TVK44", error) == "Wow, it really works!");
        REQUIRE(base32::decode<std::string>("axqqeb10d5t20wk5c5p6ry90exqq4tvk44", error) == "Wow, it really works!");

        // Dashes are allowed for visual separation and ignored when decoding.
        REQUIRE(base32::decode<std::string>("-C-SQ--PY-", error) == "foo");

        // An invalid number of symbols should throw the right kind of parse_error.
        error = std::monostate();
        base32::decode("0", error);
        REQUIRE(std::holds_alternative<cppcodec::invalid_input_length_error_value>(error));
        
        error = std::monostate();
        base32::decode("000", error);
        REQUIRE(std::holds_alternative<cppcodec::invalid_input_length_error_value>(error));
        
        error = std::monostate();
        base32::decode("000000", error);
        REQUIRE(std::holds_alternative<cppcodec::invalid_input_length_error_value>(error));

        error = std::monostate();
        base32::decode("000000000", error);
        REQUIRE(std::holds_alternative<cppcodec::invalid_input_length_error_value>(error));

        // An invalid symbol should throw a symbol error.
        error = std::monostate();
        base32::decode("00======", error); // no padding for Crockford
        REQUIRE(std::holds_alternative<cppcodec::symbol_error_value>(error));

        error = std::monostate();
        base32::decode("Uu", error); // only a checksum symbol here
        REQUIRE(std::holds_alternative<cppcodec::symbol_error_value>(error));

        error = std::monostate();
        base32::decode("++", error); // make sure it's not base64
        REQUIRE(std::holds_alternative<cppcodec::symbol_error_value>(error));

        error = std::monostate();
        base32::decode("//", error); // ...ditto
        REQUIRE(std::holds_alternative<cppcodec::symbol_error_value>(error));
    }

    // Only test overloads once (for base32_crockford, since it happens to be the first one).
    // Since it's all templated, we assume that overloads work/behave similarly for other codecs.
    SECTION("encode() overloads") {
        cppcodec::error error;
        
        // Other convenient overloads for taking raw pointer input.
        REQUIRE(base32::encode(hello.data(), hello.size(), error) == hello_encoded);
        REQUIRE(base32::encode(hello_uint_ptr, hello.size(), error) == hello_encoded);

        // Reused result pointer. Put the extra null terminator version in the middle to test resizing.
        std::string result;
        REQUIRE((base32::encode(result, hello_uint_ptr, hello.size()), result) == hello_encoded);
        REQUIRE((base32::encode(result, "Hello World"), result) == hello_encoded_null);
        REQUIRE((base32::encode(result, hello.data(), hello.size()), result) == hello_encoded);

        // Templated result. Use std::vector<uint8_t> to exercise non-char array types.
        REQUIRE(base32::encode<std::vector<uint8_t>>(hello, error, 0) == hello_uint_vector_encoded);
        REQUIRE(base32::encode<std::vector<uint8_t>>(hello.data(), hello.size(), error) == hello_uint_vector_encoded);
        REQUIRE(base32::encode<std::vector<uint8_t>>(hello_uint_ptr, hello.size(), error) == hello_uint_vector_encoded);

        // Raw pointer output.
        std::vector<char> hello_char_result;
        hello_char_result.resize(base32::encoded_size(hello.size()));
        REQUIRE(hello_char_result.size() == hello_char_vector_encoded.size());

        size_t result_size;
        error = base32::encode(hello_char_result.data(), hello_char_result.size(), hello, result_size);
        REQUIRE(result_size == hello_char_vector_encoded.size());
        REQUIRE(hello_char_result == hello_char_vector_encoded);

        error = base32::encode(
                hello_char_result.data(), hello_char_result.size(), hello.data(), hello.size(), result_size);
        REQUIRE(result_size == hello_char_vector_encoded.size());
        REQUIRE(hello_char_result == hello_char_vector_encoded);

        // Test that when passed a larger buffer, the null termination character will be written
        // after the last proper symbol. (Also test uint8_t* overload.)
        hello_char_result.resize(hello_char_result.size() + 1);
        hello_char_result[hello_char_result.size() - 1] = 'x';
        error = base32::encode(
                hello_char_result.data(), hello_char_result.size(), hello_uint_ptr, hello.size(), result_size);
        REQUIRE(result_size == hello_char_vector_encoded.size());
        REQUIRE(hello_char_result[hello_char_result.size() - 1] == '\0');
        hello_char_result.resize(hello_char_result.size() - 1);
        REQUIRE(hello_char_result == hello_char_vector_encoded);
    }

    // Only test overloads once (for base32_crockford, since it happens to be the first one).
    // Since it's all templated, we assume that overloads work/behave similarly for other codecs.
    SECTION("decode() overloads") {
        cppcodec::error error;
        
        // Other convenient overloads for taking raw pointer input.
        REQUIRE(base32::decode(hello_encoded.data(), hello_encoded.size(), error) == hello_uint_vector);

        // Reused result pointer. Put a different string in the middle to test resizing.
        std::vector<uint8_t> result;
        REQUIRE((base32::decode(result, hello_encoded.data(), hello_encoded.size()), result)
                == hello_uint_vector);
        REQUIRE((base32::decode(result, "00"), result) == std::vector<uint8_t>({0}));
        REQUIRE((base32::decode(result, hello_encoded), result) == hello_uint_vector);

        // Templated result. Use std::string to exercise non-uint8_t array types.
        REQUIRE(base32::decode<std::string>(hello_encoded, error) == hello);
        REQUIRE(base32::decode<std::string>(hello_uint_vector_encoded, error) == hello);
        REQUIRE(base32::decode<std::string>(hello_encoded.data(), hello_encoded.size(), error) == hello);

        // Raw pointer output.
        std::vector<uint8_t> hello_uint_result;
        std::vector<char> hello_char_result;
        size_t hello_decoded_max_size = base32::decoded_max_size(hello_encoded.size());
        REQUIRE(hello.size() <= hello_decoded_max_size);

        {
        hello_char_result.resize(hello_decoded_max_size);
        size_t result_size = 0;
        error = base32::decode(
                hello_char_result.data(), hello_char_result.size(), hello_encoded, result_size);
        REQUIRE(result_size == hello.size());
        REQUIRE(std::string(hello_char_result.data(), hello_char_result.data() + result_size) == hello);
        }

        {
        hello_char_result.resize(hello_decoded_max_size);
        size_t result_size = 0;
        error = base32::decode(
                hello_char_result.data(), hello_char_result.size(),
                hello_encoded.data(), hello_encoded.size(), result_size);
        REQUIRE(result_size == hello.size());
        REQUIRE(std::string(hello_char_result.data(), hello_char_result.data() + result_size) == hello);
        }

        {
        hello_uint_result.resize(hello_decoded_max_size);
        size_t result_size = 0;
        error = base32::decode(
                hello_uint_result.data(), hello_uint_result.size(), hello_encoded, result_size);
        REQUIRE(result_size == hello.size());
        hello_uint_result.resize(result_size);
        REQUIRE(hello_uint_result == hello_uint_vector);
        }

        {
        hello_uint_result.resize(hello_decoded_max_size);
        size_t result_size = 0;
        error = base32::decode(
                hello_uint_result.data(), hello_uint_result.size(),
                hello_encoded.data(), hello_encoded.size(), result_size);
        REQUIRE(result_size == hello.size());
        hello_uint_result.resize(result_size);
        REQUIRE(hello_uint_result == hello_uint_vector);
        }
    }
}

TEST_CASE("base32hex", "[base32][hex]") {
    using base32 = cppcodec::base32_hex;

    SECTION("encoded size calculation") {
        REQUIRE(base32::encoded_size(0) == 0);
        REQUIRE(base32::encoded_size(1) == 8);
        REQUIRE(base32::encoded_size(2) == 8);
        REQUIRE(base32::encoded_size(3) == 8);
        REQUIRE(base32::encoded_size(4) == 8);
        REQUIRE(base32::encoded_size(5) == 8);
        REQUIRE(base32::encoded_size(6) == 16);
        REQUIRE(base32::encoded_size(10) == 16);
    }

    SECTION("maximum decoded size calculation") {
        REQUIRE(base32::decoded_max_size(0) == 0);
        REQUIRE(base32::decoded_max_size(1) == 0);
        REQUIRE(base32::decoded_max_size(2) == 0);
        REQUIRE(base32::decoded_max_size(3) == 0);
        REQUIRE(base32::decoded_max_size(4) == 0);
        REQUIRE(base32::decoded_max_size(5) == 0);
        REQUIRE(base32::decoded_max_size(6) == 0);
        REQUIRE(base32::decoded_max_size(7) == 0);
        REQUIRE(base32::decoded_max_size(8) == 5);
        REQUIRE(base32::decoded_max_size(9) == 5);
        REQUIRE(base32::decoded_max_size(10) == 5);
        REQUIRE(base32::decoded_max_size(16) == 10);
    }

    SECTION("encoding data") {
        cppcodec::error error;
        REQUIRE(base32::encode(std::vector<uint8_t>(), error, 0) == "");
        REQUIRE(base32::encode(std::vector<uint8_t>({0}), error, 0) == "00======");
        REQUIRE(base32::encode(std::vector<uint8_t>({0, 0}), error, 0) == "0000====");
        REQUIRE(base32::encode(std::vector<uint8_t>({0, 0, 0}), error, 0) == "00000===");
        REQUIRE(base32::encode(std::vector<uint8_t>({0, 0, 0, 0}), error, 0) == "0000000=");
        REQUIRE(base32::encode(std::vector<uint8_t>({0, 0, 0, 0, 0}), error, 0) == "00000000");
        REQUIRE(base32::encode(std::vector<uint8_t>({0, 0, 0, 0, 0, 0}), error, 0) == "0000000000======");

        // Constructing an std::string reduces the size of the char array by one (null terminator).
        // Therefore, the result for passing the string literal directly ends up encoding
        // one more character, which produces two more symbols in this particular case.
        REQUIRE(base32::encode(std::string("Hello"), error, 0) == "91IMOR3F");
        REQUIRE(base32::encode("Hello", error, 0) == "91IMOR3F00======");

        // RFC 4648: 10. Test Vectors
        REQUIRE(base32::encode(std::string(""), error, 0) == "");
        REQUIRE(base32::encode(std::string("f"), error, 0) == "CO======");
        REQUIRE(base32::encode(std::string("fo"), error, 0) == "CPNG====");
        REQUIRE(base32::encode(std::string("foo"), error, 0) == "CPNMU===");
        REQUIRE(base32::encode(std::string("foob"), error, 0) == "CPNMUOG=");
        REQUIRE(base32::encode(std::string("fooba"), error, 0) == "CPNMUOJ1");
        REQUIRE(base32::encode(std::string("foobar"), error, 0) == "CPNMUOJ1E8======");

        // Other test strings.
        REQUIRE(base32::encode(std::vector<uint8_t>({255, 255, 255, 255, 255}), error, 0) == "VVVVVVVV");
    }

    SECTION("decoding data") {
        cppcodec::error error;
        REQUIRE(base32::decode("", error) == std::vector<uint8_t>());
        REQUIRE(base32::decode("00======", error) == std::vector<uint8_t>({0}));
        REQUIRE(base32::decode("0000====", error) == std::vector<uint8_t>({0, 0}));
        REQUIRE(base32::decode("00000===", error) == std::vector<uint8_t>({0, 0, 0}));
        REQUIRE(base32::decode("0000000=", error) == std::vector<uint8_t>({0, 0, 0, 0}));
        REQUIRE(base32::decode("00000000", error) == std::vector<uint8_t>({0, 0, 0, 0, 0}));
        REQUIRE(base32::decode("0000000000======", error) == std::vector<uint8_t>({0, 0, 0, 0, 0, 0}));

        // For decoding data, the result should be the same whether or not there is
        // a null terminator at the end, because the input is a string (not binary array).
        REQUIRE(base32::decode<std::string>(std::string("91IMOR3F"), error) == "Hello");
        REQUIRE(base32::decode<std::string>("91IMOR3F", error) == "Hello");

        // RFC 4648: 10. Test Vectors
        REQUIRE(base32::decode<std::string>("", error) == "");
        REQUIRE(base32::decode<std::string>("CO======", error) == "f");
        REQUIRE(base32::decode<std::string>("CPNG====", error) == "fo");
        REQUIRE(base32::decode<std::string>("CPNMU===", error) == "foo");
        REQUIRE(base32::decode<std::string>("CPNMUOG=", error) == "foob");
        REQUIRE(base32::decode<std::string>("CPNMUOJ1", error) == "fooba");
        REQUIRE(base32::decode<std::string>("CPNMUOJ1E8======", error) == "foobar");

        // Other test strings.
        REQUIRE(base32::decode("VVVVVVVV", error) == std::vector<uint8_t>({255, 255, 255, 255, 255}));

        // Lowercase should decode just as well as uppercase.
        REQUIRE(base32::decode<std::string>("cpnmuoj1", error) == "fooba");
        REQUIRE(base32::decode<std::string>("cPnMuOj1", error) == "fooba");

        // An invalid number of symbols should throw the right kind of parse_error.
        error = std::monostate();
        base32::decode("0", error);
        REQUIRE(std::holds_alternative<cppcodec::padding_error_value>(error));

        error = std::monostate();
        base32::decode("00", error);
        REQUIRE(std::holds_alternative<cppcodec::padding_error_value>(error));

        error = std::monostate();
        base32::decode("00===", error);
        REQUIRE(std::holds_alternative<cppcodec::padding_error_value>(error));

        error = std::monostate();
        base32::decode("0=======", error);
        REQUIRE(std::holds_alternative<cppcodec::invalid_input_length_error_value>(error));

        error = std::monostate();
        base32::decode("000=====", error);
        REQUIRE(std::holds_alternative<cppcodec::invalid_input_length_error_value>(error));

        error = std::monostate();
        base32::decode("000000==", error);
        REQUIRE(std::holds_alternative<cppcodec::invalid_input_length_error_value>(error));

        // An invalid symbol should throw a symbol error.
        error = std::monostate();
        base32::decode("W0======", error);
        REQUIRE(std::holds_alternative<cppcodec::symbol_error_value>(error));

        error = std::monostate();
        base32::decode("X0======", error);
        REQUIRE(std::holds_alternative<cppcodec::symbol_error_value>(error));

        error = std::monostate();
        base32::decode("Y0======", error);
        REQUIRE(std::holds_alternative<cppcodec::symbol_error_value>(error));

        error = std::monostate();
        base32::decode("Z0======", error);
        REQUIRE(std::holds_alternative<cppcodec::symbol_error_value>(error));

        error = std::monostate();
        base32::decode("CPNM UOJ1", error); // no spaces
        REQUIRE(std::holds_alternative<cppcodec::symbol_error_value>(error));

        error = std::monostate();
        base32::decode("CPNM-UOJ1", error); // no dashes
        REQUIRE(std::holds_alternative<cppcodec::symbol_error_value>(error));
    }
}

TEST_CASE("base32 (RFC 4648)", "[base32][rfc4648]") {
    using base32 = cppcodec::base32_rfc4648;

    SECTION("encoded size calculation") {
        REQUIRE(base32::encoded_size(0) == 0);
        REQUIRE(base32::encoded_size(1) == 8);
        REQUIRE(base32::encoded_size(2) == 8);
        REQUIRE(base32::encoded_size(3) == 8);
        REQUIRE(base32::encoded_size(4) == 8);
        REQUIRE(base32::encoded_size(5) == 8);
        REQUIRE(base32::encoded_size(6) == 16);
        REQUIRE(base32::encoded_size(10) == 16);
    }

    SECTION("maximum decoded size calculation") {
        REQUIRE(base32::decoded_max_size(0) == 0);
        REQUIRE(base32::decoded_max_size(1) == 0);
        REQUIRE(base32::decoded_max_size(2) == 0);
        REQUIRE(base32::decoded_max_size(3) == 0);
        REQUIRE(base32::decoded_max_size(4) == 0);
        REQUIRE(base32::decoded_max_size(5) == 0);
        REQUIRE(base32::decoded_max_size(6) == 0);
        REQUIRE(base32::decoded_max_size(7) == 0);
        REQUIRE(base32::decoded_max_size(8) == 5);
        REQUIRE(base32::decoded_max_size(9) == 5);
        REQUIRE(base32::decoded_max_size(10) == 5);
        REQUIRE(base32::decoded_max_size(16) == 10);
    }

    SECTION("encoding data") {
        cppcodec::error error;
        REQUIRE(base32::encode(std::vector<uint8_t>(), error, 0) == "");
        REQUIRE(base32::encode(std::vector<uint8_t>({0}), error, 0) == "AA======");
        REQUIRE(base32::encode(std::vector<uint8_t>({0, 0}), error, 0) == "AAAA====");
        REQUIRE(base32::encode(std::vector<uint8_t>({0, 0, 0}), error, 0) == "AAAAA===");
        REQUIRE(base32::encode(std::vector<uint8_t>({0, 0, 0, 0}), error, 0) == "AAAAAAA=");
        REQUIRE(base32::encode(std::vector<uint8_t>({0, 0, 0, 0, 0}), error, 0) == "AAAAAAAA");
        REQUIRE(base32::encode(std::vector<uint8_t>({0, 0, 0, 0, 0, 0}), error, 0) == "AAAAAAAAAA======");

        // Constructing an std::string reduces the size of the char array by one (null terminator).
        // Therefore, the result for passing the string literal directly ends up encoding
        // one more character, which produces two more symbols in this particular case.
        REQUIRE(base32::encode(std::string("12345"), error, 0) == "GEZDGNBV");
        REQUIRE(base32::encode("12345", error, 0) == "GEZDGNBVAA======");

        // RFC 4648: 10. Test Vectors
        REQUIRE(base32::encode(std::string(""), error, 0) == "");
        REQUIRE(base32::encode(std::string("f"), error, 0) == "MY======");
        REQUIRE(base32::encode(std::string("fo"), error, 0) == "MZXQ====");
        REQUIRE(base32::encode(std::string("foo"), error, 0) == "MZXW6===");
        REQUIRE(base32::encode(std::string("foob"), error, 0) == "MZXW6YQ=");
        REQUIRE(base32::encode(std::string("fooba"), error, 0) == "MZXW6YTB");
        REQUIRE(base32::encode(std::string("foobar"), error, 0) == "MZXW6YTBOI======");

        // Other test strings.
        REQUIRE(base32::encode(std::string("ABCDE"), error, 0) == "IFBEGRCF");
        REQUIRE(base32::encode(std::vector<uint8_t>({255, 255, 255, 255, 255}), error, 0) == "77777777");
    }

    SECTION("decoding data") {
        cppcodec::error error;
        REQUIRE(base32::decode("", error) == std::vector<uint8_t>());
        REQUIRE(base32::decode("AA======", error) == std::vector<uint8_t>({0}));
        REQUIRE(base32::decode("AAAA====", error) == std::vector<uint8_t>({0, 0}));
        REQUIRE(base32::decode("AAAAA===", error) == std::vector<uint8_t>({0, 0, 0}));
        REQUIRE(base32::decode("AAAAAAA=", error) == std::vector<uint8_t>({0, 0, 0, 0}));
        REQUIRE(base32::decode("AAAAAAAA", error) == std::vector<uint8_t>({0, 0, 0, 0, 0}));
        REQUIRE(base32::decode("AAAAAAAAAA======", error) == std::vector<uint8_t>({0, 0, 0, 0, 0, 0}));

        // For decoding data, the result should be the same whether or not there is
        // a null terminator at the end, because the input is a string (not binary array).
        REQUIRE(base32::decode<std::string>(std::string("GEZDGNBV"), error) == "12345");
        REQUIRE(base32::decode<std::string>("GEZDGNBV", error) == "12345");

        // RFC 4648: 10. Test Vectors
        REQUIRE(base32::decode<std::string>("", error) == "");
        REQUIRE(base32::decode<std::string>("MY======", error) == "f");
        REQUIRE(base32::decode<std::string>("MZXQ====", error) == "fo");
        REQUIRE(base32::decode<std::string>("MZXW6===", error) == "foo");
        REQUIRE(base32::decode<std::string>("MZXW6YQ=", error) == "foob");
        REQUIRE(base32::decode<std::string>("MZXW6YTB", error) == "fooba");
        REQUIRE(base32::decode<std::string>("MZXW6YTBOI======", error) == "foobar");

        // Other test strings.
        REQUIRE(base32::decode<std::string>("IFBEGRCF", error) == "ABCDE");
        REQUIRE(base32::decode("77777777", error) == std::vector<uint8_t>({255, 255, 255, 255, 255}));

        // Lowercase should decode just as well as uppercase.
        REQUIRE(base32::decode<std::string>("mzxw6ytb", error) == "fooba");
        REQUIRE(base32::decode<std::string>("mZxW6yTb", error) == "fooba");

        // An invalid number of symbols should throw the right kind of parse_error.
        error = std::monostate();
        base32::decode("A", error);
        REQUIRE(std::holds_alternative<cppcodec::padding_error_value>(error));

        error = std::monostate();
        base32::decode("AA", error);
        REQUIRE(std::holds_alternative<cppcodec::padding_error_value>(error));

        error = std::monostate();
        base32::decode("AA===", error);
        REQUIRE(std::holds_alternative<cppcodec::padding_error_value>(error));

        error = std::monostate();
        base32::decode("A=======", error);
        REQUIRE(std::holds_alternative<cppcodec::invalid_input_length_error_value>(error));

        error = std::monostate();
        base32::decode("AAA=====", error);
        REQUIRE(std::holds_alternative<cppcodec::invalid_input_length_error_value>(error));

        error = std::monostate();
        base32::decode("AAAAAA==", error);
        REQUIRE(std::holds_alternative<cppcodec::invalid_input_length_error_value>(error));

        // An invalid symbol should throw a symbol error.
        error = std::monostate();
        base32::decode("0A======", error);
        REQUIRE(std::holds_alternative<cppcodec::symbol_error_value>(error));

        error = std::monostate();
        base32::decode("1A======", error);
        REQUIRE(std::holds_alternative<cppcodec::symbol_error_value>(error));

        error = std::monostate();
        base32::decode("8A======", error);
        REQUIRE(std::holds_alternative<cppcodec::symbol_error_value>(error));

        error = std::monostate();
        base32::decode("9A======", error);
        REQUIRE(std::holds_alternative<cppcodec::symbol_error_value>(error));

        error = std::monostate();
        base32::decode("GEZD GNBV", error); // no spaces
        REQUIRE(std::holds_alternative<cppcodec::symbol_error_value>(error));

        error = std::monostate();
        base32::decode("GEZD-GNBV", error); // no dashes
        REQUIRE(std::holds_alternative<cppcodec::symbol_error_value>(error));
    }
}

TEST_CASE("base64 (RFC 4648)", "[base64][rfc4648]") {
    using base64 = cppcodec::base64_rfc4648;

    SECTION("encoded size calculation") {
        REQUIRE(base64::encoded_size(0) == 0);
        REQUIRE(base64::encoded_size(1) == 4);
        REQUIRE(base64::encoded_size(2) == 4);
        REQUIRE(base64::encoded_size(3) == 4);
        REQUIRE(base64::encoded_size(4) == 8);
        REQUIRE(base64::encoded_size(5) == 8);
        REQUIRE(base64::encoded_size(6) == 8);
        REQUIRE(base64::encoded_size(7) == 12);
        REQUIRE(base64::encoded_size(12) == 16);
    }

    SECTION("maximum decoded size calculation") {
        REQUIRE(base64::decoded_max_size(0) == 0);
        REQUIRE(base64::decoded_max_size(1) == 0);
        REQUIRE(base64::decoded_max_size(2) == 0);
        REQUIRE(base64::decoded_max_size(3) == 0);
        REQUIRE(base64::decoded_max_size(4) == 3);
        REQUIRE(base64::decoded_max_size(5) == 3);
        REQUIRE(base64::decoded_max_size(6) == 3);
        REQUIRE(base64::decoded_max_size(7) == 3);
        REQUIRE(base64::decoded_max_size(8) == 6);
        REQUIRE(base64::decoded_max_size(9) == 6);
        REQUIRE(base64::decoded_max_size(10) == 6);
        REQUIRE(base64::decoded_max_size(11) == 6);
        REQUIRE(base64::decoded_max_size(12) == 9);
        REQUIRE(base64::decoded_max_size(16) == 12);
    }

    SECTION("encoding data") {
        cppcodec::error error;
        REQUIRE(base64::encode(std::vector<uint8_t>(), error, 0) == "");
        REQUIRE(base64::encode(std::vector<uint8_t>({0}), error, 0) == "AA==");
        REQUIRE(base64::encode(std::vector<uint8_t>({0, 0}), error, 0) == "AAA=");
        REQUIRE(base64::encode(std::vector<uint8_t>({0, 0, 0}), error, 0) == "AAAA");
        REQUIRE(base64::encode(std::vector<uint8_t>({0, 0, 0, 0}), error, 0) == "AAAAAA==");
        REQUIRE(base64::encode(std::vector<uint8_t>({0, 0, 0, 0, 0}), error, 0) == "AAAAAAA=");
        REQUIRE(base64::encode(std::vector<uint8_t>({0, 0, 0, 0, 0, 0}), error, 0) == "AAAAAAAA");

        // Constructing an std::string reduces the size of the char array by one (null terminator).
        // Therefore, the result for passing the string literal directly ends up encoding
        // one more character, which produces two more symbols in this particular case.
        REQUIRE(base64::encode(std::string("Man"), error, 0) == "TWFu");
        REQUIRE(base64::encode("Man", error, 0) == "TWFuAA==");

        // Wikipedia
        REQUIRE(base64::encode(std::string("pleasure."), error, 0) == "cGxlYXN1cmUu");
        REQUIRE(base64::encode(std::string("leasure."), error, 0) == "bGVhc3VyZS4=");
        REQUIRE(base64::encode(std::string("easure."), error, 0) == "ZWFzdXJlLg==");
        REQUIRE(base64::encode(std::string("asure."), error, 0) == "YXN1cmUu");
        REQUIRE(base64::encode(std::string("sure."), error, 0) == "c3VyZS4=");

        REQUIRE(base64::encode(std::string("any carnal pleas"), error, 0) == "YW55IGNhcm5hbCBwbGVhcw==");
        REQUIRE(base64::encode(std::string("any carnal pleasu"), error, 0) == "YW55IGNhcm5hbCBwbGVhc3U=");
        REQUIRE(base64::encode(std::string("any carnal pleasur"), error, 0) == "YW55IGNhcm5hbCBwbGVhc3Vy");

        // RFC 4648: 9. Illustrations and Examples, adapted for more special characters
        REQUIRE(base64::encode(std::vector<uint8_t>({0x14, 0xFB, 0xBF, 0x03, 0xD9, 0x7E}), error, 0) == "FPu/A9l+");
        REQUIRE(base64::encode(std::vector<uint8_t>({0x14, 0xFB, 0xBF, 0x03, 0xD9}), error, 0) == "FPu/A9k=");
        REQUIRE(base64::encode(std::vector<uint8_t>({0x14, 0xFB, 0xBF, 0x03}), error, 0) == "FPu/Aw==");

        // RFC 4648: 10. Test Vectors
        REQUIRE(base64::encode(std::string(""), error, 0) == "");
        REQUIRE(base64::encode(std::string("f"), error, 0) == "Zg==");
        REQUIRE(base64::encode(std::string("fo"), error, 0) == "Zm8=");
        REQUIRE(base64::encode(std::string("foo"), error, 0) == "Zm9v");
        REQUIRE(base64::encode(std::string("foob"), error, 0) == "Zm9vYg==");
        REQUIRE(base64::encode(std::string("fooba"), error, 0) == "Zm9vYmE=");
        REQUIRE(base64::encode(std::string("foobar"), error, 0) == "Zm9vYmFy");

        // Other test strings.
        REQUIRE(base64::encode(std::string("123"), error, 0) == "MTIz");
        REQUIRE(base64::encode(std::string("ABC"), error, 0) == "QUJD");
        REQUIRE(base64::encode(std::string("\xFF\xFF\xFF"), error, 0) == "////");
    }

    SECTION("decoding data") {
        cppcodec::error error;
        REQUIRE(base64::decode("", error) == std::vector<uint8_t>());
        REQUIRE(base64::decode("AA==", error) == std::vector<uint8_t>({0}));
        REQUIRE(base64::decode("AAA=", error) == std::vector<uint8_t>({0, 0}));
        REQUIRE(base64::decode("AAAA", error) == std::vector<uint8_t>({0, 0, 0}));
        REQUIRE(base64::decode("AAAAAA==", error) == std::vector<uint8_t>({0, 0, 0, 0}));
        REQUIRE(base64::decode("AAAAAAA=", error) == std::vector<uint8_t>({0, 0, 0, 0, 0}));
        REQUIRE(base64::decode("AAAAAAAA", error) == std::vector<uint8_t>({0, 0, 0, 0, 0, 0}));

        // For decoding data, the result should be the same whether or not there is
        // a null terminator at the end, because the input is a string (not binary array).
        REQUIRE(base64::decode<std::string>(std::string("TWFu"), error) == "Man");
        REQUIRE(base64::decode<std::string>("TWFu", error) == "Man");

        // Wikipedia
        REQUIRE(base64::decode<std::string>("cGxlYXN1cmUu", error) == "pleasure.");
        REQUIRE(base64::decode<std::string>("bGVhc3VyZS4=", error) == "leasure.");
        REQUIRE(base64::decode<std::string>("ZWFzdXJlLg==", error) == "easure.");
        REQUIRE(base64::decode<std::string>("YXN1cmUu", error) == "asure.");
        REQUIRE(base64::decode<std::string>("c3VyZS4=", error) == "sure.");

        REQUIRE(base64::decode<std::string>("YW55IGNhcm5hbCBwbGVhcw==", error) == "any carnal pleas");
        REQUIRE(base64::decode<std::string>("YW55IGNhcm5hbCBwbGVhc3U=", error) == "any carnal pleasu");
        REQUIRE(base64::decode<std::string>("YW55IGNhcm5hbCBwbGVhc3Vy", error) == "any carnal pleasur");

        // RFC 4648: 9. Illustrations and Examples, adapted for more special characters
        REQUIRE(base64::decode("FPu/A9l+", error) == std::vector<uint8_t>({0x14, 0xFB, 0xBF, 0x03, 0xD9, 0x7E}));
        REQUIRE(base64::decode("FPu/A9k=", error) == std::vector<uint8_t>({0x14, 0xFB, 0xBF, 0x03, 0xD9}));
        REQUIRE(base64::decode("FPu/Aw==", error) == std::vector<uint8_t>({0x14, 0xFB, 0xBF, 0x03}));

        // RFC 4648: 10. Test Vectors
        REQUIRE(base64::decode<std::string>("", error) == "");
        REQUIRE(base64::decode<std::string>("Zg==", error) == "f");
        REQUIRE(base64::decode<std::string>("Zm8=", error) == "fo");
        REQUIRE(base64::decode<std::string>("Zm9v", error) == "foo");
        REQUIRE(base64::decode<std::string>("Zm9vYg==", error) == "foob");
        REQUIRE(base64::decode<std::string>("Zm9vYmE=", error) == "fooba");
        REQUIRE(base64::decode<std::string>("Zm9vYmFy", error) == "foobar");

        // Other test strings.
        REQUIRE(base64::decode<std::string>("MTIz", error) == "123");
        REQUIRE(base64::decode<std::string>("QUJD", error) == "ABC");
        REQUIRE(base64::decode("////", error) == std::vector<uint8_t>({255, 255, 255}));

        // An invalid number of symbols should throw the right kind of parse_error.
        error = std::monostate();
        base64::decode("A", error);
        REQUIRE(std::holds_alternative<cppcodec::padding_error_value>(error));
        
        error = std::monostate();
        base64::decode("AA", error);
        REQUIRE(std::holds_alternative<cppcodec::padding_error_value>(error));
        
        error = std::monostate();
        base64::decode("ABCDE", error);
        REQUIRE(std::holds_alternative<cppcodec::padding_error_value>(error));
        
        error = std::monostate();
        base64::decode("A===", error);
        REQUIRE(std::holds_alternative<cppcodec::invalid_input_length_error_value>(error));
        
        error = std::monostate();
        base64::decode("AAAA====", error);
        REQUIRE(std::holds_alternative<cppcodec::padding_error_value>(error));
        
        error = std::monostate();
        base64::decode("AAAAA===", error);
        REQUIRE(std::holds_alternative<cppcodec::invalid_input_length_error_value>(error));
        
        // An invalid symbol should throw a symbol error.
        error = std::monostate();
        base64::decode("A&B=", error);
        REQUIRE(std::holds_alternative<cppcodec::symbol_error_value>(error));
        
        error = std::monostate();
        base64::decode("--", error); // this is not base64url
        REQUIRE(std::holds_alternative<cppcodec::symbol_error_value>(error));
        
        error = std::monostate();
        base64::decode("__", error); // ...ditto
        REQUIRE(std::holds_alternative<cppcodec::symbol_error_value>(error));
    }
}

TEST_CASE("base64 (unpadded URL-safe)", "[base64][url_unpadded]") {
    using base64 = cppcodec::base64_url_unpadded;

    SECTION("encoded size calculation") {
        REQUIRE(base64::encoded_size(0) == 0);
        REQUIRE(base64::encoded_size(1) == 2);
        REQUIRE(base64::encoded_size(2) == 3);
        REQUIRE(base64::encoded_size(3) == 4);
        REQUIRE(base64::encoded_size(4) == 6);
        REQUIRE(base64::encoded_size(5) == 7);
        REQUIRE(base64::encoded_size(6) == 8);
        REQUIRE(base64::encoded_size(7) == 10);
        REQUIRE(base64::encoded_size(12) == 16);
    }

    SECTION("maximum decoded size calculation") {
        REQUIRE(base64::decoded_max_size(0) == 0);
        REQUIRE(base64::decoded_max_size(1) == 0);
        REQUIRE(base64::decoded_max_size(2) == 1);
        REQUIRE(base64::decoded_max_size(3) == 2);
        REQUIRE(base64::decoded_max_size(4) == 3);
        REQUIRE(base64::decoded_max_size(5) == 3);
        REQUIRE(base64::decoded_max_size(6) == 4);
        REQUIRE(base64::decoded_max_size(7) == 5);
        REQUIRE(base64::decoded_max_size(8) == 6);
        REQUIRE(base64::decoded_max_size(9) == 6);
        REQUIRE(base64::decoded_max_size(10) == 7);
        REQUIRE(base64::decoded_max_size(11) == 8);
        REQUIRE(base64::decoded_max_size(12) == 9);
        REQUIRE(base64::decoded_max_size(16) == 12);
    }

    SECTION("encoding data") {
        cppcodec::error error;
        REQUIRE(base64::encode(std::vector<uint8_t>(), error, 0) == "");
        REQUIRE(base64::encode(std::vector<uint8_t>({0}), error, 0) == "AA");
        REQUIRE(base64::encode(std::vector<uint8_t>({0, 0}), error, 0) == "AAA");
        REQUIRE(base64::encode(std::vector<uint8_t>({0, 0, 0}), error, 0) == "AAAA");
        REQUIRE(base64::encode(std::vector<uint8_t>({0, 0, 0, 0}), error, 0) == "AAAAAA");
        REQUIRE(base64::encode(std::vector<uint8_t>({0, 0, 0, 0, 0}), error, 0) == "AAAAAAA");
        REQUIRE(base64::encode(std::vector<uint8_t>({0, 0, 0, 0, 0, 0}), error, 0) == "AAAAAAAA");

        // RFC 4648: 9. Illustrations and Examples, adapted for more special characters
        REQUIRE(base64::encode(std::vector<uint8_t>({0x14, 0xFB, 0xBF, 0x03, 0xD9, 0x7E}), error, 0) == "FPu_A9l-");
        REQUIRE(base64::encode(std::vector<uint8_t>({0x14, 0xFB, 0xBF, 0x03, 0xD9}), error, 0) == "FPu_A9k");
        REQUIRE(base64::encode(std::vector<uint8_t>({0x14, 0xFB, 0xBF, 0x03}), error, 0) == "FPu_Aw");

        // RFC 4648: 10. Test Vectors
        REQUIRE(base64::encode(std::string(""), error, 0) == "");
        REQUIRE(base64::encode(std::string("f"), error, 0) == "Zg");
        REQUIRE(base64::encode(std::string("fo"), error, 0) == "Zm8");
        REQUIRE(base64::encode(std::string("foo"), error, 0) == "Zm9v");
        REQUIRE(base64::encode(std::string("foob"), error, 0) == "Zm9vYg");
        REQUIRE(base64::encode(std::string("fooba"), error, 0) == "Zm9vYmE");
        REQUIRE(base64::encode(std::string("foobar"), error, 0) == "Zm9vYmFy");

        // Other test strings.
        REQUIRE(base64::encode(std::string("123"), error, 0) == "MTIz");
        REQUIRE(base64::encode(std::string("ABC"), error, 0) == "QUJD");
        REQUIRE(base64::encode(std::string("\xFF\xFF\xFF"), error, 0) == "____");
    }

    SECTION("decoding data") {
        cppcodec::error error;
        REQUIRE(base64::decode("", error) == std::vector<uint8_t>());
        REQUIRE(base64::decode("AA", error) == std::vector<uint8_t>({0}));
        REQUIRE(base64::decode("AAA", error) == std::vector<uint8_t>({0, 0}));
        REQUIRE(base64::decode("AAAA", error) == std::vector<uint8_t>({0, 0, 0}));
        REQUIRE(base64::decode("AAAAAA", error) == std::vector<uint8_t>({0, 0, 0, 0}));
        REQUIRE(base64::decode("AAAAAAA", error) == std::vector<uint8_t>({0, 0, 0, 0, 0}));
        REQUIRE(base64::decode("AAAAAAAA", error) == std::vector<uint8_t>({0, 0, 0, 0, 0, 0}));

        // RFC 4648: 9. Illustrations and Examples, adapted for more special characters
        REQUIRE(base64::decode("FPu_A9l-", error) == std::vector<uint8_t>({0x14, 0xFB, 0xBF, 0x03, 0xD9, 0x7E}));
        REQUIRE(base64::decode("FPu_A9k", error) == std::vector<uint8_t>({0x14, 0xFB, 0xBF, 0x03, 0xD9}));
        REQUIRE(base64::decode("FPu_Aw", error) == std::vector<uint8_t>({0x14, 0xFB, 0xBF, 0x03}));

        // RFC 4648: 10. Test Vectors
        REQUIRE(base64::decode<std::string>("", error) == "");
        REQUIRE(base64::decode<std::string>("Zg", error) == "f");
        REQUIRE(base64::decode<std::string>("Zg==", error) == "f");
        REQUIRE(base64::decode<std::string>("Zm8", error) == "fo");
        REQUIRE(base64::decode<std::string>("Zm8=", error) == "fo");
        REQUIRE(base64::decode<std::string>("Zm9v", error) == "foo");
        REQUIRE(base64::decode<std::string>("Zm9vYg", error) == "foob");
        REQUIRE(base64::decode<std::string>("Zm9vYg==", error) == "foob");
        REQUIRE(base64::decode<std::string>("Zm9vYmE", error) == "fooba");
        REQUIRE(base64::decode<std::string>("Zm9vYmE=", error) == "fooba");
        REQUIRE(base64::decode<std::string>("Zm9vYmFy", error) == "foobar");

        // Unpadded base64_url allows padding, but an incorrect number of padding characters is still wrong.
        error = std::monostate();
        base64::decode<std::string>("Zg=", error);
        REQUIRE(std::holds_alternative<cppcodec::padding_error_value>(error));

        // Other test strings.
        REQUIRE(base64::decode<std::string>("MTIz", error) == "123");
        REQUIRE(base64::decode<std::string>("QUJD", error) == "ABC");
        REQUIRE(base64::decode("____", error) == std::vector<uint8_t>({255, 255, 255}));

        // An invalid number of symbols should throw the right kind of parse_error.
        error = std::monostate();
        base64::decode("A", error);
        REQUIRE(std::holds_alternative<cppcodec::invalid_input_length_error_value>(error));
        
        error = std::monostate();
        base64::decode("AAAAA", error);
        REQUIRE(std::holds_alternative<cppcodec::invalid_input_length_error_value>(error));
        
        // An invalid symbol should throw a symbol error.
        error = std::monostate();
        base64::decode("A&B", error);
        REQUIRE(std::holds_alternative<cppcodec::symbol_error_value>(error));
        
        error = std::monostate();
        base64::decode("++", error); // this is not standard base64
        REQUIRE(std::holds_alternative<cppcodec::symbol_error_value>(error));
        
        error = std::monostate();
        base64::decode("//", error); // ...ditto
        REQUIRE(std::holds_alternative<cppcodec::symbol_error_value>(error));
    }
}

TEST_CASE("base64 (URL-safe)", "[base64][url]") {
    using base64 = cppcodec::base64_url;

    SECTION("encoded size calculation") {
        REQUIRE(base64::encoded_size(0) == 0);
        REQUIRE(base64::encoded_size(1) == 4);
        REQUIRE(base64::encoded_size(2) == 4);
        REQUIRE(base64::encoded_size(3) == 4);
        REQUIRE(base64::encoded_size(4) == 8);
        REQUIRE(base64::encoded_size(5) == 8);
        REQUIRE(base64::encoded_size(6) == 8);
        REQUIRE(base64::encoded_size(7) == 12);
        REQUIRE(base64::encoded_size(12) == 16);
    }

    SECTION("maximum decoded size calculation") {
        REQUIRE(base64::decoded_max_size(0) == 0);
        REQUIRE(base64::decoded_max_size(1) == 0);
        REQUIRE(base64::decoded_max_size(2) == 0);
        REQUIRE(base64::decoded_max_size(3) == 0);
        REQUIRE(base64::decoded_max_size(4) == 3);
        REQUIRE(base64::decoded_max_size(5) == 3);
        REQUIRE(base64::decoded_max_size(6) == 3);
        REQUIRE(base64::decoded_max_size(7) == 3);
        REQUIRE(base64::decoded_max_size(8) == 6);
        REQUIRE(base64::decoded_max_size(9) == 6);
        REQUIRE(base64::decoded_max_size(10) == 6);
        REQUIRE(base64::decoded_max_size(11) == 6);
        REQUIRE(base64::decoded_max_size(12) == 9);
        REQUIRE(base64::decoded_max_size(16) == 12);
    }

    SECTION("encoding data") {
        cppcodec::error error;
        REQUIRE(base64::encode(std::vector<uint8_t>(), error, 0) == "");
        REQUIRE(base64::encode(std::vector<uint8_t>({0}), error, 0) == "AA==");
        REQUIRE(base64::encode(std::vector<uint8_t>({0, 0}), error, 0) == "AAA=");
        REQUIRE(base64::encode(std::vector<uint8_t>({0, 0, 0}), error, 0) == "AAAA");
        REQUIRE(base64::encode(std::vector<uint8_t>({0, 0, 0, 0}), error, 0) == "AAAAAA==");
        REQUIRE(base64::encode(std::vector<uint8_t>({0, 0, 0, 0, 0}), error, 0) == "AAAAAAA=");
        REQUIRE(base64::encode(std::vector<uint8_t>({0, 0, 0, 0, 0, 0}), error, 0) == "AAAAAAAA");

        // RFC 4648: 9. Illustrations and Examples, adapted for more special characters
        REQUIRE(base64::encode(std::vector<uint8_t>({0x14, 0xFB, 0xBF, 0x03, 0xD9, 0x7E}), error, 0) == "FPu_A9l-");
        REQUIRE(base64::encode(std::vector<uint8_t>({0x14, 0xFB, 0xBF, 0x03, 0xD9}), error, 0) == "FPu_A9k=");
        REQUIRE(base64::encode(std::vector<uint8_t>({0x14, 0xFB, 0xBF, 0x03}), error, 0) == "FPu_Aw==");

        // RFC 4648: 10. Test Vectors
        REQUIRE(base64::encode(std::string(""), error, 0) == "");
        REQUIRE(base64::encode(std::string("f"), error, 0) == "Zg==");
        REQUIRE(base64::encode(std::string("fo"), error, 0) == "Zm8=");
        REQUIRE(base64::encode(std::string("foo"), error, 0) == "Zm9v");
        REQUIRE(base64::encode(std::string("foob"), error, 0) == "Zm9vYg==");
        REQUIRE(base64::encode(std::string("fooba"), error, 0) == "Zm9vYmE=");
        REQUIRE(base64::encode(std::string("foobar"), error, 0) == "Zm9vYmFy");

        // Other test strings.
        REQUIRE(base64::encode(std::string("123"), error, 0) == "MTIz");
        REQUIRE(base64::encode(std::string("ABC"), error, 0) == "QUJD");
        REQUIRE(base64::encode(std::string("\xFF\xFF\xFF"), error, 0) == "____");
    }

    SECTION("decoding data") {
        cppcodec::error error;
        REQUIRE(base64::decode("", error) == std::vector<uint8_t>());
        REQUIRE(base64::decode("AA==", error) == std::vector<uint8_t>({0}));
        REQUIRE(base64::decode("AAA=", error) == std::vector<uint8_t>({0, 0}));
        REQUIRE(base64::decode("AAAA", error) == std::vector<uint8_t>({0, 0, 0}));
        REQUIRE(base64::decode("AAAAAA==", error) == std::vector<uint8_t>({0, 0, 0, 0}));
        REQUIRE(base64::decode("AAAAAAA=", error) == std::vector<uint8_t>({0, 0, 0, 0, 0}));
        REQUIRE(base64::decode("AAAAAAAA", error) == std::vector<uint8_t>({0, 0, 0, 0, 0, 0}));

        // RFC 4648: 9. Illustrations and Examples, adapted for more special characters
        REQUIRE(base64::decode("FPu_A9l-", error) == std::vector<uint8_t>({0x14, 0xFB, 0xBF, 0x03, 0xD9, 0x7E}));
        REQUIRE(base64::decode("FPu_A9k=", error) == std::vector<uint8_t>({0x14, 0xFB, 0xBF, 0x03, 0xD9}));
        REQUIRE(base64::decode("FPu_Aw==", error) == std::vector<uint8_t>({0x14, 0xFB, 0xBF, 0x03}));

        // RFC 4648: 10. Test Vectors
        REQUIRE(base64::decode<std::string>("", error) == "");
        REQUIRE(base64::decode<std::string>("Zg==", error) == "f");
        REQUIRE(base64::decode<std::string>("Zm8=", error) == "fo");
        REQUIRE(base64::decode<std::string>("Zm9v", error) == "foo");
        REQUIRE(base64::decode<std::string>("Zm9vYg==", error) == "foob");
        REQUIRE(base64::decode<std::string>("Zm9vYmE=", error) == "fooba");
        REQUIRE(base64::decode<std::string>("Zm9vYmFy", error) == "foobar");

        // Other test strings.
        REQUIRE(base64::decode<std::string>("MTIz", error) == "123");
        REQUIRE(base64::decode<std::string>("QUJD", error) == "ABC");
        REQUIRE(base64::decode("____", error) == std::vector<uint8_t>({255, 255, 255}));

        // An invalid number of symbols should throw the right kind of parse_error.
        error = std::monostate();
        base64::decode("A", error);
        REQUIRE(std::holds_alternative<cppcodec::padding_error_value>(error));
        
        error = std::monostate();
        base64::decode("AA", error);
        REQUIRE(std::holds_alternative<cppcodec::padding_error_value>(error));
        
        error = std::monostate();
        base64::decode("ABCDE", error);
        REQUIRE(std::holds_alternative<cppcodec::padding_error_value>(error));
        
        error = std::monostate();
        base64::decode("A===", error);
        REQUIRE(std::holds_alternative<cppcodec::invalid_input_length_error_value>(error));
        
        error = std::monostate();
        base64::decode("AAAA====", error);
        REQUIRE(std::holds_alternative<cppcodec::padding_error_value>(error));
        
        error = std::monostate();
        base64::decode("AAAAA===", error);
        REQUIRE(std::holds_alternative<cppcodec::invalid_input_length_error_value>(error));

        // An invalid symbol should throw a symbol error.
        error = std::monostate();
        base64::decode("A&B=", error);
        REQUIRE(std::holds_alternative<cppcodec::symbol_error_value>(error));
        
        error = std::monostate();
        base64::decode("++", error); // this is not standard base64
        REQUIRE(std::holds_alternative<cppcodec::symbol_error_value>(error));
        
        error = std::monostate();
        base64::decode("//", error); // ...ditto
        REQUIRE(std::holds_alternative<cppcodec::symbol_error_value>(error));
    }
}

TEST_CASE("hex (lowercase)", "[hex][lower]") {
    using hex = cppcodec::hex_lower;

    SECTION("encoded size calculation") {
        REQUIRE(hex::encoded_size(0) == 0);
        REQUIRE(hex::encoded_size(1) == 2);
        REQUIRE(hex::encoded_size(2) == 4);
        REQUIRE(hex::encoded_size(3) == 6);
        REQUIRE(hex::encoded_size(4) == 8);
        REQUIRE(hex::encoded_size(5) == 10);
        REQUIRE(hex::encoded_size(6) == 12);
        REQUIRE(hex::encoded_size(8) == 16);
        REQUIRE(hex::encoded_size(10) == 20);
    }

    SECTION("maximum decoded size calculation") {
        REQUIRE(hex::decoded_max_size(0) == 0);
        REQUIRE(hex::decoded_max_size(1) == 0);
        REQUIRE(hex::decoded_max_size(2) == 1);
        REQUIRE(hex::decoded_max_size(3) == 1);
        REQUIRE(hex::decoded_max_size(4) == 2);
        REQUIRE(hex::decoded_max_size(5) == 2);
        REQUIRE(hex::decoded_max_size(6) == 3);
        REQUIRE(hex::decoded_max_size(7) == 3);
        REQUIRE(hex::decoded_max_size(8) == 4);
        REQUIRE(hex::decoded_max_size(9) == 4);
        REQUIRE(hex::decoded_max_size(10) == 5);
        REQUIRE(hex::decoded_max_size(16) == 8);
        REQUIRE(hex::decoded_max_size(20) == 10);
    }

    SECTION("encoding data") {
        cppcodec::error error;
        REQUIRE(hex::encode(std::vector<uint8_t>(), error, 0) == "");
        REQUIRE(hex::encode(std::vector<uint8_t>({0}), error, 0) == "00");
        REQUIRE(hex::encode(std::vector<uint8_t>({0, 0}), error, 0) == "0000");
        REQUIRE(hex::encode(std::vector<uint8_t>({0, 0, 0}), error, 0) == "000000");
        REQUIRE(hex::encode(std::vector<uint8_t>({0, 0, 0, 0}), error, 0) == "00000000");
        REQUIRE(hex::encode(std::vector<uint8_t>({0, 0, 0, 0, 0}), error, 0) == "0000000000");
        REQUIRE(hex::encode(std::vector<uint8_t>({0, 0, 0, 0, 0, 0}), error, 0) == "000000000000");

        // Constructing an std::string reduces the size of the char array by one (null terminator).
        // Therefore, the result for passing the string literal directly ends up encoding
        // one more character, which produces two more symbols in this particular case.
        REQUIRE(hex::encode(std::string("1"), error, 0) == "31");
        REQUIRE(hex::encode("1", error, 0) == "3100");

        REQUIRE(hex::encode(std::string("A"), error, 0) == "41");
        REQUIRE(hex::encode(std::vector<uint8_t>({255}), error, 0) == "ff");

        // RFC 4648: 10. Test Vectors
        REQUIRE(hex::encode(std::string(""), error, 0) == "");
        REQUIRE(hex::encode(std::string("f"), error, 0) == "66");
        REQUIRE(hex::encode(std::string("fo"), error, 0) == "666f");
        REQUIRE(hex::encode(std::string("foo"), error, 0) == "666f6f");
        REQUIRE(hex::encode(std::string("foob"), error, 0) == "666f6f62");
        REQUIRE(hex::encode(std::string("fooba"), error, 0) == "666f6f6261");
        REQUIRE(hex::encode(std::string("foobar"), error, 0) == "666f6f626172");
    }

    SECTION("decoding data") {
        cppcodec::error error;
        REQUIRE(hex::decode("", error) == std::vector<uint8_t>());
        REQUIRE(hex::decode("00", error) == std::vector<uint8_t>({0}));
        REQUIRE(hex::decode("0000", error) == std::vector<uint8_t>({0, 0}));
        REQUIRE(hex::decode("000000", error) == std::vector<uint8_t>({0, 0, 0}));
        REQUIRE(hex::decode("00000000", error) == std::vector<uint8_t>({0, 0, 0, 0}));
        REQUIRE(hex::decode("0000000000", error) == std::vector<uint8_t>({0, 0, 0, 0, 0}));
        REQUIRE(hex::decode("000000000000", error) == std::vector<uint8_t>({0, 0, 0, 0, 0, 0}));

        // For decoding data, the result should be the same whether or not there is
        // a null terminator at the end, because the input is a string (not binary array).
        REQUIRE(hex::decode<std::string>(std::string("31"), error) == "1");
        REQUIRE(hex::decode<std::string>("31", error) == "1");

        // RFC 4648: 10. Test Vectors
        REQUIRE(hex::decode<std::string>("", error) == "");
        REQUIRE(hex::decode<std::string>("66", error) == "f");
        REQUIRE(hex::decode<std::string>("666f", error) == "fo");
        REQUIRE(hex::decode<std::string>("666f6f", error) == "foo");
        REQUIRE(hex::decode<std::string>("666f6f62", error) == "foob");
        REQUIRE(hex::decode<std::string>("666f6f6261", error) == "fooba");
        REQUIRE(hex::decode<std::string>("666f6f626172", error) == "foobar");

        // Uppercase should decode just as well as lowercase.
        REQUIRE(hex::decode<std::string>("666F6F6261", error) == "fooba");
        REQUIRE(hex::decode<std::string>("666F6f6261", error) == "fooba");

        // An invalid number of symbols should throw the right kind of parse_error.
        error = std::monostate();
        hex::decode("0", error);
        REQUIRE(std::holds_alternative<cppcodec::invalid_input_length_error_value>(error));
        
        error = std::monostate();
        hex::decode("000", error);
        REQUIRE(std::holds_alternative<cppcodec::invalid_input_length_error_value>(error));
        
        // An invalid symbol should throw a symbol error.
        error = std::monostate();
        hex::decode("1g", error);
        REQUIRE(std::holds_alternative<cppcodec::symbol_error_value>(error));
        
        error = std::monostate();
        hex::decode("66 6f", error); // no spaces
        REQUIRE(std::holds_alternative<cppcodec::symbol_error_value>(error));
        
        error = std::monostate();
        hex::decode("66-6f", error); // no dashes
        REQUIRE(std::holds_alternative<cppcodec::symbol_error_value>(error));
    }
}

TEST_CASE("hex (uppercase)", "[hex][upper]") {
    using hex = cppcodec::hex_upper;

    SECTION("encoded size calculation") {
        REQUIRE(hex::encoded_size(0) == 0);
        REQUIRE(hex::encoded_size(1) == 2);
        REQUIRE(hex::encoded_size(2) == 4);
        REQUIRE(hex::encoded_size(3) == 6);
        REQUIRE(hex::encoded_size(4) == 8);
        REQUIRE(hex::encoded_size(5) == 10);
        REQUIRE(hex::encoded_size(6) == 12);
        REQUIRE(hex::encoded_size(8) == 16);
        REQUIRE(hex::encoded_size(10) == 20);
    }

    SECTION("maximum decoded size calculation") {
        REQUIRE(hex::decoded_max_size(0) == 0);
        REQUIRE(hex::decoded_max_size(1) == 0);
        REQUIRE(hex::decoded_max_size(2) == 1);
        REQUIRE(hex::decoded_max_size(3) == 1);
        REQUIRE(hex::decoded_max_size(4) == 2);
        REQUIRE(hex::decoded_max_size(5) == 2);
        REQUIRE(hex::decoded_max_size(6) == 3);
        REQUIRE(hex::decoded_max_size(7) == 3);
        REQUIRE(hex::decoded_max_size(8) == 4);
        REQUIRE(hex::decoded_max_size(9) == 4);
        REQUIRE(hex::decoded_max_size(10) == 5);
        REQUIRE(hex::decoded_max_size(16) == 8);
        REQUIRE(hex::decoded_max_size(20) == 10);
    }

    SECTION("encoding data") {
        cppcodec::error error;
        REQUIRE(hex::encode(std::vector<uint8_t>(), error, 0) == "");
        REQUIRE(hex::encode(std::vector<uint8_t>({0}), error, 0) == "00");
        REQUIRE(hex::encode(std::vector<uint8_t>({0, 0}), error, 0) == "0000");
        REQUIRE(hex::encode(std::vector<uint8_t>({0, 0, 0}), error, 0) == "000000");
        REQUIRE(hex::encode(std::vector<uint8_t>({0, 0, 0, 0}), error, 0) == "00000000");
        REQUIRE(hex::encode(std::vector<uint8_t>({0, 0, 0, 0, 0}), error, 0) == "0000000000");
        REQUIRE(hex::encode(std::vector<uint8_t>({0, 0, 0, 0, 0, 0}), error, 0) == "000000000000");

        // Constructing an std::string reduces the size of the char array by one (null terminator).
        // Therefore, the result for passing the string literal directly ends up encoding
        // one more character, which produces two more symbols in this particular case.
        REQUIRE(hex::encode(std::string("1"), error, 0) == "31");
        REQUIRE(hex::encode("1", error, 0) == "3100");

        REQUIRE(hex::encode(std::string("A"), error, 0) == "41");
        REQUIRE(hex::encode(std::vector<uint8_t>({255}), error, 0) == "FF");

        // RFC 4648: 10. Test Vectors
        REQUIRE(hex::encode(std::string(""), error, 0) == "");
        REQUIRE(hex::encode(std::string("f"), error, 0) == "66");
        REQUIRE(hex::encode(std::string("fo"), error, 0) == "666F");
        REQUIRE(hex::encode(std::string("foo"), error, 0) == "666F6F");
        REQUIRE(hex::encode(std::string("foob"), error, 0) == "666F6F62");
        REQUIRE(hex::encode(std::string("fooba"), error, 0) == "666F6F6261");
        REQUIRE(hex::encode(std::string("foobar"), error, 0) == "666F6F626172");
    }

    SECTION("decoding data") {
        cppcodec::error error;
        REQUIRE(hex::decode("", error) == std::vector<uint8_t>());
        REQUIRE(hex::decode("00", error) == std::vector<uint8_t>({0}));
        REQUIRE(hex::decode("0000", error) == std::vector<uint8_t>({0, 0}));
        REQUIRE(hex::decode("000000", error) == std::vector<uint8_t>({0, 0, 0}));
        REQUIRE(hex::decode("00000000", error) == std::vector<uint8_t>({0, 0, 0, 0}));
        REQUIRE(hex::decode("0000000000", error) == std::vector<uint8_t>({0, 0, 0, 0, 0}));
        REQUIRE(hex::decode("000000000000", error) == std::vector<uint8_t>({0, 0, 0, 0, 0, 0}));

        // For decoding data, the result should be the same whether or not there is
        // a null terminator at the end, because the input is a string (not binary array).
        REQUIRE(hex::decode<std::string>(std::string("31"), error) == "1");
        REQUIRE(hex::decode<std::string>("31", error) == "1");

        // RFC 4648: 10. Test Vectors
        REQUIRE(hex::decode<std::string>("", error) == "");
        REQUIRE(hex::decode<std::string>("66", error) == "f");
        REQUIRE(hex::decode<std::string>("666F", error) == "fo");
        REQUIRE(hex::decode<std::string>("666F6F", error) == "foo");
        REQUIRE(hex::decode<std::string>("666F6F62", error) == "foob");
        REQUIRE(hex::decode<std::string>("666F6F6261", error) == "fooba");
        REQUIRE(hex::decode<std::string>("666F6F626172", error) == "foobar");

        // Lowercase should decode just as well as uppercase.
        REQUIRE(hex::decode<std::string>("666f6f6261", error) == "fooba");
        REQUIRE(hex::decode<std::string>("666f6F6261", error) == "fooba");

        // An invalid number of symbols should throw the right kind of parse_error.
        error = std::monostate();
        hex::decode("0", error);
        REQUIRE(std::holds_alternative<cppcodec::invalid_input_length_error_value>(error));
        
        error = std::monostate();
        hex::decode("000", error);
        REQUIRE(std::holds_alternative<cppcodec::invalid_input_length_error_value>(error));

        // An invalid symbol should throw a symbol error.
        error = std::monostate();
        hex::decode("1G", error);
        REQUIRE(std::holds_alternative<cppcodec::symbol_error_value>(error));
        
        error = std::monostate();
        hex::decode("66 6F", error); // no spaces
        REQUIRE(std::holds_alternative<cppcodec::symbol_error_value>(error));
        
        error = std::monostate();
        hex::decode("66-6F", error); // no dashes
        REQUIRE(std::holds_alternative<cppcodec::symbol_error_value>(error));
    }
}
