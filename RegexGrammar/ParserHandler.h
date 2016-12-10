#pragma once

#include <Grammar/Handler.h>

namespace mws { namespace regex {

struct REToken;
struct CCToken;

class ParserHandler
:
    public grammar::Handler
{
public:
    virtual void onEof() = 0;
	virtual void onChoice() = 0;
	virtual void onConcat() = 0;
    virtual void onZeroOrOne() = 0;
	virtual void onZeroToMany() = 0;
    virtual void onOneToMany() = 0;
	virtual void onSymbol(const REToken& t_) = 0;
	virtual void onCharClass() = 0;
	virtual void onNegate() = 0;
    virtual void onRngConcat() = 0;
	virtual void onRng() = 0;
    virtual void onCharClassSymbol(const CCToken& t_) = 0;
};

}}
