#pragma once

#include "Token.h"
#include <CommonLib/Buffer.h>
#include <unordered_map>
#include <boost/circular_buffer.hpp>

namespace mws { namespace regex {

class RETokenTypeMap
{
public:
	using Token = REToken;
   
	RETokenTypeMap()
	{
		_map[CP('|') ] = Token::Enum::Choice;
        _map[CP('?') ] = Token::Enum::ZeroOrOne;
		_map[CP('*') ] = Token::Enum::ZeroToMany;
        _map[CP('+') ] = Token::Enum::OneToMany;
		_map[CP('(') ] = Token::Enum::SubExprB;
		_map[CP(')') ] = Token::Enum::SubExprE;
		_map[CP('[') ] = Token::Enum::CharClassB;
		_map[CP(']') ] = Token::Enum::CharClassE;
		_map[CP('\0')] = Token::Enum::Eof;
	}

	REToken::Enum type(CodePoint cp_) const
	{
		auto itr = _map.find(cp_);
		return (itr != _map.end() ? itr->second : Token::Enum::Symbol);
	}

	bool isValid(CodePoint cp_) const
	{
		return true;
	}

	bool isValidEscaped(CodePoint cp_,  Token::Enum type_) const
	{
		return cp_ == CP('\\') || (type_ != Token::Enum::Symbol && type_ != Token::Enum::Eof);
	}

	template<typename Lexer>
	REToken::Enum postProcess(Lexer& lexer_, Token::Enum type_) const
	{
		return type_;	
	}

private:
	std::unordered_map<CodePoint, Token::Enum> _map;
};

class CCTokenTypeMap
{
public:
	using Token = CCToken;

	CCTokenTypeMap()
	{
		_map[CP('^' )] = Token::Enum::CharClassNeg;
		_map[CP('-' )] = Token::Enum::RngSep;
	}

	CCToken::Enum type(CodePoint cp_) const
	{
		auto itr = _map.find(cp_);
		return (itr != _map.end() ? itr->second : Token::Enum::Symbol);
	}

	bool isValid(CodePoint cp_) const
	{
		return cp_ != CP(']');
	}

	bool isValidEscaped(CodePoint cp_,  Token::Enum type_) const
	{
		return cp_ == CP('\\') || cp_ == CP(']') || type_ != Token::Enum::Symbol;
	}

	template<typename Lexer>
	CCToken::Enum postProcess(Lexer& lexer_, Token::Enum type_) const
	{
		auto& buf = lexer_._buf;
		const auto& lookBehind = lexer_._lookBehind;

		//Handle context dependencies of RngSep::lexeme
		auto type = type_;

		if (type_ == Token::Enum::RngSep)
		{
			//peek 1 char ahead
			const auto beg = buf.pos();
			const auto c = buf.next();
			const auto end = buf.pos();
			buf.retract(end - beg);

			if (c == CP(']') // -]
				||
				lookBehind.size() == 0 // [-
				||
				lookBehind.size() == 1 && lookBehind.back() == Token::Enum::CharClassNeg) // [^-
			{
				type = Token::Enum::Symbol;
			}
		}
		else 
		if (type_ == Token::Enum::CharClassNeg)
		{
			if (lookBehind.size() > 0) // [...^
			{
				type = Token::Enum::Symbol;
			}
		}

		return type;
	}
private:
	std::unordered_map<CodePoint, Token::Enum> _map;
};

template<typename BufferT, typename TokenTypeMapT>
class Lexer
{
	friend typename TokenTypeMapT;

public:
	using Token     = typename TokenTypeMapT::Token;
	using TokenType = typename Token::Type;

	Lexer(BufferT& buf_)
		:
		_buf(buf_), _lookBehind(2)
	{
		
	}

	typename TokenTypeMapT::Token next()
	{
		Token t;

		const auto beg = _buf.pos();
		auto c = _buf.next();
		if (!_map.isValid(c))
		{
			_buf.retract(_buf.pos() - beg);
			t._type = Token::Enum::Invalid;
			return t;
		}

		if (c != CP('\\'))
		{
			t._type = _map.type(c);
			t._type = _map.postProcess(*this, t._type);
		}
		else
		{
			c = _buf.next();

			//TODO: set correct lexeme on Token for escaped characters

			auto type = _map.type(c);
            // supported escape seq is '\\' or '\<operator>'
			if (!_map.isValidEscaped(c, type))
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
		_lastTokenSize = _buf.pos() - beg;

		return t;
	}

protected:
	BufferT&                          _buf;
	boost::circular_buffer<TokenType> _lookBehind;
	const TokenTypeMapT               _map;
	std::size_t                       _lastTokenSize = 0;
};

template<typename BufferT>
using RELexer = Lexer<BufferT, RETokenTypeMap>;

template<typename BufferT>
using CCLexer = Lexer<BufferT, CCTokenTypeMap>;

}}
