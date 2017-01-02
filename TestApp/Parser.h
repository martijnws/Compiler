#pragma once

#include "Grammar.h"
#include "SyntaxTreeBuilder.h"
#include "Token.h"
#include <FA/Lexer.h>
#include <SLRParserLib/ParserDriver.h>
#include <SLRParserLib/ParserTable.h>
#include <CommonLib/Buffer.h>
#include <iostream>

namespace mws { namespace arith {

class Parser
{
public:
	using Lexer   = LexerT<Token>;
	using IStream = std::basic_istream<Lexer::Buffer::Char>;

	Parser(IStream& is_)
		: _is(is_)

	{
	
	}

	void parse();

	IStream&          _is;
	SyntaxTreeBuilder _astBuilder;
};

}}
