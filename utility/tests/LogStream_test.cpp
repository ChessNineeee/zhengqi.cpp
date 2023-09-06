//
// Created by 70903 on 2023/9/5.
//
#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.h"
#include "utility/LogStream.h"
#include <string>
using namespace zhengqi::utility;

TEST_CASE("testLogStreamBooleans") {
  LogStream os;
  const LogStream::Buffer &buf = os.buffer();

  REQUIRE(buf.toString() == string(""));
  os << true;
  REQUIRE(buf.toString() == string("1"));
  os << '\n';
  REQUIRE(buf.toString() == string("1\n"));
  os << false;
  REQUIRE(buf.toString() == string("1\n0"));
}

TEST_CASE("testLogStreamIntegers") {
  LogStream os;
  const LogStream::Buffer &buf = os.buffer();

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

TEST_CASE("testLogStreamIntegerLimits") {
  LogStream os;
  const LogStream::Buffer &buf = os.buffer();
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

TEST_CASE("testLogStreamFloats")
{
    LogStream os;
    const LogStream::Buffer& buf = os.buffer();

    os << 0.0;
    REQUIRE(buf.toString() == "0");
    os.resetBuffer();

    os << 1.0;
    REQUIRE(buf.toString() == "1");
    os.resetBuffer();

    os << 0.1;
    REQUIRE(buf.toString() == "0.1");
    os.resetBuffer();

    os << 0.05;
    REQUIRE(buf.toString() == "0.05");
    os.resetBuffer();

    os << 0.15;
    REQUIRE(buf.toString() == "0.15");
    os.resetBuffer();

    double a = 0.1;
    os << a;
    REQUIRE(buf.toString() == "0.1");
    os.resetBuffer();

    double b = 0.05;
    os << b;
    REQUIRE(buf.toString() == "0.05");
    os.resetBuffer();

    double c = 0.15;
    os << c;
    REQUIRE(buf.toString() == "0.15");
    os.resetBuffer();

    os << a+b;
    REQUIRE(buf.toString() == "0.15");
    os.resetBuffer();

    os << 1.23456789;
    REQUIRE(buf.toString() == "1.23456789");
    os.resetBuffer();

    os << 1.234567;
    REQUIRE(buf.toString() == "1.234567");
    os.resetBuffer();

    os << -123.456;
    REQUIRE(buf.toString() == "-123.456");
    os.resetBuffer();
}

TEST_CASE("testLogStreamVoid")
{
    LogStream os;
    const LogStream::Buffer& buf = os.buffer();

    os << static_cast<void*>(0);
    REQUIRE(buf.toString() == "0x0");
    os.resetBuffer();

    os << reinterpret_cast<void*>(8888);
    REQUIRE(buf.toString() == "0x22B8");
    os.resetBuffer();
}

TEST_CASE("testLogStreamStrings")
{
    LogStream os;
    const LogStream::Buffer& buf = os.buffer();

    os << "Hello ";
    REQUIRE(buf.toString() == "Hello ");

    string zhengqi = "zhengqi";
    os << zhengqi;
    REQUIRE(buf.toString() == "Hello zhengqi");
}

TEST_CASE("testLogStreamStringFmts")
{
    LogStream os;
    const LogStream::Buffer& buf = os.buffer();

    os << Fmt("%4d", 1);
    REQUIRE(buf.toString() == "   1");
    os.resetBuffer();

    os << Fmt("%4.2f", 1.2);
    REQUIRE(buf.toString() == "1.20");
    os.resetBuffer();

    os << Fmt("%4.2f", 1.2) << Fmt("%4d", 43);
    REQUIRE(buf.toString() == "1.20  43");
    os.resetBuffer();
}

TEST_CASE("testLogStreamLong")
{
    LogStream os;
    const LogStream::Buffer& buf = os.buffer();

    for (int i = 0; i < 399; ++i)
    {
        os << "123456789 ";
        REQUIRE(buf.length() == 10*(i+1));
        REQUIRE(buf.avail() == 4000 - 10*(i+1));
    }

    os << "abcdefghi ";
    REQUIRE(buf.length() == 3990);
    REQUIRE(buf.avail() == 10);

    os << "abcdefghi";
    REQUIRE(buf.length() == 3999);
    REQUIRE(buf.avail() == 1);
}

TEST_CASE("testFormatSI") {
    REQUIRE(formatSI(0) == string("0"));
    REQUIRE(formatSI(999) == string("999"));
    REQUIRE(formatSI(1000) == string("1.00k"));
    REQUIRE(formatSI(1001) == string("1.00k"));
    REQUIRE(formatSI(1023) == string("1.02k"));
    REQUIRE(formatSI(9990) == string("9.99k"));
    REQUIRE(formatSI(9994) == string("9.99k"));
    REQUIRE(formatSI(9995) == string("10.0k"));
    REQUIRE(formatSI(10000) == string("10.0k"));
    REQUIRE(formatSI(10049) == string("10.0k"));
    REQUIRE(formatSI(10050) == string("10.1k"));
    REQUIRE(formatSI(99900) == string("99.9k"));
    REQUIRE(formatSI(99949) == string("99.9k"));
    REQUIRE(formatSI(99950) == string("100k"));
    REQUIRE(formatSI(100499) == string("100k"));
    REQUIRE(formatSI(100501) == string("101k"));
    REQUIRE(formatSI(999499) == string("999k"));
    REQUIRE(formatSI(999500) == string("1.00M"));
    REQUIRE(formatSI(1004999) == string("1.00M"));
    REQUIRE(formatSI(1005001) == string("1.01M"));
    REQUIRE(formatSI(INT64_MAX) == string("9.22E"));
}

TEST_CASE("testFormatIEC") {
    REQUIRE(formatIEC(0) == string("0"));
    REQUIRE(formatIEC(1023) == string("1023"));
    REQUIRE(formatIEC(1024) == string("1.00Ki"));
    REQUIRE(formatIEC(1025) == string("1.00Ki"));
    REQUIRE(formatIEC(10234) == string("9.99Ki"));
    REQUIRE(formatIEC(10235) == string("10.0Ki"));
    REQUIRE(formatIEC(10240) == string("10.0Ki"));
    REQUIRE(formatIEC(10291) == string("10.0Ki"));
    REQUIRE(formatIEC(10292) == string("10.1Ki"));
    REQUIRE(formatIEC(102348) == string("99.9Ki"));
    REQUIRE(formatIEC(102349) == string("100Ki"));
    REQUIRE(formatIEC(102912) == string("100Ki"));
    REQUIRE(formatIEC(102913) == string("101Ki"));
    REQUIRE(formatIEC(1022976) == string("999Ki"));
    REQUIRE(formatIEC(1047552) == string("1023Ki"));
    REQUIRE(formatIEC(1047961) == string("1023Ki"));
    REQUIRE(formatIEC(1048063) == string("1023Ki"));
    REQUIRE(formatIEC(1048064) == string("1.00Mi"));
    REQUIRE(formatIEC(1048576) == string("1.00Mi"));
    REQUIRE(formatIEC(10480517) == string("9.99Mi"));
    REQUIRE(formatIEC(10480518) == string("10.0Mi"));
    REQUIRE(formatIEC(INT64_MAX) == string("8.00Ei"));
}