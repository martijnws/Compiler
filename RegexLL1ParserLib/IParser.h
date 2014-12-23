#pragma once

#include <Grammar/Grammar.h>
#include <map>

namespace mws { namespace td { namespace LL1 {

class IParser
{
public:
    virtual void parse(common::Buffer& buf_, grammar::Token& cur_) = 0;
};

using SubParserMap = std::map<grammar::Token::Type, IParser*>;

}}}