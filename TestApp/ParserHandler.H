#pragma once

#include <Grammar/Handler.h>

namespace mws { namespace arith {

struct Token;

class ParserHandler
:
    public grammar::Handler
{
public:
    virtual void onEof() = 0;
	virtual void onE() = 0;
	virtual void onF(const Token& t_) = 0;
};

}}

