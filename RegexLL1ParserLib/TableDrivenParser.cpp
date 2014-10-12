#include "TableDrivenParser.h"
#include <CommonLib/Exception.h>
#include <algorithm>
#include <map>

namespace mws { namespace td { namespace LL1 {

GrammarSymbol gs(bool isTerminal_, uint32_t type_)
{
	static auto no_op = [](const Token& t_, ParserHandler& h_, TokenStore& store_){};
	return { isTerminal_, type_, no_op };
}

inline GrammarSymbol t(uint32_t type_) { return gs(true, type_); }
inline GrammarSymbol n(uint32_t type_) { return gs(false, type_); }

GrammarSymbol gs(bool isTerminal_, uint32_t type_, onEvt callback_)
{
	return { isTerminal_, type_, [callback_](const Token& t_, ParserHandler& h_, TokenStore& store_)
	{
		(h_.*callback_)();
	}};
}

inline GrammarSymbol t(uint32_t type_, onEvt callback_) { return gs(true, type_, callback_); }
inline GrammarSymbol n(uint32_t type_, onEvt callback_) { return gs(false, type_, callback_); }

GrammarSymbol gs(bool isTerminal_, uint32_t type_, onTokenEvt callback_)
{
	return { isTerminal_, type_, [callback_](const Token& t_, ParserHandler& h_, TokenStore& store_)
	{
		(h_.*callback_)(t_);
	}};
}

inline GrammarSymbol t(uint32_t type_, onTokenEvt callback_) { return gs(true, type_, callback_); }
inline GrammarSymbol n(uint32_t type_, onTokenEvt callback_) { return gs(false, type_, callback_); }

GrammarSymbol t_S(uint32_t type_, const uint8_t index_)
{
	return { true, type_, [index_](const Token& t_, ParserHandler& h_, TokenStore& store_)
	{
		store_.save(index_, t_);
	}};
}

GrammarSymbol n_G(uint32_t type_, onTokenEvt callback_, const uint8_t index_)
{
	return { false, type_, [callback_, index_](const Token& t_, ParserHandler& h_, TokenStore& store_)
	{
		(h_.*callback_)(store_.get(index_));
	}};
}

GrammarSymbol n_P(uint32_t type_, onTokenEvt callback_, const uint8_t index_)
{
	return { false, type_, [callback_, index_](const Token& t_, ParserHandler& h_, TokenStore& store_)
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

class NT
{
public:
	NT(const std::initializer_list<Production>& prodList_)
		: _prodList(prodList_.begin(), prodList_.end())
	{
		
	}

	bool derivesEmpty() const
	{
		return std::any_of(_prodList.begin(), _prodList.end(), [](const Production& prod_){ return prod_._derivesEmpty; });
	}

	void getFirst(std::set<uint8_t>& firstSet_) const
	{
		for (const Production& prod : _prodList)
		{
			firstSet_.insert(prod._first.begin(), prod._first.end());
		}
	}

	std::vector<Production> _prodList;
	std::set<uint8_t>       _follow;
};


static const uint8_t E = -1;
static const uint8_t e = -2;

static const Production empty({ t(e) });

NT grammar[] = { 
//choice
{ { n(N::Concat), n(N::ChoiceT) },
  { empty } },
//choiceT
{ { t(T::Choice), n(N::Concat, &H::onChoice), n(N::ChoiceT) },
  { empty } },
//concat
{ { n(N::Term), n(N::ConcatT) } },
//concatT
{ { n(N::Term, &H::onConcat), n(N::ConcatT) },
  { empty } },
//term
{ { n(N::Factor), n(N::ZeroToManyO) } },
//zeroToManyO
{ { t(T::ZeroToMany, &H::onZeroToMany) },
  { empty } },
//factor
{ { t(T::Symbol, &H::onSymbol) },
//{ n(N::CharClass) },
  { t(T::SubExprB), n(N::Choice), t(T::SubExprE) } },
};

static const std::size_t cN = N::CharClass + 1;
static const std::size_t cT = T::Eof + 1;
static uint8_t parserTable[cN][cT] = { E };

void first(uint8_t ntt_)
{
	assert(ntt_ >= 0 && ntt_ <= N::CharClass);

	NT& nt = grammar[ntt_];
	if (!nt.derivesEmpty())
	{
		return;
	}

	for (Production& prod : nt._prodList)
	{
		for (const GrammarSymbol& gs : prod._gsList)
		{
			if (gs._type == e)
			{
				continue;
			}

			// FIRST of terminal is terminal itself.
			if (gs._isTerminal)
			{
				prod._first.insert(gs._type);
				prod._derivesEmpty = false;
				break;
			}

			// FIRST of non terminal
			first(static_cast<NonTerminal>(gs._type));

			const NT& ntL = grammar[gs._type];
			ntL.getFirst(prod._first);

			// If non terminal does not derive empty (e) we are done
			if (!ntL.derivesEmpty())
			{
				prod._derivesEmpty = false;
				break;
			}
		}
	}
}

static std::vector<uint8_t> followSetRequestStack[cN];

void resolveFollowSets(const std::set<uint8_t>& followSet_, std::vector<uint8_t>& todoStack_)
{
	while (!todoStack_.empty())
	{
		uint8_t ntt = todoStack_.back();
		assert(ntt >= 0 && ntt < N::CharClass);

		NT& nt = grammar[ntt];
		nt._follow.insert(followSet_.begin(), followSet_.end());
		todoStack_.pop_back();

		resolveFollowSets(nt._follow, followSetRequestStack[ntt]);
	}
}

void follow(uint8_t nttHead_, Production& prod_)
{
	assert(nttHead_ >= 0 && nttHead_ <= N::CharClass);
	
	NT& ntHead = grammar[nttHead_];

	for (uint8_t i = 0; i < prod_._gsList.size(); ++i)
	{
		const GrammarSymbol& gs = prod_._gsList[i];

		//not interested in follow of terminals
		if (gs._isTerminal)
			continue;

		uint8_t ntt = prod_._gsList[i]._type;
		assert(ntt >= 0 && ntt <= N::CharClass);
		NT& nt = grammar[ntt];

		//keep adding to nt.follow until a non empty deriving GrammarSymbol is encountered
		bool done = false;
		for (uint8_t j = i + 1; j < prod_._gsList.size(); ++j)
		{
			const GrammarSymbol& gsNext = prod_._gsList[j];

			if (gsNext._isTerminal)
			{
				nt._follow.insert(gsNext._type);
				done = true;
				break;
			}

			NT& ntNext = grammar[prod_._gsList[j]._type];
			ntNext.getFirst(nt._follow);

			if (!ntNext.derivesEmpty())
			{
				done = true;
				break;
			}
		}

		if (!done)
		{

			if (!ntHead._follow.empty())
			{
				nt._follow.insert(ntHead._follow.begin(), ntHead._follow.end());
				// resolve pending requests
				resolveFollowSets(nt._follow, followSetRequestStack[ntt]);
			}
			else
			{
				followSetRequestStack[nttHead_].push_back(ntt);
			}
		}
	}
}

void follow()
{
	NT& ntStart = grammar[0];
	ntStart._follow.insert(T::Eof);

	for (uint8_t i = 0; i < sizeof(grammar)/sizeof(NT); ++i)
	{
		for (Production& prod : grammar[i]._prodList)
		{
			follow(i, prod);
		}
	}
}

void first()
{
	for (uint8_t i = 0; i < sizeof(grammar)/sizeof(NT); ++i)
	{
		first(i);
	}
}

bool init()
{
	std::fill(*parserTable, *parserTable + cN * cT, E);

	first();
	follow();
	return true;
}

//void init(NonTerminal nt_, const std::initializer_list<Token::Type>& firstSet_, uint8_t prod_, uint8_t otherwise_)
//{
//	uint8_t* row_ = parserTable[nt_];
//	std::fill(row_, row_ + cT, otherwise_);
//
//	for (Token::Type t : firstSet_)
//	{
//		row_[t] = prod_;
//	}
//}
//
//bool init()
//{
//	std::fill(*parserTable, *parserTable + cN * cT, E);
//
//	init(N::Choice,      { T::Symbol, T::CharClassB, T::SubExprB }, 0, 1);
//	init(N::ChoiceT,     { T::Choice },                             0, 1);
//
//	init(N::Concat,      { T::Symbol, T::CharClassB, T::SubExprB }, 0, E);
//	init(N::ConcatT,     { T::Symbol, T::CharClassB, T::SubExprB }, 0, 1);
//
//	init(N::Term,        { T::Symbol, T::CharClassB, T::SubExprB }, 0, E);
//
//	init(N::ZeroToManyO, { T::ZeroToMany },                         0, 1);
//
//	init(N::Factor,      { T::Symbol },                             0, E);
//	init(N::Factor,      { T::CharClassB },                         1, E);
//	init(N::Factor,      { T::SubExprB },                           2, E);
//
//	return true;
//}

const Production& expand(NonTerminal nt_, Token::Type t_)
{
	uint8_t index = parserTable[nt_][t_];

	if (index == E)
	{
		throw common::Exception("Parser error: no production found");
	}

	return grammar[nt_]._prodList[index];
}

}}}