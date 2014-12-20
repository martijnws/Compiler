#include "ParserHandler.h"
#include <cassert>

namespace mws { namespace td { namespace LL1 {


namespace {

using NonTerminalFunc = void (ParserHandler::*)();
using TerminalFunc = void (ParserHandler::*)(const grammar::Token& t_);

static NonTerminalFunc _nonTermFuncCol[] =
{
    &ParserHandler::onEof,
	&ParserHandler::onChoice,
	&ParserHandler::onConcat,
    &ParserHandler::onZeroOrOne,
	&ParserHandler::onZeroToMany,
    &ParserHandler::onOneToMany,
	&ParserHandler::onCharClass,
	&ParserHandler::onNegate,
    &ParserHandler::onRngConcat,
	&ParserHandler::onRng
};

static TerminalFunc _termFuncCol[] =
{
    &ParserHandler::onSymbol,
    &ParserHandler::onCharClassSymbol
};

} //NS 

void ParserHandler::onNonTerminal(uint8_t type_)
{
    assert(type_ < sizeof(_nonTermFuncCol)/sizeof(NonTerminalFunc));

    (this->*_nonTermFuncCol[type_])();
}

void ParserHandler::onTerminal(uint8_t type_, const grammar::Token& t_)
{
    assert(type_ < sizeof(_termFuncCol)/sizeof(TerminalFunc));

    (this->*_termFuncCol[type_])(t_);
}

}}}
