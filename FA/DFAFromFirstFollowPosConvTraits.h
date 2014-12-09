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
   
    static void c_closure(const DFAInfo* n_, const RangeKey& rk_, DFANode* d_)
    {
        assert(rk_._l != NFA::E);
       
        RangeKey::Less less;
        if (!less(n_->_lexeme, rk_._l) && !less(rk_._l, n_->_lexeme))
        {
            assert(!n_->_followPos.empty());
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

    static std::set<RangeKey, RangeKey::Less> getTransitionCharSet(const DFANode* d_)
    {
        std::set<RangeKey, RangeKey::Less> rkSet;
        for (auto n : d_->_items)
        {
            assert(n->_lexeme._l != NFA::E);
            if (!n->_followPos.empty())
            {
                rkSet.insert(n->_lexeme);
            }
        }

        return rkSet;
    }

    static DFATraits<DFAInfo>::DFANode* createStartNode(const DFAInfo* n_)
    {
        auto d = new DFANode();
        d->_items.insert(n_->_firstPos.begin(), n_->_firstPos.end());
        return d;
    }
};

}