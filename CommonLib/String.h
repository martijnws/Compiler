#pragma once

#include <iostream>
#include <sstream>
#include <string>

//#define FMT_INTERNAL_UTF16
#define FMT_EXTERNAL_UTF16

namespace mws {

//internal format
#if   defined (FMT_INTERNAL_UTF8)
#define _C(_c) _c
#define _S(_c) u8##_c
using Char = char;
#elif defined (FMT_INTERNAL_UTF16)
#define _C(_c) u##_c
#define _S(_c) u##_c
using Char = char16_t;
#elif defined (FMT_INTERNAL_UTF32)
#define _C(_c) U##_c
#define _S(_c) U##_c
using Char = char32_t;
#else
//utf16 on windows
#define _C(_c) L##_c
#define _S(_c) L##_c
using Char = wchar_t;
#endif

//external format
#if   defined (FMT_EXTERNAL_UTF8)
#define _CExt(_c) _c
#define _SExt(_c) u8##_c
using CharExt = char;
#elif defined (FMT_EXTERNAL_UTF16)
#define _CExt(_c) u##_c
#define _SExt(_c) u##_c
using CharExt = char16_t;
#elif defined (FMT_EXTERNAL_UTF32)
#define _CExt(_c) U##_c
#define _SExt(_c) U##_c
using CharExt = char32_t;
#else
//utf16 on windows
#define _CExt(_c) L##_c
#define _SExt(_c) L##_c
using CharExt = wchar_t;
#endif

#define stdOut std::wcout

using String = std::basic_string<Char>;
using StringExt = std::basic_string<CharExt>;

using StringStream = std::basic_stringstream<Char>;
using StringStreamExt = std::basic_stringstream<CharExt>;

using IStream = std::basic_istream<Char>;
using IStreamExt = std::basic_istream<CharExt>;

}