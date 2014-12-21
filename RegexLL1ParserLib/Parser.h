#pragma once

#include "ParserDriver.h"
#include "ParserTable.h"
#include <RegexGrammar/Grammar.h>
#include <RegexGrammar/ParserID.h>
#include <RegexLexer/Lexer.h>
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
        static grammar::Grammar& g = regex::getGrammar();
        static ParserTable parserTable(g, regex::Token::Enum::Max);

        // sub parser setup
        std::vector<std::pair<IParser*, bool>> ccSubParserCol;
        ParserDriver<regex::CharClassLexer> ccParser(_astBuilder, g, parserTable, ccSubParserCol);

		try
		{
            std::vector<std::pair<IParser*, bool>> reSubParserCol = { std::make_pair(&ccParser, true) };
			ParserDriver<regex::RegexLexer> parser(_astBuilder, g, parserTable, reSubParserCol);

			parser.parse(_buf, _cur);
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
