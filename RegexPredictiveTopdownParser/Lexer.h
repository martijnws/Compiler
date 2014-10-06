#pragma once

#include "Buffer.h"
#include "Token.h"

namespace mws { namespace td { namespace pr {

// Forward declares

template<typename BufferT, typename TokenTypeMapT>
class Lexer;

class RegexTokenTypeMap;

template<typename BufferT>
using RegexLexer = Lexer<BufferT, RegexTokenTypeMap>;

class CharClassTokenTypeMap;

template<typename BufferT>
using CharClassLexer = Lexer<BufferT, CharClassTokenTypeMap>;

class RegexTokenTypeMap
{
public:
	RegexTokenTypeMap()
	{
		for (std::size_t i = 0; i < sizeof(_map)/sizeof(Token); ++i)
		{
			_map[i] = Token::Type::Symbol;
		}

		_map['|' ] = Token::Type::Choice;
		_map['*' ] = Token::Type::ZeroToMany;
		_map['(' ] = Token::Type::SubExprB;
		_map[')' ] = Token::Type::SubExprE;
		_map['[' ] = Token::Type::CharClassB;
		_map[']' ] = Token::Type::CharClassE;
		_map['\0'] = Token::Type::Eof;
	}

	Token::Type type(char c_) const
	{
		return _map[c_];
	}

private:
	Token::Type _map[256];
};

class CharClassTokenTypeMap
{
public:
	CharClassTokenTypeMap()
	{
		for (std::size_t i = 0; i < sizeof(_map)/sizeof(Token); ++i)
		{
			_map[i] = Token::Type::Symbol;
		}

		_map['^' ] = Token::Type::CharClassNeg;
		_map['-' ] = Token::Type::CharClassSep;
		_map[']' ] = Token::Type::CharClassE;
		_map['\0'] = Token::Type::Eof;
	}

	Token::Type type(char c_) const
	{
		return _map[c_];
	}

private:
	Token::Type _map[256];
};

class ILexer
{
public:
	virtual ~ILexer() {}

	virtual Token       next() = 0;
	virtual std::size_t pos() const = 0;
	virtual void        retract(std::size_t diff_) = 0;
};

template<typename BufferT, typename TokenTypeMapT>
class Lexer
	:
	public ILexer
{
public:
	Lexer(BufferT& buf_)
		:
		_buf(buf_)
	{
		
	}

	Token next() override
	{
		char c = _buf.next();

		if (c != '\\')
		{
			Token t = { _map.type(c), c };
			return t;
		}
		
		c = _buf.next();

		Token::Type type = _map.type(c);
		if (type == Token::Type::Symbol && c != '\\' || type == Token::Type::Eof)
		{
			throw Exception("invalid escape sequence");
		}
		else
		{
			Token t = { Token::Type::Symbol, c };
			return t;
		}
	}

	/*Token cur() const override
	{
		return _cur;
	}
*/
	std::size_t pos() const override
	{
		return _buf.pos();
	}

	void retract(std::size_t diff_) override
	{
		_buf.retract(diff_);
	}

private:
	static const TokenTypeMapT _map;
	BufferT& _buf;
	//Token _cur;
};

template<typename BufferT, typename TokenTypeMapT>
const TokenTypeMapT Lexer<BufferT, TokenTypeMapT>::_map = TokenTypeMapT();

}}}
