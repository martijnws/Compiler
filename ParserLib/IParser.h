#pragma once

#include <Grammar/Grammar.h>
#include <map>

namespace mws { namespace td { 

class IParser
{
public:
    virtual void parse(grammar::Token& cur_) = 0;
};

using SubParserMap = std::map<grammar::Token::Type, IParser*>;

}}