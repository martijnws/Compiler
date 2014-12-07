#pragma once

#include <unordered_map>

namespace mws {

class NFANode
{
public:
    typedef std::unordered_multimap<char, NFANode*> Map;

    NFANode() : _accept(false), _regex(0){}

    Map         _transitionMap;
    bool        _accept;
    std::size_t _regex;
};

class NFA
{
public:
    static const char E = -1;

    NFANode* _s;
    NFANode* _f;
};

} //mws