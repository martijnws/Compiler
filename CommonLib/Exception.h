#pragma once

#include "String.h"
#include <exception>

namespace mws { namespace common {

class IException
{
public:
	virtual const String& what() const = 0;
};

class Exception
:
	public IException
{
public:
	Exception(const String& msg_)
		:
		_msg(msg_)
	{

	}

	const String& what() const override
	{
		return _msg;
	}

private:
	String _msg;
};

}}
