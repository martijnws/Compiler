#pragma once

namespace mws { namespace td { namespace pr {

struct Token;

class ParserHandler
{
public:
	virtual void onEof() = 0;
	virtual void onChoice() = 0;
	virtual void onConcat() = 0;
	virtual void onZeroToMany() = 0;
	virtual void onSymbol(const Token& t_) = 0;
	virtual void onCharClassBeg() = 0;
	virtual void onCharClassEnd() = 0;
	virtual void onNegate() = 0;
	virtual void onRangeList() = 0;
	virtual void onRange() = 0;
	virtual void onRangeSep() = 0;
};

}}}
