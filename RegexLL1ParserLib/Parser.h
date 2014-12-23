#pragma once

#include "ParserDriver.h"
#include "SLRParserDriver.h"
#include <RegexGrammar/Grammar.h>
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
        static grammar::Grammar& gRE = regex::getRegexGrammar();
        static grammar::Grammar& gCC = regex::getCharClassGrammar();

        static ParserTable parserTableRE(gRE, regex::Token::Enum::Max);
        static ParserTable parserTableCC(gCC, regex::Token::Enum::Max);

        static SLRParserTable slrParserTableRE(gRE, regex::Token::Enum::Max);
        static SLRParserTable slrParserTableCC(gCC, regex::Token::Enum::Max);

		try
		{
            // TODO: remove
            // sub parser setup
            SubParserMap ccSubParserCol;
            SLRParserDriver<regex::CharClassLexer> ccParser(_astBuilder, gCC, slrParserTableCC, ccSubParserCol);

            SubParserMap reSubParserCol;
            reSubParserCol.insert(std::make_pair(regex::Token::Enum::CharClassB, &ccParser));
            SLRParserDriver<regex::RegexLexer> slr_ccParser(_astBuilder, gRE, slrParserTableRE, reSubParserCol);

            slr_ccParser.parse(_buf, _cur);

             // sub parser setup
            /*SubParserMap ccSubParserCol;
            ParserDriver<regex::CharClassLexer> ccParser(_astBuilder, gCC, parserTableCC, ccSubParserCol);

            SubParserMap reSubParserCol;
            reSubParserCol.insert(std::make_pair(regex::Token::Enum::CharClassB, &ccParser));
			ParserDriver<regex::RegexLexer> parser(_astBuilder, gRE, parserTableRE, reSubParserCol);

			parser.parse(_buf, _cur);*/
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
