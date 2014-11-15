#pragma once

#include <unordered_map>

namespace mws {

class NFANode
{
public:
    typedef std::unordered_multimap<char, NFANode*> Map;

    Map _transitionMap;
};

class NFA
{
public:
    static const char E = -1;

    NFANode* _s;
    NFANode* _f;
};

} //mws