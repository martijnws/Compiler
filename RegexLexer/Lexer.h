#pragma once

#include "Token.h"
#include <CommonLib/Buffer.h>
#include <boost/circular_buffer.hpp>

namespace mws { namespace regex {

class RegexTokenTypeMap
{
public:
   
	RegexTokenTypeMap()
	{
		for (std::size_t i = 0; i < sizeof(_map)/sizeof(Token); ++i)
		{
			_map[i] = Token::Enum::Symbol;
		}

		_map['|' ] = Token::Enum::Choice;
        _map['?' ] = Token::Enum::ZeroOrOne;
		_map['*' ] = Token::Enum::ZeroToMany;
        _map['+' ] = Token::Enum::OneToMany;
		_map['(' ] = Token::Enum::SubExprB;
		_map[')' ] = Token::Enum::SubExprE;
		_map['[' ] = Token::Enum::CharClassB;
		_map[']' ] = Token::Enum::CharClassE;
		_map['\0'] = Token::Enum::Eof;
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
    static const grammar::Token::Type Eof = Token::Enum::CharClassE;
	CharClassTokenTypeMap()
	{
		for (std::size_t i = 0; i < sizeof(_map)/sizeof(Token); ++i)
		{
			_map[i] = Token::Enum::Symbol;
		}

		_map['^' ] = Token::Enum::CharClassNeg;
		_map['-' ] = Token::Enum::RngSep;
		_map[']' ] = Eof;
		_map['\0'] = Eof;
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

	grammar::Token next()
	{
		auto c = _buf.next();

		grammar::Token t = { Token::None, c };

		if (c != '\\')
		{
			t._type = _map.type(c);
			auto& derived = static_cast<DerivedT&>(*this);
			derived.postProcess(t);
		}
		else
		{
			c = _buf.next();

			Token::Type type = _map.type(c);
            // supported escape seq is '\\' or '\<operator>'
			if (type == Token::Enum::Symbol && c != '\\' || type == Token::Enum::Eof)
			{
				throw common::Exception("invalid escape sequence");
			}
			else
			{
				t._type = Token::Enum::Symbol;
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

	void postProcess(grammar::Token& t_)
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

	void postProcess(grammar::Token& t_)
	{
		//Handle context dependencies of RngSep::lexeme

		if (t_._type == Token::Enum::RngSep)
		{
			//peek 1 char ahead
			char c = _buf.next();
			_buf.retract(1);

			if (_map.type(c) == Token::Enum::CharClassE // -]
				||
				_lookBehind.size() == 0 // [-
				||
				_lookBehind.size() == 1 && _lookBehind.back() == Token::Enum::CharClassNeg) // [^-
			{
				t_._type = Token::Enum::Symbol;
			}
		}
		else 
		if (t_._type == Token::Enum::CharClassNeg)
		{
			if (_lookBehind.size() > 0) // [...^
			{
				t_._type = Token::Enum::Symbol;
			}
		}
	}
};

}}
