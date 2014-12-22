#pragma once

#include "LR0State.h"
#include <Grammar/Grammar.h>

namespace mws { namespace td { namespace LL1 {

class SLRParserTable
{
public:
    SLRParserTable(const grammar::Grammar& grammar_, uint8_t cTerminal_);

    const LR0State* start() const
    {
        return _start;
    }

private:
    void build(const grammar::Grammar& grammar_);

private:
    LR0State* _start;
};

void print(const GrammarSymbol& gs_, const grammar::Grammar& grammar_);

inline void print(const grammar::GrammarSymbol& gs_, const grammar::Grammar& grammar_)
{
    GrammarSymbol gs = { gs_._type, gs_._isTerminal };
    print(gs, grammar_);
}

}}}