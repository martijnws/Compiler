#pragma once

#include <Grammar/Grammar.h>

namespace mws { namespace regex {

//Note: It is VERY important to use 'enum class' rather than 'enum', otherwise these names will conflict with SyntaxNode derived classes of the same name

enum class ReNonTerminal { Start, Choice, ChoiceT, Concat, ConcatT, Term, QuantifierO, ZeroOrOne, ZeroToMany, OneToMany, Factor };

enum class CCNonTerminal { CharClass, CharClassNeg, RngConcat, RngConcatT, Rng, RngT, Option };

grammar::Grammar& getRegexGrammar();
grammar::Grammar& getCharClassGrammar();

}}
