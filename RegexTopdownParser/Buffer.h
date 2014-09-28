#pragma once

#include <istream>

class Buffer
{
	static const size_t Size = 4096;

public:
	Buffer(std::istream& is_)
		:
		_is(is_), _cur(_buf), _fwd(_buf + Size * 2 - 1)
	{
		memset(_buf, '\0', sizeof(_buf));
	}
	
	char cur()
	{
		return *_cur;
	}

	char fwd()
	{
		if (!_is && *_fwd == '\0')
			throw std::exception();

		if (*++_fwd == '\0')
		{
			// reached end of first buffer?
			if (_fwd == _buf + Size)
			{
				//load second buffer
				_is.read(_fwd, Size);
				_fwd[_is.gcount()] = '\0';
			}
			else
			if (_fwd == _buf + Size * 2)
			{
				// circle back to first buffer
				_fwd = _buf;
				//load first buffer
				_is.read(_fwd, Size);
				_fwd[_is.gcount()] = '\0';
			}
		}

		return *_fwd;
	}

	void syncCur()
	{
		_cur = _fwd;
	}

private:
	char _buf[2 * Size + 8];
	std::istream& _is;
	char* _cur;
	char* _fwd;
};