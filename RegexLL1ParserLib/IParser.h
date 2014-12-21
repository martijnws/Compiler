#pragma once

#include <Grammar/Grammar.h>

namespace mws { namespace td { namespace LL1 {

class IParser
{
public:
    virtual void parse(common::Buffer& buf_, grammar::Token& cur_) = 0;
    virtual void parse(const grammar::GrammarSymbol& startSymbol_, common::Buffer& buf_, grammar::Token& cur_) = 0;
};

}}}