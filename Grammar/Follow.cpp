#include "Follow.h"
#include <iostream>

namespace mws { namespace grammar {

inline std::ostream& operator << (std::ostream& os_, const std::set<uint8_t>& set_)
{
	for (auto i : set_)
	{
		os_ << (int)i << " ";
	}

	return os_;
}

struct Context
{
    Context(std::size_t size_)
    :
        _ntFollowSetsCount(std::vector<uint8_t>(size_, 0)), _followSetToNtsMap(std::vector<std::vector<uint8_t>>(size_))
    {
    
    }

    std::vector<uint8_t>              _ntFollowSetsCount;
    std::vector<std::vector<uint8_t>> _followSetToNtsMap;
};


void follow(grammar::Grammar& grammar_, uint8_t nttHead_, grammar::Production& prod_, Context& ctx_)
{
	assert(nttHead_ >= 0 && nttHead_ < grammar_.size());
	
	auto& ntHead = grammar_[nttHead_];

	// start with true because last grammar symbol requires follow(ntHead)
	bool requiresFollow = true;
	std::set<uint8_t> followSet;

    // i must be signed to detect >= 0 condition
	for (int64_t i = prod_._gsList.size() - 1; i >= 0; --i)
	{
		const auto& gs = prod_._gsList[i];
		if (gs._isTerminal)
		{
			requiresFollow = false;
			followSet.clear();
			followSet.insert(gs._type);
			continue;
		}

		// update nt.follow with followSet prepared in previous iteration
		auto& nt = grammar_[gs._type];
		nt._follow.insert(followSet.begin(), followSet.end());

		// make a note that follow(ntHead) must be added in a later pass
		if (requiresFollow && gs._type != nttHead_ /*prevent H -> aH recursion*/)
		{
			ctx_._followSetToNtsMap[nttHead_].push_back(gs._type);
			++ctx_._ntFollowSetsCount[gs._type];
		}

		// prepare followSet for symbol to the left (next iteration)

		// 1) as soon as we encounter a non-empty deriving symbol, follow(ntHead) will no longer be required
		requiresFollow &= nt.derivesEmpty();
		if (!requiresFollow)
		{
			followSet.clear();
		}

		// 2) first of current = follow of symbol on the left
		nt.getFirst(followSet);
	}
}

void follow(grammar::Grammar& grammar_, const grammar::Token::Type& tokEof_)
{
	std::cout << "Follow:" << std::endl;

    Context ctx(grammar_.size());

	auto& ntStart = grammar_[0];
	ntStart._follow.insert(tokEof_);

	// first pass: add first sets
	for (auto i = 0; i < grammar_.size(); ++i)
	{
		for (auto& prod : grammar_[i]._prodList)
		{
			follow(grammar_, i, prod, ctx);
		}
	}

	// second pass: add followsets
	std::vector<uint8_t> completeFollowSets;
	
	for (auto i = 0; i < ctx._ntFollowSetsCount.size(); ++i)
	{
		if (ctx._ntFollowSetsCount[i] == 0)
		{
			completeFollowSets.push_back(i);
		}
	}

	for (size_t i = 0; i < completeFollowSets.size(); ++i)
	{
		auto nttComplete = completeFollowSets[i];
		const auto& ntComplete = grammar_[nttComplete];

		std::vector<uint8_t>& incompleteFollowSets = ctx._followSetToNtsMap[nttComplete];
		for (const auto nttIncomplete : incompleteFollowSets)
		{
			// add follow(nttComplete) to follow(nttIncomplete)
			auto& ntIncomplete = grammar_[nttIncomplete];
			ntIncomplete._follow.insert(ntComplete._follow.begin(), ntComplete._follow.end());

			auto& count = ctx._ntFollowSetsCount[nttIncomplete];
			assert(count > 0);

			if (--count == 0)
			{
				completeFollowSets.push_back(nttIncomplete);
			}
		}
	}

	for (const auto& nt : grammar_)
	{
		std::cout << nt._name << " " << nt._follow << std::endl;
	}
	
	assert(completeFollowSets.size() == ctx._ntFollowSetsCount.size());
}

}}
