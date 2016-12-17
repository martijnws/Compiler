#include "Grammar.h"
#include "ParserHandler.h"
#include <Grammar/Grammar.h>
#include <Grammar/First.h>
#include <Grammar/Follow.h>
#include <RegexLexer/Token.h>
#include <functional>

namespace mws { namespace regex {

using namespace grammar;

using H = ParserHandler;

grammar::Grammar& getRegexGrammar()
{
	using T = REToken::Enum;

    using N = ReNonTerminal;
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
                        { t(T::CharClassB), /*n(CCNonTerminal::CharClass),*/ t(T::CharClassE, a(&H::onCharClass)) },
                        { t(T::SubExprB), n(N::Choice), t(T::SubExprE) } })
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

grammar::Grammar& getCharClassGrammar()
{
	using T = CCToken::Enum;
    using N = CCNonTerminal;
    using e = Production;

    static grammar::Grammar g = { 

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
	    grammar::follow(g, T::Invalid);
        init = true;
    }

    return g;
}

}}