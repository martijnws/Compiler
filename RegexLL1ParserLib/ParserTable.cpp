#include "ParserTable.h"

#include <CommonLib/Exception.h>
#include <algorithm>
#include <unordered_map>

namespace mws { namespace td { namespace LL1 {

static const uint8_t E = -1;

ParserTable::ParserTable(const grammar::Grammar& grammar_, uint8_t cTerminal_)
:
    _table(grammar_.size(), cTerminal_, E)
{
	build(grammar_, cTerminal_);
}

void ParserTable::build(const grammar::Grammar& grammar_, uint8_t cTerminal_)
{
	for (uint8_t ntt = 0; ntt < grammar_.size(); ++ntt)
	{
		const auto& nt = grammar_[ntt];

		for (uint8_t i = 0; i < nt._prodList.size(); ++i)
		{
			const auto& prod = nt._prodList[i];

			// populate first
			for (uint8_t t : prod._first)
			{
				if (_table[ntt][t] != E) 
					throw common::Exception("ParserTable confict: first set not disjoint");

				_table[ntt][t] = i;
			}

			if (!prod._derivesEmpty)
			{
				continue;
			}

			// populate follow
			for (uint8_t t : nt._follow)
			{
				if (_table[ntt][t] != E && _table[ntt][t] != i) 
					throw common::Exception("ParserTable confict: follow set not disjoint");

				_table[ntt][t] = i;
			}
		}
	}
}

const grammar::Production& ParserTable::expand(const grammar::Grammar& grammar_, uint8_t nt_, grammar::Token::Type t_) const
{
	auto index = _table[nt_][t_];

	if (index == E)
	{
		throw common::Exception("Parser error: no production found");
	}

	return grammar_[nt_]._prodList[index];
}

}}}