#pragma once

#include <Grammar/Handler.h>

namespace mws { namespace td { namespace LL1 {

class ParserHandler
:
    public grammar::Handler
{
public:
    class Enum
    {
    public:
        enum N { onEof, onChoice, onConcat, onZeroOrOne, onZeroToMany, onOneToMany, onCharClass, onNegate, onRngConcat, onRng };
        enum T { onSymbol, onCharClassSymbol };
    };

	void onNonTerminal(uint8_t type_) override;
	void onTerminal(uint8_t type_, const grammar::Token& t_) override;

    virtual void onEof() = 0;
	virtual void onChoice() = 0;
	virtual void onConcat() = 0;
    virtual void onZeroOrOne() = 0;
	virtual void onZeroToMany() = 0;
    virtual void onOneToMany() = 0;
	virtual void onSymbol(const grammar::Token& t_) = 0;
	virtual void onCharClass() = 0;
	virtual void onNegate() = 0;
    virtual void onRngConcat() = 0;
	virtual void onRng() = 0;
    virtual void onCharClassSymbol(const grammar::Token& t_) = 0;

private:
    
};

}}}
