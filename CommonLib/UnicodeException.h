#pragma once

#include "Exception.h"

namespace mws { namespace common {

class EncodingException
	:
	public Exception
{
public:
	EncodingException()
	:
		Exception(_C("Encoding Exception"))
	{

	}

};

class DecodingException
	:
	public Exception
{
public:
	DecodingException()
	:
		Exception(_C("Decoding Exception"))
	{

	}
};

}} //mws::common
