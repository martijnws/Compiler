#include "stdafx.h"
#include "Grammar.h"
#include "Token.h"
#include "ParserHandler.h"
#include <Grammar/Grammar.h>
#include <Grammar/First.h>
#include <Grammar/Follow.h>
#include <functional>

namespace mws { namespace arith {

using namespace grammar;

using H = ParserHandler;

//SLR Grammar

grammar::Grammar& getGrammar()
{
	using T = Token::Enum;

    using N = NonTerminal;
    using e = Production;

    static grammar::Grammar g = { 

    NT("S",   { { n(N::E) } }),

    NT("E",   { { n(N::E), t(T::Add), n(N::F) },
                { n(N::F) } }),

	NT("F",   { { t(T::Num) } }), 
    };

    static bool init = false;
    if (!init)
    {
        grammar::first(g);
	    grammar::follow(g, T::Eof);
        init = true;
    }

    return g;
}

}}

