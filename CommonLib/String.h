#pragma once

#include <iostream>
#include <sstream>
#include <string>

namespace mws {

//internal format
#define _C(_c) L##_c
#define _S(_c) L##_c

//external format
#define _CExt(_c) _c
#define _SExt(_c) u8##_c

#define stdOut std::wcout

//utf8
using CharExt = char;
//utf16 on windows
using Char = wchar_t;

using String = std::basic_string<Char>;
using StringExt = std::basic_string<CharExt>;

using StringStream = std::basic_stringstream<Char>;
using StringStreamExt = std::basic_stringstream<CharExt>;

using IStream = std::basic_istream<Char>;
using IStreamExt = std::basic_istream<CharExt>;

}