#pragma once

#include <set>
#include <map>
#include <cassert>

namespace mws {

template<typename Item>
class DFANode
{
public:
    using ItemSet = std::set<const Item*>;
    using DFANodeMap = std::map<char, DFANode*>;

    DFANode() : _hash(0){}

    void insert(const Item* item_)
    {
        _items.insert(item_);
    }

    void calculateHash()
    {
        // not a tested and proven hash (its rubbish!) but good enough for now
        std::hash<const Item*> hash;
            
        for (const Item* n : _items)
        {
            _hash <<= 1;
            _hash += hash(n);
            _hash >>= 1;
        }
    }

    class Hash
    {
    public:
     
        std::size_t operator()(const DFANode* d_) const
        {
            assert(d_->_hash != 0);
            return d_->_hash;
        }
    };

    class Pred
    {
    public:
        bool operator()(const DFANode* lhs_, const DFANode* rhs_) const
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

            return lhs_->_items == rhs_->_items;
        }
    };

    DFANodeMap  _transitionMap;
    ItemSet     _items;
    std::size_t _hash;
};

}
