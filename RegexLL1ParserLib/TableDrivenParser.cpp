#include "TableDrivenParser.h"
#include <CommonLib/Exception.h>
#include <algorithm>
#include <map>
#include <iostream>

namespace mws { namespace td { namespace LL1 {

template< template<typename> class LexerT>
GrammarSymbol sgss(uint32_t type_, onEvt callback_)
{
    static auto parser_func = [](const GrammarSymbol& startSymbol_, typename common::Buffer<256>& buf_, Token& t_, ParserHandler& h_)
    {
        ParserDriver<LexerT> parser(buf_, t_, h_);

        parser.parse(startSymbol_);
    };

	return { false, true, true, type_, [callback_](const Token& t_, ParserHandler& h_, TokenStore& store_)
	{
		(h_.*callback_)();
	}, parser_func};
}

GrammarSymbol gs(bool isTerminal_, bool fetchNext_, uint32_t type_)
{
	static auto no_op = [](const Token& t_, ParserHandler& h_, TokenStore& store_){};
	return { isTerminal_, false, fetchNext_, type_, no_op };
}

inline GrammarSymbol t(uint32_t type_) { return gs(true, true, type_); }
inline GrammarSymbol t(uint32_t type_, bool fetchNext_) { return gs(true, fetchNext_, type_); }
inline GrammarSymbol n(uint32_t type_) { return gs(false, true, type_); }

GrammarSymbol gs(bool isTerminal_, uint32_t type_, onEvt callback_)
{
	return { isTerminal_, false, true, type_, [callback_](const Token& t_, ParserHandler& h_, TokenStore& store_)
	{
		(h_.*callback_)();
	}};
}

inline GrammarSymbol t(uint32_t type_, onEvt callback_) { return gs(true, type_, callback_); }
inline GrammarSymbol n(uint32_t type_, onEvt callback_) { return gs(false, type_, callback_); }

GrammarSymbol gs(bool isTerminal_, uint32_t type_, onTokenEvt callback_)
{
	return { isTerminal_, false, true, type_, [callback_](const Token& t_, ParserHandler& h_, TokenStore& store_)
	{
		(h_.*callback_)(t_);
	}};
}

inline GrammarSymbol t(uint32_t type_, onTokenEvt callback_) { return gs(true, type_, callback_); }
inline GrammarSymbol n(uint32_t type_, onTokenEvt callback_) { return gs(false, type_, callback_); }

GrammarSymbol t_S(uint32_t type_, const uint8_t index_)
{
	return { true, false, true, type_, [index_](const Token& t_, ParserHandler& h_, TokenStore& store_)
	{
		store_.save(index_, t_);
	}};
}

GrammarSymbol n_G(uint32_t type_, onTokenEvt callback_, const uint8_t index_)
{
	return { false, false, true, type_, [callback_, index_](const Token& t_, ParserHandler& h_, TokenStore& store_)
	{
		(h_.*callback_)(store_.get(index_));
	}};
}

GrammarSymbol n_P(uint32_t type_, onTokenEvt callback_, const uint8_t index_)
{
	return { false, false, true, type_, [callback_, index_](const Token& t_, ParserHandler& h_, TokenStore& store_)
	{
		(h_.*callback_)(store_.pop(index_));
	}};
}


// Regex grammar:
	//
	// choice      = concat choiceT             First = { symbol, [, ( }
	//             | e
	// choiceT     =  '|' concat {A;}  choiceT  First = { |, e }
	//             | e
	// concat      = term concatT               First = { symbol, [, ( }
	// concatT     = term {A;} concatT          First = { symbol, [, ( }
	//             | e
	// term        = factor zeroToManyO         First = { symbol, [, ( }
	// zeroToManyO = * {A}                      First = { *, e }
	//             | e                             
	// factor      = symbol {A}                 First = { symbol }
	//             | [ charClass ]			    First = { [ }
	//             | ( choice )                 First = { ( }

// charClass grammar:
	//
	// charClass   = ^ RngConcat {A;}           First = { symbol, ^ }
	//             | RngConcat                  First = { symbol }
	// RngConcat   = Rng RngConcatT             First = { symbol }
	// RngConcatT  = Rng {A;} RngConcatT        First = { symbol }
	//             | e
	// Rng         = option RngT                First = { symbol }
	// RngT        = - option {A;}              First = { - }
	//             | e
	// option      = symbol {A;}

// Rows


using H = ParserHandler;
using T = Token::Type;
using N = NonTerminal;

class NT
{
public:
	NT(const std::string& name_, const std::initializer_list<Production>& prodList_)
		: _prodList(prodList_.begin(), prodList_.end()), _name(name_)
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
	std::string             _name;
};


static const uint8_t E = -1;
static const uint8_t e = -2;

static const Production empty({});

NT grammar[] = { 

NT("Start",       { { n(N::Choice) },
                    { empty } }),

NT("Choice",      { { n(N::Concat), n(N::ChoiceT) } }),

NT("ChoiceT",     { { t(T::Choice), n(N::Concat, &H::onChoice), n(N::ChoiceT) },
                    { empty } }),

NT("Concat",      { { n(N::Term), n(N::ConcatT) } }),

NT("ConcatT",     { { n(N::Term, &H::onConcat), n(N::ConcatT) },
                    { empty } }),

NT("Term",        { { n(N::Factor), n(N::ZeroToManyO) } }),

NT("ZeroToManyO", { { t(T::ZeroToMany, &H::onZeroToMany) },
                    { empty } }),

NT("Factor",      { { t(T::Symbol, &H::onSymbol) },
                    { t(T::CharClassB, false), sgss<CharClassLexer>(N::CharClass, &H::onCharClass), t(T::CharClassE) },
                    { t(T::SubExprB), n(N::Choice), t(T::SubExprE) } }),

NT("CharClass",   { { t(T::CharClassNeg), n(N::RngConcat, &H::onNegate) },
                    { n(N::RngConcat) } }),

NT("RngConcat",     { { n(N::Rng), n(N::RngConcatT) } }),
                    
NT("RngConcatT",    { { n(N::Rng, &H::onRngConcat), n(N::RngConcatT) },
                    { empty } }),

NT("Rng",         { { n(N::Option), n(N::RngT) } }),

NT("RngT",        { { t(T::RngSep), n(N::Option, &H::onRng) },
                    { empty } }),

NT("Option",      { { t(T::Symbol, &H::onCharClassSymbol) } }),

};

std::ostream& operator << (std::ostream& os_, const std::set<uint8_t>& set_)
{
	for (auto i : set_)
	{
		os_ << (int)i << " ";
	}

	return os_;
}

static const std::size_t cN = N::Option + 1;
static const std::size_t cT = T::Eof + 1;
static uint8_t parserTable[cN][cT] = { E };

static bool isFirstComputed[cN] = { false };

void first(uint8_t ntt_)
{
	assert(ntt_ >= 0 && ntt_ <= N::Option);

	if (isFirstComputed[ntt_])
	{
		return;
	}

	NT& nt = grammar[ntt_];

	for (Production& prod : nt._prodList)
	{
		prod._derivesEmpty = true;

		for (const GrammarSymbol& gs : prod._gsList)
		{
			// FIRST of terminal is terminal itself.
			if (gs._isTerminal)
			{
				prod._first.insert(gs._type);
				prod._derivesEmpty = false;
				break;
			}

			// FIRST of non terminal
			first(gs._type);

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

	isFirstComputed[ntt_] = true;
}

void first()
{
	std::cout << "First:" << std::endl;

	for (uint8_t i = 0; i < sizeof(grammar)/sizeof(NT); ++i)
	{
		first(i);
		std::set<uint8_t> firstSet;
		grammar[i].getFirst(firstSet);
		std::cout << grammar[i]._name << " " << firstSet << std::endl;
	}
}

static uint8_t    ntFollowSetsCount[cN] = { 0 };
static std::vector<uint8_t> followSetToNtsMap[cN];

void follow(uint8_t nttHead_, Production& prod_)
{
	assert(nttHead_ >= 0 && nttHead_ <= N::Option);
	
	NT& ntHead = grammar[nttHead_];

	// start with true because last grammar symbol requires follow(ntHead)
	bool requiresFollow = true;
	std::set<uint8_t> followSet;

	for (int64_t i = prod_._gsList.size() - 1; i >= 0; --i)
	{
		const GrammarSymbol& gs = prod_._gsList[i];
		if (gs._isTerminal)
		{
			requiresFollow = false;
			followSet.clear();
			followSet.insert(gs._type);
			continue;
		}

		// update nt.follow with followSet prepared in previous iteration
		NT& nt = grammar[gs._type];
		nt._follow.insert(followSet.begin(), followSet.end());

		// make a note that follow(ntHead) must be added in a later pass
		if (requiresFollow && gs._type != nttHead_ /*prevent H -> aH recursion*/)
		{
			followSetToNtsMap[nttHead_].push_back(gs._type);
			++ntFollowSetsCount[gs._type];
		}

		// prepare followSet for symbol to the left (next iteration)

		// 1) as soon as we encounter a non-empty deriving symbol, follow(ntHead) will no longer be required
		requiresFollow &= nt.derivesEmpty();
		if (!requiresFollow)
		{
			followSet.clear();
		}

		// 2) first of current = follow of symbol on the left
		nt.getFirst(followSet);
	}
}


void follow()
{
	std::cout << "Follow:" << std::endl;

	NT& ntStart = grammar[0];
	ntStart._follow.insert(T::Eof);

	// first pass: add first sets
	for (uint8_t i = 0; i < sizeof(grammar)/sizeof(NT); ++i)
	{
		for (Production& prod : grammar[i]._prodList)
		{
			follow(i, prod);
		}
	}

	// second pass: add followsets
	std::vector<uint8_t> completeFollowSets;
	
	for (uint8_t i = 0; i < cN; ++i)
	{
		if (ntFollowSetsCount[i] == 0)
		{
			completeFollowSets.push_back(i);
		}
	}

	for (size_t i = 0; i < completeFollowSets.size(); ++i)
	{
		uint8_t nttComplete = completeFollowSets[i];
		const NT& ntComplete = grammar[nttComplete];

		std::vector<uint8_t>& incompleteFollowSets = followSetToNtsMap[nttComplete];
		for (uint8_t nttIncomplete : incompleteFollowSets)
		{
			// add follow(nttComplete) to follow(nttIncomplete)
			NT& ntIncomplete = grammar[nttIncomplete];
			ntIncomplete._follow.insert(ntComplete._follow.begin(), ntComplete._follow.end());

			uint8_t& count = ntFollowSetsCount[nttIncomplete];
			assert(count > 0);

			if (--count == 0)
			{
				completeFollowSets.push_back(nttIncomplete);
			}
		}
	}

	for (uint8_t i = 0; i < sizeof(grammar)/sizeof(NT); ++i)
	{
		std::cout << grammar[i]._name << " " << grammar[i]._follow << std::endl;
	}
	
	assert(completeFollowSets.size() == cN);
}

void buildParserTable()
{
	for (uint8_t ntt = 0; ntt < cN; ++ntt)
	{
		const NT& nt = grammar[ntt];

		for (uint8_t i = 0; i < nt._prodList.size(); ++i)
		{
			const Production& prod = nt._prodList[i];

			// populate first
			for (uint8_t t : prod._first)
			{
				if (parserTable[ntt][t] != E) 
					throw common::Exception("ParserTable confict: first set not disjoint");

				parserTable[ntt][t] = i;
			}

			if (!prod._derivesEmpty)
			{
				continue;
			}

			// populate follow
			for (uint8_t t : nt._follow)
			{
				if (parserTable[ntt][t] != E && parserTable[ntt][t] != i) 
					throw common::Exception("ParserTable confict: follow set not disjoint");

				parserTable[ntt][t] = i;
			}
		}
	}
}

bool init()
{
	std::fill(*parserTable, *parserTable + cN * cT, E);

	first();
	follow();
	buildParserTable();
	return true;
}

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