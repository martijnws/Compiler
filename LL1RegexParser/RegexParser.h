#pragma once

#include <LL1ParserLib/ParserDriver.h>
#include <LL1ParserLib/ParserTable.h>
#include <RegexGrammar/Grammar.h>
#include <RegexLexer/Lexer.h>
#include <RegexSyntaxTreeLib/SyntaxTreeBuilder.h>
#include <CommonLib/Buffer.h>
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

template<typename BufferT>
class RegexParser
{
public:
	using Buffer = BufferT;
	using Char = typename Buffer::Char;
	using IStream =  std::basic_istream<Char>;

	RegexParser(IStream& is_)
		: _buf(is_)

	{
	
	}

	//TODO: make void and handle exception in caller (fix unittests appropriately)
	bool parse()
	{
        static grammar::Grammar& gRE = regex::getRegexGrammar();
        static grammar::Grammar& gCC = regex::getCharClassGrammar();

        static ParserTable parserTableRE(gRE, regex::REToken::max() + 1);
        static ParserTable parserTableCC(gCC, regex::CCToken::max() + 1);

		try
		{
			using CCLexer = regex::CCLexer<Buffer>;
			using RELexer = regex::RELexer<Buffer>;

             // sub parser setup
            ParserDriver<CCLexer>::SubParserMap ccSubParserCol;
			CCLexer ccLexer(_buf);
			ParserDriver<CCLexer> ccParser(ccLexer, _astBuilder, gCC, parserTableCC, ccSubParserCol);

            ParserDriver<RELexer>::SubParserMap reSubParserCol;
			RELexer reLexer(_buf);
            reSubParserCol.insert(std::make_pair(regex::REToken::Enum::CharClassB, &ccParser));
			ParserDriver<RELexer> reParser(reLexer, _astBuilder, gRE, parserTableRE, reSubParserCol);

			reParser.parse();
			return true;
		}
		catch(const common::Exception& e)
		{
			stdOut << _C("Exception: ") << e.what() << std::endl;
			return false;
		}
	}

	Buffer                   _buf;
	regex::SyntaxTreeBuilder _astBuilder;
};

}}}
