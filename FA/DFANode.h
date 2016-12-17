#pragma once

#include "RangeKey.h"
#include <CommonLib/TokenID.h>
#include <set>
#include <map>
#include <cassert>
#include <iostream>
#include <memory>

namespace mws {

class DFANode
{
public:
    DFANode(TokenID regexID_) : _regexID(regexID_){}

    using Map = std::map<RangeKey, DFANode*>;

    bool accept() const
    {
        return _regexID != InvalidTokenID;
    }

    Map     _transitionMap;
    TokenID _regexID;
};

template<typename Item, typename HashT = std::hash<Item>>
class ItemSet
{
    using _ItemSet = ItemSet<Item, HashT>;

    ItemSet() = delete;
    ItemSet(const _ItemSet&) = delete;
    _ItemSet& operator = (const _ItemSet&) = delete;
    _ItemSet& operator = (_ItemSet&&) = delete;

    friend class Hash;

public:
    using Ptr = _ItemSet*;
    using Set = std::set<Item>;

    // only way to construct ItemSet. This garantuees a hash will be calculated
    ItemSet(Set&& rhs_) : _items(std::move(rhs_))
    {
        calculateHash();
    }

    inline bool empty() const
    {
        return _items.empty();
    }

    class Hash
    {
    public:
     
        std::size_t operator()(const Ptr& itemSet_) const
        {
            return itemSet_->_hash;
        }
    };

    class Pred
    {
    public:
        bool operator()(const Ptr& lhs_, const Ptr& rhs_) const
        {
            if (lhs_->_items.size() != rhs_->_items.size())
            {
                return false;
            }

            Hash hash;
            auto hlhs = hash(lhs_);
            auto hrhs = hash(rhs_);

            if (hlhs != hrhs)
            {
                return false;
            }

            // TODO: this is very inefficient. Most compares will be equal and thus fall through
            // to below full blown item by item comparison.

            return lhs_->_items == rhs_->_items;
        }
    };

private:
   void calculateHash()
    {
        _hash = 0;

        // not a tested and proven hash (its rubbish!) but good enough for now
        HashT hash;
            
        for (const auto& n : _items)
        {
            _hash <<= 1;
            _hash += hash(n);
            _hash >>= 1;
        }
    }

    std::size_t _hash;

public:
    Set         _items;
};

}
