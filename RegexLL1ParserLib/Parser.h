#pragma once

#include "RegexParser.h"
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

using Buf = common::Buffer<256>;

class Parser
{
public:
	Parser(std::istream& is_)
		: _buf(is_), _cur({ Token::Type::None, '\0' })

	{
	
	}

	//TODO: make void and handle exception in caller (fix unittests appropriately)
	bool parse()
	{
		try
		{
			RegexParser<Buf> parser(_buf, _cur, _astBuilder);

			parser.parse();
			return true;
		}
		catch(const common::Exception& e)
		{
			std::cout << "Exception: " << e.what() << std::endl;
			return false;
		}
	}

private:
	Buf   _buf;
	Token _cur;
	ast::SyntaxTreeBuilder _astBuilder;
};

}}}
