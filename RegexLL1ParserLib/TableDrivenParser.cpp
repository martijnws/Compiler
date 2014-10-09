#include "TableDrivenParser.h"
#include <CommonLib/Exception.h>
#include <algorithm>

namespace mws { namespace td { namespace LL1 {


GrammarSymbol gs(uint32_t type_)
{
	static auto no_op = [](const Token& t_, ParserHandler& h_, TokenStore& store_){};
	return { type_, no_op };
}

GrammarSymbol gs(uint32_t type_, onEvt callback_)
{
	return { type_, [callback_](const Token& t_, ParserHandler& h_, TokenStore& store_)
	{
		(h_.*callback_)();
	}};
}

GrammarSymbol gs(uint32_t type_, onTokenEvt callback_)
{
	return { type_, [callback_](const Token& t_, ParserHandler& h_, TokenStore& store_)
	{
		(h_.*callback_)(t_);
	}};
}

GrammarSymbol gs_S(uint32_t type_, const uint8_t index_)
{
	return { type_, [index_](const Token& t_, ParserHandler& h_, TokenStore& store_)
	{
		store_.save(index_, t_);
	}};
}

GrammarSymbol gs_G(uint32_t type_, onTokenEvt callback_, const uint8_t index_)
{
	return { type_, [callback_, index_](const Token& t_, ParserHandler& h_, TokenStore& store_)
	{
		(h_.*callback_)(store_.get(index_));
	}};
}

GrammarSymbol gs_P(uint32_t type_, onTokenEvt callback_, const uint8_t index_)
{
	return { type_, [callback_, index_](const Token& t_, ParserHandler& h_, TokenStore& store_)
	{
		(h_.*callback_)(store_.pop(index_));
	}};
}


// Regex grammar:
	//
	// choice     = concat choiceT              First = { symbol, [, ( }
	//             | e
	// choiceT =  '|' concat {A;}  choiceT      First = { |, e }
	//             | e
	// concat        = term concatT             First = { symbol, [, ( }
	// concatT    = term {A;} concatT           First = { symbol, [, ( }
	//             | e
	// term        = factor zeroToManyO         First = { symbol, [, ( }
	// zeroToManyO = * {A}                      First = { *, e }
	//             | e                             
	// factor      = symbol {A}                 First = { symbol }
	//             | charClass					First = { [ }
	//             | ( choice )                 First = { ( }


// Rows


using H = ParserHandler;
using T = Token::Type;
using N = NonTerminal;

static const Production prodList[] =
{
	//choice
	{ gs(N::Concat), gs(N::Choice) }, // 0
	{ gs(N::Empty) }, // 1
	//choiceT
	{ gs(T::Choice), gs(N::Concat, &H::onChoice), gs(N::ChoiceT) }, // 2
	{ gs(N::Empty) }, // 3
	//concat
	{ gs(N::Term), gs(N::Concat) }, // 4
	//concatT
	{ gs(N::Term, &H::onConcat), gs(N::ConcatT) }, // 5
	{ gs(N::Empty) }, // 6
	//term
	{ gs(N::Factor), gs(N::ZeroToManyO) }, // 7
	//zeroToManyO
	{ gs(T::ZeroToMany, &H::onZeroToMany) }, // 8
	{ gs(N::Empty) }, // 9
	//factor
	{ gs(T::Symbol, &H::onSymbol) }, // 10
	{ gs(N::CharClass) }, // 11
	{ gs(T::SubExprB), gs(N::Choice), gs(T::SubExprE) } // 12
};

static const uint8_t E = -1;

static const std::size_t cN = N::CharClass - N::Choice + 1;
static const std::size_t cT = T::Eof - T::None + 1;
static uint8_t parserTable[cN][cT] = { E };

void init(NonTerminal nt_, const std::initializer_list<Token::Type>& firstSet_, uint8_t prod_, uint8_t otherwise_)
{
	uint8_t* row_ = parserTable[nt_ - N::Choice];
	std::fill(row_, row_ + cT, otherwise_);

	for (Token::Type t : firstSet_)
	{
		row_[t] = prod_;
	}
}

bool init()
{
	std::fill(*parserTable, *parserTable + cN * cT, E);

	init(N::Choice,      { T::Symbol, T::CharClassB, T::SubExprB }, 0 , 1);
	init(N::ChoiceT,     { T::Choice },                             2 , 3);

	init(N::Concat,      { T::Symbol, T::CharClassB, T::SubExprB }, 4 , E);
	init(N::ConcatT,     { T::Symbol, T::CharClassB, T::SubExprB }, 5 , 6);

	init(N::Term,        { T::Symbol, T::CharClassB, T::SubExprB }, 7 , E);

	init(N::ZeroToManyO, { T::ZeroToMany },                         8 , 9);

	init(N::Factor,      { T::Symbol },                             10, E);
	init(N::Factor,      { T::CharClassB },                         11, E);
	init(N::Factor,      { T::SubExprB },                           12, E);

	return true;
}

const Production& expand(NonTerminal nt_, Token::Type t_)
{
	uint8_t index = parserTable[nt_][t_];

	if (index == E)
	{
		throw common::Exception("Parser error: no production found");
	}

	return prodList[index];
}

}}}