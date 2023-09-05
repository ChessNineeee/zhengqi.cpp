//
// Created by 70903 on 2023/9/5.
//
#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.h"
#include "utility/LogStream.h"
#include <string>
using namespace zhengqi::utility;

TEST_CASE("testLogStreamBooleans")
{
    LogStream os;
    const LogStream::Buffer& buf = os.buffer();

    REQUIRE(buf.toString() == string(""));
    os << true;
    REQUIRE(buf.toString() == string("1"));
    os << '\n';
    REQUIRE(buf.toString() == string("1\n"));
    os << false;
    REQUIRE(buf.toString() == string("1\n0"));
}

TEST_CASE("testLogStreamIntegers")
{
    LogStream os;
    const LogStream::Buffer& buf = os.buffer();

    REQUIRE(buf.toString() == string(""));
    os << 1;
    REQUIRE(buf.toString() == string("1"));
    os << 0;
    REQUIRE(buf.toString() == string("10"));
    os << -1;
    REQUIRE(buf.toString() == string("10-1"));

    os.resetBuffer();
    os << 0 << " " << 123 << 'x' << 0x64;
    REQUIRE(buf.toString() == string("0 123x100"));
}

TEST_CASE("testLogStreamIntegerLimits")
{
    LogStream os;
    const LogStream::Buffer& buf = os.buffer();
    os << -2147483647;
    REQUIRE(buf.toString() == string("-2147483647"));
    os << static_cast<int>(-2147483647 - 1);
    REQUIRE(buf.toString() == string("-2147483647-2147483648"));
    os << ' ';
    os << 2147483647;
    REQUIRE(buf.toString() == string("-2147483647-2147483648 2147483647"));
    os.resetBuffer();

    os << std::numeric_limits<int16_t>::min();
    REQUIRE(buf.toString() == string("-32768"));
    os.resetBuffer();

    os << std::numeric_limits<int16_t>::max();
    REQUIRE(buf.toString() == string("32767"));
    os.resetBuffer();

    os << std::numeric_limits<int32_t>::min();
    REQUIRE(buf.toString() == string("-2147483648"));
    os.resetBuffer();

    os << std::numeric_limits<int32_t>::max();
    REQUIRE(buf.toString() == string("2147483647"));
    os.resetBuffer();

    os << std::numeric_limits<int64_t>::min();
    REQUIRE(buf.toString() == string("-9223372036854775808"));
    os.resetBuffer();

    os << std::numeric_limits<int64_t>::max();
    REQUIRE(buf.toString() == string("9223372036854775807"));
    os.resetBuffer();

    os << std::numeric_limits<uint16_t>::min();
    REQUIRE(buf.toString() == string("0"));
    os.resetBuffer();

    os << std::numeric_limits<uint16_t>::max();
    REQUIRE(buf.toString() == string("65535"));
    os.resetBuffer();

    os << std::numeric_limits<uint32_t>::min();
    REQUIRE(buf.toString() == string("0"));
    os.resetBuffer();

    os << std::numeric_limits<uint32_t>::max();
    REQUIRE(buf.toString() == string("4294967295"));
    os.resetBuffer();

    os << std::numeric_limits<uint64_t>::min();
    REQUIRE(buf.toString() == string("0"));
    os.resetBuffer();

    os << std::numeric_limits<uint64_t>::max();
    REQUIRE(buf.toString() == string("18446744073709551615"));
    os.resetBuffer();

    int16_t a = 0;
    int32_t b = 0;
    int64_t c = 0;
    os << a;
    os << b;
    os << c;
    REQUIRE(buf.toString() == string("000"));
}

TEST_CASE("testFormatSI")
{
    REQUIRE(formatSI(100499) == string("100k"));
    REQUIRE(formatSI(INT64_MAX) == string("9.22E"));
}