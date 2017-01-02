#pragma once

#include <Grammar/Grammar.h>

namespace mws { namespace arith {

enum class NonTerminal { S, E, F };

grammar::Grammar& getGrammar();

}}
