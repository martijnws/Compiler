#pragma once

#include "Grammar.h"

namespace mws { namespace grammar {

void follow(Grammar& grammar_);
// sub grammars have have alternative Eof token
void follow(Grammar& grammar_, const grammar::Token::Type& tokEof_);

}}

