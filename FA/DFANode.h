#pragma once

#include "RangeKey.h"
#include <set>
#include <map>
#include <cassert>
#include <iostream>
#include <memory>

namespace mws {

class DFANode
{
public:
    DFANode(std::size_t regexID_) : _regexID(regexID_){}

    using Map = std::map<RangeKey, DFANode*, RangeKey::Less>;

    bool accept() const
    {
        return _regexID != -1;
    }

    Map _transitionMap;
    std::size_t _regexID;
};

template<typename Item>
class ItemSet
{
    ItemSet() = delete;
    ItemSet(const ItemSet<Item>&) = delete;
    ItemSet<Item>& operator = (const ItemSet<Item>&) = delete;
    ItemSet<Item>& operator = (ItemSet<Item>&&) = delete;

public:
    using Ptr = ItemSet<Item>*;
    using Set = std::set<const Item*>;

    ItemSet(std::set<const Item*>&& rhs_) : _items(std::move(rhs_))
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
            assert(itemSet_->_hash != 0);
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
            std::size_t hlhs = hash(lhs_);
            std::size_t hrhs = hash(rhs_);

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
        std::hash<const Item*> hash;
            
        for (const Item* n : _items)
        {
            _hash <<= 1;
            _hash += hash(n);
            _hash >>= 1;
        }
    }

public:
    Set         _items;
    std::size_t _hash;
};

}
