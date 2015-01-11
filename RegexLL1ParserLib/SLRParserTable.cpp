#include "SLRParserTable.h"
#include <FA/DFANode.h>
#include <unordered_map>
#include <functional>
#include <iomanip>

namespace mws { namespace td { namespace LL1 {

using LR0ItemSet = ItemSet<LR0Item, LR0Item::Hash>;
using LR0StateMap = std::unordered_map<typename LR0ItemSet::Ptr, LR0State*, typename LR0ItemSet::Hash, typename LR0ItemSet::Pred>;

static const uint8_t E = -1;

void printTransition(const GrammarSymbol& gs_, const grammar::Grammar& grammar_)
{
    std::cout << "on ";
    print(gs_, grammar_); 

    std::cout << std::endl;
    std::cout << std::endl;
}

void print(const GrammarSymbol& gs_, const grammar::Grammar& grammar_)
{
    if (!gs_._isTerminal)
    {
        std::cout << grammar_[gs_._type]._name;
    }
    else
    {
        std::cout << "'" << static_cast<int>(gs_._type) << "'";
    }
}

void print(const std::set<LR0Item>& itemSet_, const grammar::Grammar& grammar_)
{
    for (const auto& item : itemSet_)
    {
        const auto& nt = grammar_[item._nt];
        std::cout << std::setw(12) << nt._name << " := ";

        const auto& prod = nt._prodList[item._prod];
        for (std::size_t i = 0; i < prod._gsList.size(); ++i)
        {
            if (i == item._dot)
            {
                std::cout << ". ";
            }

            const auto& gs = prod._gsList[i];
            if (!gs._isTerminal)
            {
                std::cout << grammar_[gs._type]._name << " ";
            }
            else
            {
                std::cout << "'" << static_cast<int>(gs._type) << "' ";
            }
        }

        if (prod._gsList.size() == item._dot)
        {
            std::cout << ". ";
        }

        std::cout << std::endl;
    }

    std::cout << std::endl;
}

std::set<GrammarSymbol> getTransitionCharSet(const std::set<LR0Item>& itemSet_, const grammar::Grammar& grammar_)
{
    std::set<GrammarSymbol> gsSet;
    
    for (const auto& item : itemSet_)
    {
        // transitions can be made from the GrammarSymbol immediately to the right of the dot
        const auto& prod = item.prod(grammar_);
        
        if (prod._gsList.size() > item._dot)
        {
            const auto& gs = prod._gsList[item._dot];
            gsSet.insert({ gs._type, gs._isTerminal });
        }    
    }

    return gsSet;
}

void closure(std::set<LR0Item>& itemSet_, const grammar::Grammar& grammar_, const LR0Item& item_)
{
    // All items added in the closure are non-kernel items (dot all the way to the left followed by non-terminal

    // the item represents a production (+ position in production)
    const auto& prod = item_.prod(grammar_);

    // If item to the right of dot is a NonTerminal, then add its productions to the closure
    // This is equivalent to adding E transitions from the item (where can we go without moving the dot, that is, without fetching next char on input)
    // and immediately performing the e_closure
    if (prod._gsList.size() > item_._dot && !prod._gsList[item_._dot]._isTerminal)
    {
        const auto ntType = prod._gsList[item_._dot]._type;
        assert(ntType < grammar_.size());

        for (uint8_t i = 0; i < grammar_[ntType]._prodList.size(); ++i)
        {
            LR0Item item = { ntType, i, 0 };
            if (itemSet_.insert(item).second)
            {
                closure(itemSet_, grammar_, item);
            }
        }
    }
}

std::set<LR0Item> closure(std::set<LR0Item>& itemSet_, const grammar::Grammar& grammar_)
{
    for (const auto& item : itemSet_)
    {
        closure(itemSet_, grammar_, item);
    }

    return itemSet_;
}

std::set<LR0Item> move(const std::set<LR0Item>& srcItemSet_, const GrammarSymbol& gs_, const grammar::Grammar& grammar_)
{
    std::set<LR0Item> dstItemSet;

    // each item can have at most 1 transition.
    // - transition to same item with dot moved 1 to the right
    // - no transition if dot is already all the way to the right
    for (const auto& item : srcItemSet_)
    {
        // c_closure

        // production this item represents
        const auto& prod = item.prod(grammar_);

        // Does the production have GrammarSymbol gs_ to right of the dot?
        if (item._dot < prod._gsList.size() 
            && 
            prod._gsList[item._dot]._isTerminal == gs_._isTerminal
            &&
            prod._gsList[item._dot]._type == gs_._type)
        {
            auto kernelItem(item);
             // dot moves 1 to the right
            kernelItem._dot++;
            dstItemSet.insert(kernelItem);
        } 
    }

    return dstItemSet;
}

void convert(LR0State* dSrc_, const std::set<LR0Item>& srcItemSet_, LR0StateMap& states_, const grammar::Grammar& grammar_)
{
    //print(srcItemSet_, grammar_);

    // obtain all disjoint ranges for which dSrc contains outgoing transitions
    auto gsSet = getTransitionCharSet(srcItemSet_, grammar_);

    for (const auto& gs : gsSet)
    {
        auto dstItemSet = new LR0ItemSet(closure(move(srcItemSet_, gs, grammar_), grammar_));
       
        // this condition holds because we just checked each c has a transition for some n in d
        assert(!dstItemSet->empty());

        auto res = states_.insert(std::make_pair(dstItemSet, nullptr));
        // If we have an equivalent node in the map already, use it instead.
        LR0State* dDst = res.first->second;
        assert(res.second || dDst);

        if (res.second)
        {
            dDst = new LR0State();
            dDst->_itemSet = &dstItemSet->_items;

            res.first->second = dDst;

            //printTransition(gs, grammar_);

            convert(dDst, dstItemSet->_items, states_, grammar_);
        }
       
        assert(dDst);
        dSrc_->_transitionMap.insert(std::make_pair(gs, dDst));
    }
}

std::set<LR0Item> createStartNode(const grammar::NT& ntStart_)
{
    std::set<LR0Item> itemSet;

    // the Start items that reduce to ntStart are kernel items
    for (uint8_t i = 0; i < ntStart_._prodList.size(); ++i)
    {
        LR0Item item = { 0, i, 0 };
        itemSet.insert(item);
    }

    return itemSet;
}

void SLRParserTable::printActionTable()
{
    
    std::cout << "   | ";
    for (int i = 0; i < _actionTable.colCount(); ++i)
    {
        std::cout << "  " << std::setw(2) << i << "  " << "|";
    }
    std::cout << std::endl;

    for (int i = 0; i < _actionTable.rowCount(); ++i)
    {
        std::cout << std::setw(2) << i << " | ";

        for (int j = 0; j < _actionTable.colCount(); ++j)
        {
            auto entry = _actionTable[i][j];
            switch(entry._action)
            {
                case Action::Shift:
                    std::cout << "s" << std::setw(2) << (int)entry._state << "   "; 
                    break;
                case Action::Reduce:
                    std::cout << "r" << std::setw(2) << (int)entry._prod._head << "," << std::setw(2) << (int)entry._prod._body; 
                    break;
                case Action::Accept:
                    std::cout << "a" << "     ";
                    break;
                case Action::Error:
                    std::cout << "e" << "     ";
                    break;

            }

            std::cout << " ";
        }

        std::cout << std::endl;
    }
}

SLRParserTable::SLRParserTable(const grammar::Grammar& grammar_, uint8_t cTerminal_)
{
	build(grammar_, cTerminal_);
}

void SLRParserTable::build(const grammar::Grammar& grammar_, uint8_t cTerminal_)
{
    auto itemSet = new LR0ItemSet(createStartNode(grammar_[0]));
   
    closure(itemSet->_items, grammar_);

    _start = new LR0State();
    _start->_itemSet = &itemSet->_items;
   
    LR0StateMap states;
    states.insert(std::make_pair(itemSet, _start));

    convert(_start, itemSet->_items, states, grammar_);

    // rows = states, columns = terminal + nonterminal <goto>
    Entry entry;
    entry._action = Action::Error;
    entry._state = 0;
    _actionTable.init(states.size(), cTerminal_, entry);
    _gotoTable.init(states.size(), grammar_.size(), E);

    // label states
    uint8_t label = 0;
    for (const auto& kvpair : states)
    {
        auto state = kvpair.second;
        state->_label = label++;
    }

    // iterate rows
    for (const auto& kvpair : states)
    {
        const auto state = kvpair.second;

        // 1: Shift/Goto actions.
        for (const auto& kvpair : state->_transitionMap)
        {
            const auto& gs = kvpair.first;
            const auto nextState = kvpair.second;

            if (gs._isTerminal)
            {
                Entry entry;
                entry._action = Action::Shift;
                entry._state = nextState->_label;
                _actionTable[state->_label][gs._type] = entry;
            }
            else
            {
                // Goto is executed directly after a Reduce action
                _gotoTable[state->_label][gs._type] = nextState->_label;
            }
        }

        // 2: Reduce/Accept actions
        for (const auto& item : *state->_itemSet)
        {
            const auto& nt = grammar_[item._nt];
            const auto& prod = nt._prodList[item._prod];

            if (prod._gsList.size() != item._dot)
            {
                continue;
            }
         
            // Reduce (dot to the right)
            for (const auto t : nt._follow)
            {
                // follow sets of Reduce items not disjoint
                if (_actionTable[state->_label][t]._action == Action::Reduce)
                {
                    throw mws::common::Exception("Reduce/Reduce conflict");
                }

                // Shift takes precedence over Reduce
                if (_actionTable[state->_label][t]._action == Action::Shift)
                {
                    throw mws::common::Exception("Shift/Reduce conflict");
                }

                Entry entry;
                //Token::Eof does not work because subGrammars may have different 'Eof' tokens
                //assert(item._nt != 0 || t == grammar::Token::Eof);

                entry._action = item._nt != 0 ? Action::Reduce : Action::Accept;
                entry._prod._head = item._nt;
                entry._prod._body = item._prod;
                _actionTable[state->_label][t] = entry;
            }
        }
    }

    printActionTable();
    std::cout << std::endl << std::endl;
}

SLRParserTable::Entry SLRParserTable::action(uint8_t state_, uint8_t t_) const
{
    assert(state_ < _actionTable.rowCount());
    assert(t_ < _actionTable.colCount());

    return _actionTable[state_][t_];
}

uint8_t SLRParserTable::goTo(uint8_t state_, uint8_t nt_) const
{
    assert(state_ < _gotoTable.rowCount());
    assert(nt_ < _gotoTable.colCount());

    return _gotoTable[state_][nt_];
}

}}}