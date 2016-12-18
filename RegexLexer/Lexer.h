#pragma once

#include "Token.h"
#include <CommonLib/Buffer.h>
#include <CommonLib/CodePointEsc.h>
#include <unordered_map>
#include <boost/circular_buffer.hpp>

namespace mws { namespace regex {

class RETokenPolicy
{
public:
	using Token = REToken;
   
	RETokenPolicy()
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

	template<typename Lexer>
	REToken::Enum postProcess(Lexer& lexer_, Token::Enum type_) const
	{
		return type_;	
	}

private:
	std::unordered_map<CodePoint, Token::Enum> _map;
};

class CCTokenPolicy
{
public:
	using Token = CCToken;

	CCTokenPolicy()
	{
		_map[CP('^' )] = Token::Enum::CharClassNeg;
		_map[CP('-' )] = Token::Enum::RngSep;
		_map[CP(']' )] = Token::Enum::Invalid;
	}

	CCToken::Enum type(CodePoint cp_) const
	{
		auto itr = _map.find(cp_);
		return (itr != _map.end() ? itr->second : Token::Enum::Symbol);
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
			const auto cp = buf.next();
			const auto end = buf.pos();
			buf.retract(end - beg);

			if (cp == CP(']') // -]
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

template<typename Buffer, typename TokenPolicy>
class Lexer
{
	friend typename TokenPolicy;

public:
	using Token = typename TokenPolicy::Token;

	Lexer(Buffer& buf_)
		:
		_buf(buf_), _lookBehind(2)
	{
		
	}

	Token next()
	{
		Token t;

		const auto beg = _buf.pos();
		auto cp = _buf.next();
		t._type = _map.type(cp);

		if (t._type == Token::Enum::Invalid)
		{
			_buf.retract(_buf.pos() - beg);
			return t;
		}

		if (cp != CP('\\'))
		{
			t._type = _map.postProcess(*this, t._type);
		}
		else
		{
			cp = _buf.next();

			auto cpEsc = cp;
			const auto validEsc = CodePointFromEsc(cp, cpEsc);

			if (validEsc)
			{
				t._type = Token::Enum::Symbol;
				cp = cpEsc;
			}
			else
			{
				auto type = _map.type(cp);
				if (type == Token::Enum::Symbol || cp == CP('\0'))
				{
					throw common::Exception(_C("invalid escape sequence"));
				}

				t._type = Token::Enum::Symbol;
			}
		}

		t._lexeme = cp;

		_lookBehind.push_back(t._type);
		_lastTokenSize = _buf.pos() - beg;

		return t;
	}

private:
	using TokenEnum = typename Token::Enum;

	Buffer&                           _buf;
	boost::circular_buffer<TokenEnum> _lookBehind;
	const TokenPolicy                 _map;
	std::size_t                       _lastTokenSize = 0;
};

template<typename BufferT>
using RELexer = Lexer<BufferT, RETokenPolicy>;

template<typename BufferT>
using CCLexer = Lexer<BufferT, CCTokenPolicy>;

}}
