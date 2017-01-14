#pragma once

#include "LR0State.h"
#include <Grammar/Grammar.h>
#include <CommonLib/Matrix.h>

namespace mws { namespace td { namespace SLR {

class ParserTable
{
public:
    enum Action { Shift, Reduce, Accept, Error };

    struct Entry
    {
        Action _action;
        union
        {
            uint8_t _state;
            struct
            {
                uint8_t _head;
                uint8_t _body;
            } _prod;
        };
    };

    ParserTable(const grammar::Grammar& grammar_, uint8_t cTerminal_);

    Entry   action(uint8_t state_, uint8_t t_) const;
    uint8_t goTo(uint8_t state_, uint8_t nt_) const;

    const uint8_t startLabel() const
    {
        return _startLabel;
    }

private:
    void build(const grammar::Grammar& grammar_, uint8_t cTerminal_);

    void printActionTable();

private:
    uint8_t                 _startLabel = 0;
    common::Matrix<Entry>   _actionTable;
    common::Matrix<uint8_t> _gotoTable;
};

void print(const GrammarSymbol& gs_, const grammar::Grammar& grammar_);

inline void print(const grammar::GrammarSymbol& gs_, const grammar::Grammar& grammar_)
{
    GrammarSymbol gs = { gs_._type, gs_._isTerminal };
    print(gs, grammar_);
}

}}}