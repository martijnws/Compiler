#pragma once

#include <exception>
#include <string>

namespace mws { namespace td { namespace pr {

class Exception
	:
	std::exception
{
public:
	Exception(const std::string& msg_)
		:
		_msg(msg_)
	{

	}

	virtual const char* what() const
	{
		return _msg.c_str();
	}

private:
	std::string _msg;
};

}}}
