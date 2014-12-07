#pragma once

#include "DFAInfo.h"
#include "DFANode.h"
#include <cassert>
#include <set>

namespace mws {

template<typename Item>
class DFATraits;

template<>
class DFATraits<DFAInfo>
{ 
public:
    using Item = DFAInfo;
    using DFANode = mws::DFANode<Item>;
   
    static void c_closure(const DFAInfo* n_, char c_, DFANode* d_)
    {
        assert(c_ != NFA::E);

        if (n_->_lexeme == c_)
        {
            d_->_items.insert(n_->_followPos.begin(), n_->_followPos.end());
        }
    }

    static void e_closure(const DFAInfo* n_, DFANode* d_)
    {
    
    }

    static DFATraits<DFAInfo>::DFANode* e_closure(DFANode* d_)
    {
        return d_;
    }

    static std::set<char> getTransitionCharSet(const DFANode* d_)
    {
        std::set<char> charSet;
        for (auto n : d_->_items)
        {
            assert(n->_lexeme != NFA::E);
            if (!n->_followPos.empty())
            {
                charSet.insert(n->_lexeme);
            }
        }

        return charSet;
    }

    static DFATraits<DFAInfo>::DFANode* createStartNode(const DFAInfo* n_)
    {
        auto d = new DFANode();
        d->_items.insert(n_->_firstPos.begin(), n_->_firstPos.end());
        return d;
    }
};

}