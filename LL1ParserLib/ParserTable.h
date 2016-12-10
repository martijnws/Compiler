#pragma once

#include <Grammar/Grammar.h>
#include <CommonLib/Matrix.h>

namespace mws { namespace td { namespace LL1 {

class ParserTable
{
public:
    ParserTable(const grammar::Grammar& grammar_, uint8_t cTerminal_);

    const grammar::Production& expand(const grammar::Grammar& grammar_, uint8_t nt_,  grammar::Token::Type t_) const;

private:
    void build(const grammar::Grammar& grammar_, uint8_t cTerminal_);

private:
    common::Matrix<uint8_t> _table;
};

}}}