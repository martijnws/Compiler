#pragma once

#include "ParserHandler.h"

namespace mws { namespace td { namespace pr {

struct Token;

class SyntaxTreeBuilder
	:
	public ParserHandler
{
public:
	virtual void onEof();
	virtual void onChoice();
	virtual void onConcat();
	virtual void onZeroToMany();
	virtual void onSymbol(const Token& t_);
	virtual void onCharClassBeg();
	virtual void onCharClassEnd();
	virtual void onNegate();
	virtual void onRangeList();
	virtual void onRange();
	virtual void onRangeSep();
};

}}}

