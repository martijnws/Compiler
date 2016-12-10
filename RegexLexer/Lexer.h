#pragma once

#include "Token.h"
#include <CommonLib/Buffer.h>
#include <unordered_map>
#include <boost/circular_buffer.hpp>

namespace mws { namespace regex {

class RegexTokenTypeMap
{
public:
    //static const Token::Enum Eof = Token::Enum::Eof;
	using Token = REToken;
   
	RegexTokenTypeMap(CodePoint cpEoc_)
	{
		//for (std::size_t i = 0; i < sizeof(_map)/sizeof(Token::Type); ++i)
		//{
		//	_map[i] = Token::Enum::Symbol;
		//}

		_map[CP('|') ] = Token::Enum::Choice;
        _map[CP('?') ] = Token::Enum::ZeroOrOne;
		_map[CP('*') ] = Token::Enum::ZeroToMany;
        _map[CP('+') ] = Token::Enum::OneToMany;
		_map[CP('(') ] = Token::Enum::SubExprB;
		_map[CP(')') ] = Token::Enum::SubExprE;
		_map[CP('[') ] = Token::Enum::CharClassB;
		_map[CP(']') ] = Token::Enum::CharClassE;

		_map[cpEoc_  ] = Token::Enum::Eoc;
		// In case Eoc == Eof, Eof takes precedence.
		_map[CP('\0')] = Token::Enum::Eof;
	}

	REToken::Enum type(CodePoint cp_) const
	{
		auto itr = _map.find(cp_);
		return (itr != _map.end() ? itr->second : Token::Enum::Symbol);
		//return _map[c_];
	}

private:
	std::unordered_map<CodePoint, Token::Enum> _map;
	//Token::Type _map[256];
};

class CharClassTokenTypeMap
{
public:
	using Token = CCToken;

    //static const Token::Enum Eof = Token::Enum::CharClassE;

	CharClassTokenTypeMap(CodePoint cpEoc_)
	{
		//for (std::size_t i = 0; i < sizeof(_map)/sizeof(Token::Type); ++i)
		//{
		//	_map[i] = Token::Enum::Symbol;
		//}

		_map[CP('^' )] = Token::Enum::CharClassNeg;
		_map[CP('-' )] = Token::Enum::RngSep;
		//_map[CP(']' )] = Eof;
		//_map[CP('\0')] = Eof;
		_map[CP('\0')] = Token::Enum::Eof;
		_map[cpEoc_]   = Token::Enum::Eoc;
	}

	CCToken::Enum type(CodePoint cp_) const
	{
		auto itr = _map.find(cp_);
		return (itr != _map.end() ? itr->second : Token::Enum::Symbol);
		//return _map[c_];
	}

private:
	std::unordered_map<CodePoint, CCToken::Enum> _map;
	//Token::Type _map[256];
};

template<typename DerivedT, typename BufferT, typename TokenTypeMapT>
class Lexer
{
public:
	//static const Token::Enum Eof = TokenTypeMapT::Eof;
	using Token = typename TokenTypeMapT::Token;
	using TokenType = typename Token::Type;

	Lexer(BufferT& buf_, CodePoint cpEoc_)
		:
		_buf(buf_), _lookBehind(2), _map(cpEoc_)
	{
		
	}

	typename TokenTypeMapT::Token next()
	{
		const auto posOld = _buf.pos();
		auto c = _buf.next();

		Token t; //= { c, Token::None };

		if (c != CP('\\'))
		{
			t._type = _map.type(c);
			auto& derived = static_cast<DerivedT&>(*this);
			t._type = derived.postProcess(t._type);
		}
		else
		{
			c = _buf.next();

			//TODO: set correct lexeme on Token for escaped characters

			auto type = _map.type(c);
            // supported escape seq is '\\' or '\<operator>'
			if (type == Token::Enum::Symbol && c != CP('\\') || type == Token::Enum::Eof)
			{
				throw common::Exception(_C("invalid escape sequence"));
			}
			else
			{
				t._type = Token::Enum::Symbol;
			}
		}

		t._lexeme = c;

		_lookBehind.push_back(t._type);
		_lastTokenSize = _buf.pos() - posOld;

		return t;
	}

	void retractLast()
	{
		if (_lastTokenSize == 0)
		{
			throw common::Exception(_C("Cannot retract more than 1 token"));
		}

		_buf.retract(_lastTokenSize);
		_lastTokenSize = 0;
	}

protected:
	BufferT&                          _buf;
	boost::circular_buffer<TokenType> _lookBehind;
	const TokenTypeMapT               _map;
	std::size_t                       _lastTokenSize = 0;
};

//template<typename DerivedT, typename BufferT, typename TokenTypeMapT>
//const TokenTypeMapT Lexer<DerivedT, BufferT, TokenTypeMapT>::_map = TokenTypeMapT();

// TODO: restructure these templates (make postProcess internal to Lexer)

template<typename BufferT>
class RegexLexer
	: public Lexer<RegexLexer<BufferT>, BufferT, RegexTokenTypeMap>
{
public:
	using Token = RegexTokenTypeMap::Token;

	RegexLexer(BufferT& buf_, CodePoint cpEoc_)
		:
		Lexer(buf_, cpEoc_)
	{
		
	}

	RegexTokenTypeMap::Token::Enum postProcess(Token::Enum type_)
	{
		return type_;	
	}
};

template<typename BufferT>
class CharClassLexer
	: public Lexer<CharClassLexer<BufferT>, BufferT, CharClassTokenTypeMap>
{
public:
	using Token = CharClassTokenTypeMap::Token;

	CharClassLexer(BufferT& buf_, CodePoint cpEoc_)
		:
		Lexer(buf_, cpEoc_)
	{
		
	}

	CharClassTokenTypeMap::Token::Enum postProcess(Token::Enum type_)
	{
		//Handle context dependencies of RngSep::lexeme
		auto type = type_;

		if (type_ == Token::Enum::RngSep)
		{
			//peek 1 char ahead
			const auto posOld = _buf.pos();
			const auto c = _buf.next();
			const auto posNew = _buf.pos();
			_buf.retract(posNew - posOld);

			if (_map.type(c) == Token::Enum::Eoc // -]
				||
				_lookBehind.size() == 0 // [-
				||
				_lookBehind.size() == 1 && _lookBehind.back() == Token::Enum::CharClassNeg) // [^-
			{
				type = Token::Enum::Symbol;
			}
		}
		else 
		if (type_ == Token::Enum::CharClassNeg)
		{
			if (_lookBehind.size() > 0) // [...^
			{
				type = Token::Enum::Symbol;
			}
		}

		return type;
	}
};

}}
