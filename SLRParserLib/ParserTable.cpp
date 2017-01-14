#include "ParserTable.h"
#include <FA/DFANode.h>
#include <unordered_map>
#include <functional>
#include <iomanip>
#include <fstream>

namespace mws { namespace td { namespace SLR {

using LR0ItemSet = ItemSet<LR0Item, LR0Item::Hash>;
using LR0StateMap = std::unordered_map<typename LR0ItemSet::Ptr, LR0State*, typename LR0ItemSet::Hash, typename LR0ItemSet::Pred>;

void deleteStateMapKeyVal(LR0StateMap& stateMap_)
{
	for (auto pair : stateMap_)
	{
		delete pair.first;
		delete pair.second;
	}
}

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
	auto& os_ = std::cout;

    for (const auto& item : itemSet_)
    {
        const auto& nt = grammar_[item._nt];
        os_ << std::setw(12) << nt._name << " := ";

        const auto& prod = nt._prodList[item._prod];
        for (std::size_t i = 0; i < prod._gsList.size(); ++i)
        {
            if (i == item._dot)
            {
                os_ << ". ";
            }

            const auto& gs = prod._gsList[i];
            if (!gs._isTerminal)
            {
                os_ << grammar_[gs._type]._name << " ";
            }
            else
            {
                os_ << "'" << static_cast<int>(gs._type) << "' ";
            }
        }

        if (prod._gsList.size() == item._dot)
        {
            os_ << ". ";
        }

        os_ << std::endl;
    }

    os_ << std::endl;
}

void closure(std::set<LR0Item>& itemSet_, const grammar::Grammar& grammar_, const LR0Item& item_)
{
    // All items added in the closure are non-kernel items (dot all the way to the left followed by non-terminal

    // the item represents a production (+ position in production)
    const auto& prod = item_.prod(grammar_);

	if (item_._dot >= prod._gsList.size())
	{
		assert(item_._dot == prod._gsList.size());
		return;
	}

    // If item to the right of dot is a NonTerminal, then add its productions to the closure
    // This is equivalent to adding E transitions from the item (where can we go without moving the dot, that is, without fetching next char on input)
    // and immediately performing the e_closure

    if (!prod._gsList[item_._dot]._isTerminal)
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

std::set<LR0Item> closure(const std::set<LR0Item>& itemSet_, const grammar::Grammar& grammar_)
{
	//Top level invocation of closure handles kernel items. These are different in that they
	//may have the dot at any position, not just at the beginning.
	auto itemSet = itemSet_;
	//iterate over copy since original will change
    for (const auto& item : itemSet_)
    {
        closure(itemSet, grammar_, item);
    }

    return itemSet;
}

std::map<GrammarSymbol, std::set<LR0Item>> getTransitionMap(const std::set<LR0Item>& srcItemSet_, const grammar::Grammar& grammar_)
{
    std::set<LR0Item> dstItemSet;

	std::map<GrammarSymbol, std::set<LR0Item>> transitionMap;

    for (const auto& item : srcItemSet_)
    {
        // c_closure

        // production this item represents
        const auto& prod = item.prod(grammar_);

		if (item._dot >= prod._gsList.size())
		{
			continue;
		}

		const auto& gs = GrammarSymbol { prod._gsList[item._dot]._type, prod._gsList[item._dot]._isTerminal };
		auto& dstItemSet = transitionMap.insert(std::make_pair(gs, std::set<LR0Item>())).first->second;

		auto kernelItem = item;
		 // dot moves 1 to the right
		kernelItem._dot++;
		dstItemSet.insert(kernelItem);
    }

    return transitionMap;
}

void convert(LR0State* dSrc_, const std::set<LR0Item>& srcItemSet_, LR0StateMap& states_, const grammar::Grammar& grammar_)
{
	dSrc_->_itemSetClosure = closure(srcItemSet_, grammar_);
	auto transitionMap = getTransitionMap(dSrc_->_itemSetClosure, grammar_);

    print(dSrc_->_itemSetClosure, grammar_);

	for (auto& pair : transitionMap)
    {
        //auto dstItemSet = new LR0ItemSet(closure(pair.second, grammar_));
        std::unique_ptr<LR0ItemSet> dstItemSet(new LR0ItemSet(std::move(pair.second)));
       
        // this condition holds because we just checked each c has a transition for some n in d
        assert(!dstItemSet->empty());

        auto res = states_.insert(std::make_pair(dstItemSet.get(), nullptr));
        // If we have an equivalent node in the map already, use it instead.
        LR0State* dDst = res.first->second;
        assert(res.second || dDst);

		const auto gs = pair.first;

        if (res.second)
        {
            dDst = new LR0State();
            res.first->second = dDst;

            printTransition(gs, grammar_);

            convert(dDst, dstItemSet->_items, states_, grammar_);
			dstItemSet.release();
        }
       
        assert(dDst);
        dSrc_->_transitionMap.insert(std::make_pair(gs, dDst));
    }
}

std::set<LR0Item> createStartNode(const grammar::NT& ntStart_)
{
    std::set<LR0Item> itemSet;

	//add kernel items
    for (uint8_t i = 0; i < ntStart_._prodList.size(); ++i)
    {
        LR0Item item = { 0, i, 0 };
        itemSet.insert(item);
    }

    return itemSet;
}

void ParserTable::printActionTable()
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

ParserTable::ParserTable(const grammar::Grammar& grammar_, uint8_t cTerminal_)
{
	build(grammar_, cTerminal_);
}

void ParserTable::build(const grammar::Grammar& grammar_, uint8_t cTerminal_)
{
    auto itemSet = new LR0ItemSet(createStartNode(grammar_[0]));
   
    auto* start = new LR0State();

    LR0StateMap states;
    states.insert(std::make_pair(itemSet, start));

    convert(start, itemSet->_items, states, grammar_);

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
	_startLabel = start->_label;

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

		//Note: the closure is required because of added empty productions which meet the 'dot is far right' condition.
		const auto& itemSet = state->_itemSetClosure;
        //for (const auto& item : *state->_itemSet)
        for (const auto& item : itemSet)
        {
            const auto& nt = grammar_[item._nt];
            const auto& prod = item.prod(grammar_);

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
                    throw mws::common::Exception(_C("Reduce/Reduce conflict"));
                }

                // we can shift and reduce.
                if (_actionTable[state->_label][t]._action == Action::Shift)
                {
                    throw mws::common::Exception(_C("Shift/Reduce conflict"));
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
	deleteStateMapKeyVal(states);

    printActionTable();
    std::cout << std::endl << std::endl;
}

ParserTable::Entry ParserTable::action(uint8_t state_, uint8_t t_) const
{
    assert(state_ < _actionTable.rowCount());
    assert(t_ < _actionTable.colCount());

    return _actionTable[state_][t_];
}

uint8_t ParserTable::goTo(uint8_t state_, uint8_t nt_) const
{
    assert(state_ < _gotoTable.rowCount());
    assert(nt_ < _gotoTable.colCount());

    return _gotoTable[state_][nt_];
}

}}}