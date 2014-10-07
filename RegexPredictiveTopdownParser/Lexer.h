#pragma once

#include "Buffer.h"
#include "Token.h"
#include <boost/circular_buffer.hpp>

namespace mws { namespace td { namespace pr {

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

template<typename DerivedT, typename BufferT, typename TokenTypeMapT>
class Lexer
{
public:
	Lexer(BufferT& buf_)
		:
		_buf(buf_), _lookBehind(2)
	{
		
	}

	Token next()
	{
		char c = _buf.next();

		Token t = { Token::Type::None, c };

		if (c != '\\')
		{
			t._type = _map.type(c);
			DerivedT& derived = static_cast<DerivedT&>(*this);
			derived.postProcess(t);
		}
		else
		{
			c = _buf.next();

			Token::Type type = _map.type(c);
			if (type == Token::Type::Symbol && c != '\\' || type == Token::Type::Eof)
			{
				throw Exception("invalid escape sequence");
			}
			else
			{
				t._type = Token::Type::Symbol;
			}
		}

		_lookBehind.push_back(t._type);
		return t;
	}

	std::size_t pos() const
	{
		return _buf.pos();
	}

	void retract(std::size_t diff_)
	{
		_buf.retract(diff_);
	}

protected:
	static const TokenTypeMapT _map;
	BufferT&                   _buf;
	boost::circular_buffer<Token::Type> _lookBehind;
};

template<typename DerivedT, typename BufferT, typename TokenTypeMapT>
const TokenTypeMapT Lexer<DerivedT, BufferT, TokenTypeMapT>::_map = TokenTypeMapT();


template<typename BufferT>
class RegexLexer
	: public Lexer<RegexLexer<BufferT>, BufferT, RegexTokenTypeMap>
{
public:
	RegexLexer(BufferT& buf_)
		:
		Lexer(buf_)
	{
		
	}

	void postProcess(Token& t_)
	{
		
	}
};

template<typename BufferT>
class CharClassLexer
	: public Lexer<CharClassLexer<BufferT>, BufferT, CharClassTokenTypeMap>
{
public:
	CharClassLexer(BufferT& buf_)
		:
		Lexer(buf_)
	{
		
	}

	void postProcess(Token& t_)
	{
		//Handle context dependencies of CharClassSep::lexeme

		if (t_._type == Token::Type::CharClassSep)
		{
			//peek 1 char ahead
			char c = _buf.next();
			_buf.retract(1);

			const std::size_t size = _lookBehind.size();

			if (_map.type(c) == Token::Type::CharClassE
				||
				size > 0 && _lookBehind[size - 1] == Token::Type::CharClassB
				||
				size > 1 && _lookBehind[size - 2] == Token::Type::CharClassB && _lookBehind[size - 2] == Token::Type::CharClassNeg)
			{
				t_._type = Token::Type::Symbol;
			}
		}
		else 
		if (t_._type == Token::Type::CharClassNeg)
		{
			const std::size_t size = _lookBehind.size();

			if (size > 0 && _lookBehind[size - 1] != Token::Type::CharClassB)
			{
				t_._type = Token::Type::Symbol;
			}
		}
	}
};

}}}
