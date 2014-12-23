#pragma once

#include <Grammar/Grammar.h>

namespace mws { namespace regex {

enum ReNonTerminal { Start, Choice, ChoiceT, Concat, ConcatT, Term, QuantifierO, ZeroOrOne, ZeroToMany, OneToMany, Factor };

enum CCNonTerminal { CharClass, CharClassNeg, RngConcat, RngConcatT, Rng, RngT, Option };

grammar::Grammar& getRegexGrammar();
grammar::Grammar& getCharClassGrammar();

}}
