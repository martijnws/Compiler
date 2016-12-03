#pragma once

#include <CommonLib/CodePoint.h>
#include <cstdint>

namespace mws { namespace grammar {

struct Token
{
    using Type = uint8_t;

    static const Type None = -1;
    static const Type Eof = 0;

	Type      _type;
	CodePoint _lexeme;
};

}}