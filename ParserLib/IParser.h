#pragma once

#include <Grammar/Grammar.h>
#include <map>

namespace mws { namespace td { 

class IParser
{
public:
    virtual void parse() = 0;
};

}}