//
// Created by 70903 on 2023/9/11.
//
#define CATCH_CONFIG_MAIN

#include "utility/tests/catch_amalgamated.h"

#include "networking/Buffer.h"

using namespace zhengqi::networking;

TEST_CASE("testBufferAppendRetrieve") {
    Buffer buf;
    REQUIRE(buf.readableBytes() == 0);
    REQUIRE(buf.writableBytes() == Buffer::kInitialSize);
    REQUIRE(buf.prependableBytes() == Buffer::kCheapPrepend);

    const std::string str(200, 'x');
    buf.append(str);
    REQUIRE(buf.readableBytes() == str.size());
    REQUIRE(buf.writableBytes() == Buffer::kInitialSize - str.size());
    REQUIRE(buf.prependableBytes() == Buffer::kCheapPrepend);

    const std::string str2 = buf.retrieveAsString(50);
    REQUIRE(str2.size() == 50);
    REQUIRE(buf.readableBytes() == str.size() - str2.size());
    REQUIRE(buf.writableBytes() == Buffer::kInitialSize - str.size());
    REQUIRE(buf.prependableBytes() == Buffer::kCheapPrepend + str2.size());
    REQUIRE(str2 == std::string(50, 'x'));

    buf.append(str);
    REQUIRE(buf.readableBytes() == 2 * str.size() - str2.size());
    REQUIRE(buf.writableBytes() == Buffer::kInitialSize - 2 * str.size());
    REQUIRE(buf.prependableBytes() == Buffer::kCheapPrepend + str2.size());

    const std::string str3 = buf.retrieveAllAsString();
    REQUIRE(str3.size() == 350);
    REQUIRE(buf.readableBytes() == 0);
    REQUIRE(buf.writableBytes() == Buffer::kInitialSize);
    REQUIRE(buf.prependableBytes() == Buffer::kCheapPrepend);
    REQUIRE(str3 == std::string(350, 'x'));
}

TEST_CASE("testBufferGrow") {
    Buffer buf;
    buf.append(std::string(400, 'y'));
    REQUIRE(buf.readableBytes() == 400);

    buf.retrieve(50);
    REQUIRE(buf.readableBytes() == 350);
    REQUIRE(buf.writableBytes() == Buffer::kInitialSize - 400);
    REQUIRE(buf.prependableBytes() == Buffer::kCheapPrepend + 50);

    buf.append(std::string(1000, 'z'));
    REQUIRE(buf.readableBytes() == 1350);
    REQUIRE(buf.writableBytes() == 0);
    REQUIRE(buf.prependableBytes() == Buffer::kCheapPrepend + 50);

    buf.retrieveAll();
    REQUIRE(buf.readableBytes() == 0);
    REQUIRE(buf.writableBytes() == 1400); // kInitialSize * 2
    REQUIRE(buf.prependableBytes() == Buffer::kCheapPrepend);
}

TEST_CASE("testBufferInsideGrow") {
    Buffer buf;
    buf.append(std::string(800, 'y'));
    REQUIRE(buf.readableBytes() == 800);
    REQUIRE(buf.writableBytes() == Buffer::kInitialSize - 800);

    buf.retrieve(500);
    REQUIRE(buf.readableBytes() == 300);
    REQUIRE(buf.writableBytes() == Buffer::kInitialSize - 800);
    REQUIRE(buf.prependableBytes() == Buffer::kCheapPrepend + 500);

    buf.append(std::string(300, 'z'));
    REQUIRE(buf.readableBytes() == 600);
    REQUIRE(buf.writableBytes() == Buffer::kInitialSize - 600);
    REQUIRE(buf.prependableBytes() == Buffer::kCheapPrepend);
}

TEST_CASE("testBufferShrink")
{
    Buffer buf;
    buf.append(std::string(2000, 'y'));
    REQUIRE(buf.readableBytes() == 2000);
    REQUIRE(buf.writableBytes() == 0);
    REQUIRE(buf.prependableBytes() == Buffer::kCheapPrepend);

    buf.retrieve(1500);
    REQUIRE(buf.readableBytes() == 500);
    REQUIRE(buf.writableBytes() == 0);
    REQUIRE(buf.prependableBytes() == Buffer::kCheapPrepend + 1500);

    buf.shrink(0);
    REQUIRE(buf.readableBytes() == 500);
    REQUIRE(buf.writableBytes() == Buffer::kInitialSize - 500);
    REQUIRE(buf.retrieveAllAsString() == std::string(500, 'y'));
    REQUIRE(buf.prependableBytes() == Buffer::kCheapPrepend);
}

TEST_CASE("testBufferReadInt")
{
    Buffer buf;
    buf.append("HTTP");

    REQUIRE(buf.readableBytes() == 4);
    REQUIRE(buf.peekInt8() == 'H');
    int top16 = buf.peekInt16();
    REQUIRE(top16 == 'H' * 256 + 'T');
    REQUIRE(buf.peekInt32() == top16 * 65536 + 'T' * 256 + 'P');

    REQUIRE(buf.readInt8() == 'H');
    REQUIRE(buf.readInt16() == 'T' * 256 + 'T');
    REQUIRE(buf.readInt8() == 'P');
    REQUIRE(buf.readableBytes() == 0);
    // 调用了 retrieveAll()
    REQUIRE(buf.writableBytes() == Buffer::kInitialSize);

    buf.appendInt8(-1);
    buf.appendInt16(-2);
    buf.appendInt32(-3);
    REQUIRE(buf.readableBytes() == 7);
    REQUIRE(buf.readInt8() == -1);
    REQUIRE(buf.readInt16() == -2);
    REQUIRE(buf.readInt32() == -3);
}

TEST_CASE("testBufferFindEOL")
{
    Buffer buf;
    buf.append(std::string(100000, 'x'));
    const char *null = nullptr;
    REQUIRE(buf.findEOL() == null);
    REQUIRE(buf.findEOL(buf.peek() + 90000) == null);
}

void output(Buffer &&buf, const void *inner) {
    Buffer newbuf(std::move(buf));
    REQUIRE(inner == newbuf.peek());
}

TEST_CASE("testBufferMove")
{
    Buffer buf;
    buf.append("ZHENGQI", 7);

    const void *inner = buf.peek();
    output(std::move(buf), inner);
}