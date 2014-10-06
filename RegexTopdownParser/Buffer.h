#pragma once

#include <istream>

namespace mws { namespace td { namespace bt {

class Buffer
{
	static const size_t Size = 4096;

public:
	Buffer(std::istream& is_)
		:
		_is(is_), _cur(_buf)
	{
		memset(_buf, '\0', sizeof(_buf));

		//load first buffer
		_is.read(_cur, Size);
		_cur[_is.gcount()] = '\0';
	}
	
	char cur() const
	{
		return *_cur;
	}

	void next()
	{
		if (!_is && *_cur == '\0')
			throw std::exception();

		if (*++_cur == '\0')
		{
			// reached end of first buffer?
			if (_cur == _buf + Size)
			{
				//load second buffer
				_is.read(_cur, Size);
				_cur[_is.gcount()] = '\0';
			}
			else
			if (_cur == _buf + Size * 2)
			{
				// circle back to first buffer
				_cur = _buf;
				//load first buffer
				_is.read(_cur, Size);
				_cur[_is.gcount()] = '\0';
			}
		}
	}

private:
	char _buf[2 * Size + 8];
	std::istream& _is;
	char* _cur;
};

}}}
