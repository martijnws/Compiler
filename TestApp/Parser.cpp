#include "stdafx.h"
#include "Parser.h"

#define S(s_) _SExt(s_)

namespace mws { namespace arith {

using TI = Lexer::TokenInfo;

std::vector<TI> _tokenInfoCol =
{
	TI( S("\\+"),           Token::Enum::Add ),
	TI( S("[0-9]"),         Token::Enum::Num ),
	TI( S("\\0"),           Token::Enum::Eof ),
};

void Parser::parse()
{
	static const auto& g = getGrammar();

	static td::SLR::ParserTable parserTable(g, Token::max() + 1);

	 // sub parser setup
	td::SLR::ParserDriver<Lexer>::SubParserMap subParserCol;
	Lexer lexer(_is, _tokenInfoCol);
	td::SLR::ParserDriver<Lexer> parser(lexer, _astBuilder, g, parserTable, subParserCol);

	parser.parse();
}

}}
