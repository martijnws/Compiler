#pragma once

#include <map>
#include <list>

namespace mws {

template<typename Item>
std::size_t getAcceptType(Item* d_)
{
    std::size_t regex = -1;
    for (auto n : d_->_items)
    {
        if (n->_accept)
        {
            regex = std::min(regex, n->_regex);
        }
    }

    return regex;
}

template<typename Item>
void traverse(Item* d_, std::set<Item*>& visitSet_, const std::function<void(Item*)>& func_)
{
    func_(d_);

    for (const auto& kvpair : d_->_transitionMap)
    {
        auto d = kvpair.second;

        if (visitSet_.insert(d).second)
        {
            traverse(d, visitSet_, func_);
        }
    }
}

template<typename Item>
std::list<std::list<Item*>> getInitialClassList(Item* d_, std::map<Item*, Item*>& classRepresentativeMap_)
{
    std::map<std::size_t, std::list<Item*>> classMap;
    std::list<std::list<Item*>> classList;

    auto callback = [&classMap, &classRepresentativeMap_](Item* d_)
    {
        std::size_t type = getAcceptType(d_);
        auto itr = classMap.find(type);
        if (itr == classMap.end())
        {
            auto res = classMap.insert(std::make_pair(type, std::list<Item*>()));
            itr = res.first;
        }

        auto& clsList = itr->second;
        clsList.push_back(d_);
        // first item in list is representative
        assert(classRepresentativeMap_.find(d_) == classRepresentativeMap_.end());
        classRepresentativeMap_[d_] = clsList.front();
    };

    std::set<Item*> visitSet;
    visitSet.insert(d_);
    traverse<Item>(d_, visitSet, callback);

    for (const auto& kvpair : classMap)
    {
        classList.push_back(std::move(kvpair.second));
    }    

    return classList;
}

template<typename Item>
void minimize(Item* d_, const std::set<RangeKey, RangeKey::Less>& rkSet_)
{
    std::map<Item*, Item*> classRepresentativeMap;

    std::list<std::list<Item*>> classList = getInitialClassList(d_, classRepresentativeMap);

    for (std::size_t size = 0; size != classList.size(); )
    {
        size = classList.size();

        for (const auto& rk : rkSet_)
        {
            std::list<std::list<Item*>> classListNew;

            for (const auto& cls : classList)
            {
                std::map<Item*, std::list<Item*>> classMap;

                for (Item* d : cls)
                {
                    Item* dNext = nullptr;
                    auto itrNxt = d->_transitionMap.find(rk);
                    if (itrNxt != d->_transitionMap.end())
                    {
                        dNext = itrNxt->second;
                        assert(classRepresentativeMap.find(dNext) != classRepresentativeMap.end());
                        dNext = classRepresentativeMap[dNext];
                    }

                    auto itrCls = classMap.find(dNext);
                    if (itrCls == classMap.end())
                    {
                        auto res = classMap.insert(std::make_pair(dNext, std::list<Item*>()));
                        itrCls = res.first;
                    }

                    auto& clsList = itrCls->second;
                    clsList.push_back(d);
                }

                for (const auto& kvpair : classMap)
                {
                    classListNew.push_back(std::move(kvpair.second));
                }
            }

            classList.swap(classListNew);
        }

        //reset representative map
        classRepresentativeMap.clear();
        for (const auto& cls : classList)
        {
            for (Item* d : cls)
            {   
                assert(classRepresentativeMap.find(d) == classRepresentativeMap.end());
                classRepresentativeMap[d] = cls.front();
            }
        }
    }

    std::map<Item*, Item*> dupToRepMap;
    for (const auto& cls : classList)
    {
        if (cls.size() > 1)
        {
            auto itr = cls.begin();
            auto rep = *itr;
            itr++;

            for ( ; itr != cls.end(); ++itr)
            {
                auto dup = *itr;
                dupToRepMap.insert(std::make_pair(dup, rep));
            }
        }
    }

    auto callback = [&dupToRepMap](Item* d_)
    {
        for (auto& kvpair : d_->_transitionMap)
        {
            auto& dup = kvpair.second;
            auto itr = dupToRepMap.find(dup);
            if (itr != dupToRepMap.end())
            {
                // rewrite old ref with new ref
                auto rep = itr->second;
                dup = rep;
            }
        }
    };

    std::set<Item*> visitSet;
    visitSet.insert(d_);
    traverse<Item>(d_, visitSet, callback);

    for (const auto& kvpair : dupToRepMap)
    {
        delete kvpair.first;
    }
}

}