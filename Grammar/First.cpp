#include "First.h"
#include <iostream>

namespace mws { namespace grammar {

inline std::ostream& operator << (std::ostream& os_, const std::set<TokenID>& set_)
{
	for (auto i : set_)
	{
		os_ << (int)i << " ";
	}

	return os_;
}

void first(grammar::Grammar& grammar_, GSID ntt_, std::vector<bool>& isFirstComputed_)
{
	assert(ntt_ >= 0 && ntt_ < grammar_.size());

	if (isFirstComputed_[ntt_])
	{
		return;
	}

	auto& nt = grammar_[ntt_];

	for (auto& prod : nt._prodList)
	{
		prod._derivesEmpty = true;

		for (const auto& gs : prod._gsList)
		{
			// FIRST of terminal is terminal itself.
			if (gs._isTerminal)
			{
				prod._first.insert(gs._type);
				prod._derivesEmpty = false;
				break;
			}

			// FIRST of non terminal
			first(grammar_, gs._type, isFirstComputed_);

			const auto& ntL = grammar_[gs._type];
			ntL.getFirst(prod._first);

			// If non terminal does not derive empty (e) we are done
			if (!ntL.derivesEmpty())
			{
				prod._derivesEmpty = false;
				break;
			}
		}
	}

	isFirstComputed_[ntt_] = true;
}

void first(grammar::Grammar& grammar_)
{
	std::cout << "First:" << std::endl;

    std::vector<bool> isFirstComputed(grammar_.size(), false);

	for (GSID i = 0; i < grammar_.size(); ++i)
	{
		first(grammar_, i, isFirstComputed);
		std::set<TokenID> firstSet;
		grammar_[i].getFirst(firstSet);
		std::cout << grammar_[i]._name << " " << firstSet << std::endl;
	}
}

}}
