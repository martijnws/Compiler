#pragma once

#include "Token.h"
#include <CommonLib/Buffer.h>
#include <boost/circular_buffer.hpp>

namespace mws { namespace td { namespace LL1 {

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
		_map['-' ] = Token::Type::RngSep;
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
				throw common::Exception("invalid escape sequence");
			}
			else
			{
				t._type = Token::Type::Symbol;
			}
		}

		_lookBehind.push_back(t._type);
		return t;
	}

protected:
	static const TokenTypeMapT          _map;
	BufferT&                            _buf;
	boost::circular_buffer<Token::Type> _lookBehind;
};

template<typename DerivedT, typename BufferT, typename TokenTypeMapT>
const TokenTypeMapT Lexer<DerivedT, BufferT, TokenTypeMapT>::_map = TokenTypeMapT();

// TODO: restructure these templates (make postProcess internal to Lexer)

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
		//Handle context dependencies of RngSep::lexeme

		if (t_._type == Token::Type::RngSep)
		{
			//peek 1 char ahead
			char c = _buf.next();
			_buf.retract(1);

			if (_map.type(c) == Token::Type::CharClassE // -]
				||
				_lookBehind.size() == 0 // [-
				||
				_lookBehind.size() == 1 && _lookBehind.back() == Token::Type::CharClassNeg) // [^-
			{
				t_._type = Token::Type::Symbol;
			}
		}
		else 
		if (t_._type == Token::Type::CharClassNeg)
		{
			if (_lookBehind.size() > 0) // [...^
			{
				t_._type = Token::Type::Symbol;
			}
		}
	}
};

}}}
