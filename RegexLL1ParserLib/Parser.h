#pragma once

#include "ParserDriver.h"
#include "ParserTable.h"
#include "Grammar.h"
#include <CommonLib/Buffer.h>
#include <SyntaxTreeLib/SyntaxTreeBuilder.h>
#include <iostream>

// Notes on grammar
//
// A list containing zero or more items
//
// List     = Item List | empty
//
// A list containing at least 1 item
//
// List     = Item ListTail
// ListTail = Item ListTail | e
//
//  An optional item
//
// OptItem  = Item | e
//
//
// Whenever a non-terminal has more than 1 possible production we track the lexer position so that we can retry the next production if one fails
//

namespace mws { namespace td { namespace LL1 {

class Parser
{
public:
	Parser(std::istream& is_)
		: _buf(is_), _cur({ grammar::Token::None, 0 })

	{
	
	}

	//TODO: make void and handle exception in caller (fix unittests appropriately)
	bool parse()
	{
        static grammar::Grammar& g = getGrammar();
        static ParserTable parserTable(g, LL1::Token::Enum::Max);

		try
		{
			ParserDriver<RegexLexer> parser(_buf, _cur);

			parser.parse(_astBuilder, g);
			return true;
		}
		catch(const common::Exception& e)
		{
			std::cout << "Exception: " << e.what() << std::endl;
			return false;
		}
	}

	common::Buffer _buf;
	grammar::Token _cur;
	ast::SyntaxTreeBuilder _astBuilder;
};

}}}
