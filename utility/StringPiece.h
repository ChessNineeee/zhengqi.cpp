//
// Created by zhengqi on 2023/9/4.
//
// Taken from PCRE pcre_stringpiece.h
//
// Copyright (c) 2005, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Author: Sanjay Ghemawat
//
// A string like object that points into another piece of memory.
// Useful for providing an interface that allows clients to easily
// pass in either a "const char*" or a "string".
//
// Arghh!  I wish C++ literals were automatically of type "string".

// StringPiece是PCRE的实现，是对字符串的一个proxy类(设计模式之代理模式)
// 很多时候，当传入一个字符串到函数时，往往只是读取字符串时
// 若使用std::string，当实参为const char *时，会分配内存并拷贝该字符串以生成一个std::string
// 当某个接口参数是接受字符串类型时，为了减少不必要的开销
// 该类型可以接受const char *，std::string，减少冗余代码编写
// 用以实现高效的字符串传递，这里既可以用const char*，也可以用std::string类型作为参数，并且不涉及内存拷贝

#include "Types.h"

#ifndef CPP_STRINGPIECE_H
#define CPP_STRINGPIECE_H

namespace zhengqi
{
    namespace utility
    {
        /// For passing C-style string argument to a function.
        class StringArg // copyable
        {
        public:
            StringArg(const char* str)
            : str_(str)
            {}

            StringArg(const string& str)
            : str_(str.c_str())
            {}
        private:
            const char* str_;
        };

        class StringPiece
        {
        private:
            const char* ptr_;
            int length_;
        public:
            // We provide non-explicit singleton constructors so users can pass
            // in a "const char*" or a "string" wherever a "StringPiece" is
            // expected.
            StringPiece()
            : ptr_(NULL), length_(0) {}
            StringPiece(const char* str)
            : ptr_(NULL), length_(static_cast<int>(strlen(ptr_))) {}
            StringPiece(const unsigned char* str)
            : ptr_(reinterpret_cast<const char*>(str)),
            length_(static_cast<int>(strlen(ptr_))) {}
            StringPiece(const string& str)
            : ptr_(str.data()), length_(static_cast<int>(str.size())) {}
            StringPiece(const char* offset, int len)
            :ptr_(offset), length_(len) {}

            // data() may return a pointer to a buffer with embedded NULs, and the
            // returned buffer may or may not be null terminated.  Therefore it is
            // typically a mistake to pass data() to a routine that expects a NUL
            // terminated string.  Use "as_string().c_str()" if you really need to do
            // this.  Or better yet, change your routine so it does not rely on NUL
            // termination.

            const char* data() const { return ptr_; }
            int size() const { return length_; }
            bool empty() const { return length_ == 0; }
            const char* begin() const { return ptr_; }
            const char* end() const { return ptr_ + length_; }

            void clear() { ptr_ = NULL; length_ = 0; }
            void set(const char* buffer, int len) { ptr_ = buffer; length_ = len; }
            void set(const char* str) {
                ptr_ = str;
                length_ = static_cast<int>(strlen(str));
            }
            void set(const void* buffer, int len) {
                ptr_ = reinterpret_cast<const char*>(buffer);
                length_ = len;
            }

            char operator[](int i) const { return ptr_[i]; }

            void remove_prefix(int n) {
                ptr_ += n;
                length_ -= n;
            }

            void remove_suffix(int n) {
                length_ -= n;
            }

            bool operator==(const StringPiece& x) const {
                return ((length_ == x.length_) &&
                        (memcmp(ptr_, x.ptr_, length_) == 0));
            }
            bool operator!=(const StringPiece& x) const {
                return !(*this == x);
            }

        #define STRINGPIECE_BINARY_PREDICATE(cmp,auxcmp)                             \
          bool operator cmp (const StringPiece& x) const {                           \
            int r = memcmp(ptr_, x.ptr_, length_ < x.length_ ? length_ : x.length_); \
            return ((r auxcmp 0) || ((r == 0) && (length_ cmp x.length_)));          \
          }
                    STRINGPIECE_BINARY_PREDICATE(<,  <);
                    STRINGPIECE_BINARY_PREDICATE(<=, <);
                    STRINGPIECE_BINARY_PREDICATE(>=, >);
                    STRINGPIECE_BINARY_PREDICATE(>,  >);
        #undef STRINGPIECE_BINARY_PREDICATE

            int compare(const StringPiece& x) const {
                int r = memcmp(ptr_, x.ptr_, length_ < x.length_ ? length_ : x.length_);
                if (r == 0) {
                    if (length_ < x.length_) r = -1;
                    else if (length_ > x.length_) r = +1;
                }
                return r;
            }

            string as_string() const {
                return string(data(), size());
            }

            void CopyToString(string* target) const {
                target->assign(ptr_, length_);
            }

            // Does "this" start with "x"
            bool starts_with(const StringPiece& x) const {
                return ((length_ >= x.length_) && (memcmp(ptr_, x.ptr_, x.length_) == 0));
            }
        };
    }
}

#endif //CPP_STRINGPIECE_H
