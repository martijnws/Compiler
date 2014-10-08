#pragma once

#include <RegexLL1ParserLib/Token.h>
#include <RegexLL1ParserLib/ParserHandler.h>

namespace mws { namespace ast {

class SyntaxTreeBuilder
	:
	public td::LL1::ParserHandler
{
public:
	virtual void onEof();
	virtual void onChoice();
	virtual void onConcat();
	virtual void onZeroToMany();
	virtual void onSymbol(const td::LL1::Token& t_);
	virtual void onCharClassBeg();
	virtual void onCharClassEnd();
	virtual void onNegate();
	virtual void onRangeList();
	virtual void onRange();
	virtual void onRangeSep();
};

}}
