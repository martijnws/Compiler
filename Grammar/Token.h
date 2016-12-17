#pragma once

#include <CommonLib/CodePoint.h>
#include <cstdint>

namespace mws { namespace grammar {

struct Token
{
	enum Enum { Invalid = 0 };

	using Type = uint8_t;
};

}}