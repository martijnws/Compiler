#pragma once

#include "Grammar.h"

namespace mws { namespace grammar {

// sub grammars have have alternative Eof token
void follow(Grammar& grammar_, const grammar::Token::Type& tokEof_);

}}

