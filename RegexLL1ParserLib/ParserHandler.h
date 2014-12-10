#pragma once

namespace mws { namespace td { namespace LL1 {

struct Token;

class ParserHandler
{
public:
	virtual void onEof() = 0;
	virtual void onChoice() = 0;
	virtual void onConcat() = 0;
    virtual void onZeroOrOne() = 0;
	virtual void onZeroToMany() = 0;
    virtual void onOneToMany() = 0;
	virtual void onSymbol(const Token& t_) = 0;
	virtual void onCharClass() = 0;
	virtual void onNegate() = 0;
    virtual void onRngConcat() = 0;
	virtual void onRng() = 0;
    virtual void onCharClassSymbol(const td::LL1::Token& t_) = 0;
};

}}}
