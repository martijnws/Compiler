#include "DFAMinimize.h"
#include "DFANode.h"
#include <map>
#include <list>
#include <functional>

namespace mws {

void traverse(DFANode* d_, std::set<DFANode*>& visitSet_, const std::function<void(DFANode*)>& func_)
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

std::list<std::list<DFANode*>> getInitialClassList(DFANode* d_, std::map<DFANode*, DFANode*>& classRepresentativeMap_)
{
    std::map<std::size_t, std::list<DFANode*>> classMap;
    std::list<std::list<DFANode*>> classList;

    auto callback = [&classMap, &classRepresentativeMap_](DFANode* d_)
    {
        auto itr = classMap.find(d_->_regexID);
        if (itr == classMap.end())
        {
            auto res = classMap.insert(std::make_pair(d_->_regexID, std::list<DFANode*>()));
            itr = res.first;
        }

        auto& clsList = itr->second;
        clsList.push_back(d_);
        // first item in list is representative
        assert(classRepresentativeMap_.find(d_) == classRepresentativeMap_.end());
        classRepresentativeMap_[d_] = clsList.front();
    };

    std::set<DFANode*> visitSet;
    visitSet.insert(d_);
    traverse(d_, visitSet, callback);

    for (const auto& kvpair : classMap)
    {
        classList.push_back(std::move(kvpair.second));
    }    

    return classList;
}

void minimize(DFANode* d_, const std::set<RangeKey>& rkSet_)
{
    std::map<DFANode*, DFANode*> classRepresentativeMap;

    std::list<std::list<DFANode*>> classList = getInitialClassList(d_, classRepresentativeMap);

    for (std::size_t size = 0; size != classList.size(); )
    {
        size = classList.size();

        for (const auto& rk : rkSet_)
        {
            std::list<std::list<DFANode*>> classListNew;

            for (const auto& cls : classList)
            {
                std::map<DFANode*, std::list<DFANode*>> classMap;

                for (DFANode* d : cls)
                {
                    DFANode* dNext = nullptr;
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
                        auto res = classMap.insert(std::make_pair(dNext, std::list<DFANode*>()));
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
            for (DFANode* d : cls)
            {   
                assert(classRepresentativeMap.find(d) == classRepresentativeMap.end());
                classRepresentativeMap[d] = cls.front();
            }
        }
    }

    std::map<DFANode*, DFANode*> dupToRepMap;
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

    auto callback = [&dupToRepMap](DFANode* d_)
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

    std::set<DFANode*> visitSet;
    visitSet.insert(d_);
    traverse(d_, visitSet, callback);

    for (const auto& kvpair : dupToRepMap)
    {
        delete kvpair.first;
    }
}

}