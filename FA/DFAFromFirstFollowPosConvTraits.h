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
    
    static void c_closure(const Item* n_, const RangeKey& rk_, std::set<const Item*>& itemSet_)
    {
        assert(rk_._l != NFA::E);
       
        RangeKey::Less less;
        if (!less(n_->_lexeme, rk_._l) && !less(rk_._l, n_->_lexeme))
        {
            assert(!n_->_followPos.empty());
            itemSet_.insert(n_->_followPos.begin(), n_->_followPos.end());
        }
    }

    static void e_closure(const Item* n_, std::set<const Item*>& itemSet_)
    {
    
    }

    static std::set<const Item*> e_closure(std::set<const Item*>& itemSet_)
    {
        return itemSet_;
    }

    static std::set<RangeKey, RangeKey::Less> getTransitionCharSet(const std::set<const Item*>& itemSet_)
    {
        std::set<RangeKey, RangeKey::Less> rkSet;
        for (auto n : itemSet_)
        {
            assert(n->_lexeme._l != NFA::E);
            if (!n->_followPos.empty())
            {
                rkSet.insert(n->_lexeme);
            }
        }

        return rkSet;
    }

    static std::set<const Item*> createStartNode(const Item* n_)
    {
        std::set<const Item*> itemSet;
        itemSet.insert(n_->_firstPos.begin(), n_->_firstPos.end());
        return itemSet;
    }
};

}