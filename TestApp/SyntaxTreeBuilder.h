#pragma once

#include "ParserHandler.h"
#include <stack>
#include <cassert>

namespace mws { namespace arith {

class SyntaxTreeBuilder
	:
	public ParserHandler
{
public:
    void onEof() override;
	void onE() override;
	void onF(const Token& t_) override;
};

}}
