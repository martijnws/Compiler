#pragma once

#include <Grammar/Grammar.h>
#include <CommonLib/Matrix.h>

namespace mws { namespace td { namespace LL1 {

class ParserTable
{
public:
	using GSID = grammar::GSID;

    ParserTable(const grammar::Grammar& grammar_, TokenID cTerminal_);

    const grammar::Production& expand(const grammar::Grammar& grammar_, GSID nt_,  TokenID t_) const;

private:
    void build(const grammar::Grammar& grammar_, TokenID cTerminal_);

private:
    common::Matrix<GSID> _table;
};

}}}