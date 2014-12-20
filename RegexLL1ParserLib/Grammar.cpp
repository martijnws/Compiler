#include "Grammar.h"
#include "ParserHandler.h"
#include "ParserDriver.h"
#include "CharClassParser.h"
#include <Grammar/Grammar.h>
#include <Grammar/First.h>
#include <Grammar/Follow.h>
#include <functional>

namespace mws { namespace td { namespace LL1 {

enum NonTerminal { Start, Choice, ChoiceT, Concat, ConcatT, Term, QuantifierO, ZeroOrOne, ZeroToMany, OneToMany, Factor, CharClass, CharClassNeg, RngConcat, RngConcatT, Rng, RngT, Option };

using namespace grammar;

using T = Token::Enum;
using N = NonTerminal;
using H = ParserHandler;

template<typename P>
grammar::Parse parse()
{
    return [](grammar::Handler& h_, const grammar::Grammar& grammar_, const GrammarSymbol& startSymbol_, common::Buffer& buf_, grammar::Token& t_)
    {
        P parser(buf_, t_);

        parser.parse(h_, grammar_, startSymbol_);
    };
}

grammar::Grammar& getGrammar()
{
    using e = Production;

    static grammar::Grammar g = { 

    NT("Start",       { { n(N::Choice) },
                        { e() } }),

    NT("Choice",      { { n(N::Concat), n(N::ChoiceT) } }),

    NT("ChoiceT",     { { t(T::Choice), n(N::Concat, a(&H::onChoice)), n(N::ChoiceT) },
                        { e() } }),

    NT("Concat",      { { n(N::Term), n(N::ConcatT) } }),

    NT("ConcatT",     { { n(N::Term, a(&H::onConcat)), n(N::ConcatT) },
                        { e() } }),

    NT("Term",        { { n(N::Factor), n(N::QuantifierO) } }),

    NT("QuantifierO", { { n(N::ZeroOrOne) },
                        { n(N::ZeroToMany) },
                        { n(N::OneToMany) },
                        { e() } }),

    NT("ZeroOrOne",   { { t(T::ZeroOrOne, a(&H::onZeroOrOne)) } }),

    NT("ZeroToMany",  { { t(T::ZeroToMany, a(&H::onZeroToMany)) } }),

    NT("OneToMany",   { { t(T::OneToMany, a(&H::onOneToMany)) } }),

    NT("Factor",      { { t(T::Symbol, a(&H::onSymbol)) },
                        { t(T::CharClassB, false), n(N::CharClass, a(&H::onCharClass), parse<ParserDriver<CharClassLexer>>()), t(T::CharClassE) },
                        { t(T::SubExprB), n(N::Choice), t(T::SubExprE) } }),

    NT("CharClass",   { { n(N::RngConcat) },
                        { n(N::CharClassNeg) } }),

    NT("CharClassNeg",  { { t(T::CharClassNeg), n(N::RngConcat, a(&H::onNegate)) } }),

    NT("RngConcat",     { { n(N::Rng), n(N::RngConcatT) } }),
                    
    NT("RngConcatT",    { { n(N::Rng, a(&H::onRngConcat)), n(N::RngConcatT) },
                        { e() } }),

    NT("Rng",         { { n(N::Option), n(N::RngT) } }),

    NT("RngT",        { { t(T::RngSep), n(N::Option, a(&H::onRng)) },
                        { e() } }),

    NT("Option",      { { t(T::Symbol, a(&H::onCharClassSymbol)) } }),

    };

    static bool init = false;
    if (!init)
    {
        grammar::first(g);
	    grammar::follow(g);
        init = true;
    }

    return g;
}

}}}