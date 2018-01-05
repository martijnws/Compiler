// TestApp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Parser.h"
#include <FA/Lexer.h>
#include <FA/ToStringVisitor.h>
#include <FA/NFABuilderVisitor.h>
#include <FA/DFAInfoBuilderVisitor.h>
#include <FA/AlphabetVisitor.h>
#include <FA/DFABuilder.h>
#include <FA/DFAMinimize.h>
#include <FA/DFAFromFirstFollowPosConvTraits.h>
#include <FA/DFAFromNFAConvTraits.h>
#include <LL1RegexParser/RegexParser.h>
#include <SyntaxTreeLib/SyntaxNode.h>
#include <CommonLib/String.h>

class Token
	:
	public mws::grammar::Token
{
public:
	enum Enum { WS, Num, If, Else, Break, Continue, Class, ID, BlockO, BlockC, Eos, Eof, Invalid /*end of statement*/ };

	static mws::TokenID max()
	{
		return Invalid;
	}

	bool isLast() const
	{
		return _type == Enum::Eof;
	}

	mws::String _lexeme;
	Enum        _type;
};

using TI = mws::Lexer::TokenInfo;

std::vector<TI> g_tokenInfoCol =
{
	TI( _CExt("[ \t\n]+"),                                Token::Enum::WS, true ),
	TI( _CExt("[0-9]+"),                                  Token::Enum::Num ),
    TI( _CExt("if"),                                      Token::Enum::If ),
    TI( _CExt("else"),                                    Token::Enum::Else ),
    TI( _CExt("break"),                                   Token::Enum::Break ),
    TI( _CExt("continue"),                                Token::Enum::Continue ),
    TI( _CExt("class"),                                   Token::Enum::Class ),
    // Note: let a = a-zA-Z0-9, b = _
    // the pattern (a|b)*a(a|b)* reduces to b*a(a|b)*
    //_CExt("[a-zA-Z_]_*[a-zA-Z0-9][a-zA-Z0-9_]*"),
    TI( _CExt("[a-zA-Z_]*[a-zA-Z][a-zA-Z_]*"),			  Token::Enum::ID ),
    TI( _CExt("{"),                                       Token::Enum::BlockO ),
	TI( _CExt("}"),                                       Token::Enum::BlockC ),
	TI( _CExt(";"),                                       Token::Enum::Eos ),
	TI{ _CExt("\\0"),                                     Token::Enum::Eof } //Doesn't work yet
};

void testMatchAndSimulate(mws::DFANode* dfa_, const mws::DFAInfo* s_, const mws::DFAInfo* a_, const mws::Char* str_, bool expect_ = true)
{
	auto res = false;
	res = match(dfa_, a_, str_);
	assert(res == expect_);
	res = simulate(s_, a_, str_);
	assert(res == expect_);
}

int _tmain(int argc, _TCHAR* argv[])
{
	try
	{
		const auto* expr = _CExt("1+2");

        mws::StringStreamExt is(expr, std::ios_base::in);
		mws::arith::Parser parser(is);
		parser.parse();
	}
	catch(const mws::common::Exception& e)
	{
		stdOut << e.what() << std::endl;
	}

	/*
	const auto* regex = "[a-z]";
	std::stringstream is(regex, std::ios::in);
	mws::td::LL1::Parser parser(is);
	parser.parse();
	auto root = parser._astBuilder.detach();
	*/
	
	try
    {
        //const auto* text = _CExt("hello world; if bla ___0_   { continue; } else elsbla else1234 { bla 1234 break; }");
        //const auto* text = _CExt("hello world; if bla ___A_   { continue; } else elsbla else1234 { bla 1234 break; }");
        const auto* text = _CExt("a");


        //const auto* text = _CExt("ababc");
        mws::StringStreamExt is(text, std::ios_base::in);

        mws::LexerT<Token> lexer(is, g_tokenInfoCol);
        for (auto t = lexer.next(); !t.isLast(); t = lexer.next())
        {
            stdOut << _C("lexeme = ") << t._lexeme << _C(", type = ") << reinterpret_cast<const wchar_t*>(g_tokenInfoCol[t._type].regex.c_str()) << std::endl;
        }
    }
	catch(mws::common::Exception& e)
	{
		stdOut << e.what() << std::endl;
	}

    const auto* regex = _CExt("abc[^a-zA-Z]*def|abc(a|ab)*ab*");
	mws::StringStreamExt is(regex, std::ios_base::in);
	mws::td::LL1::RegexParser<mws::common::BufferExt> parser(is);

	parser.parse();
    mws::regex::SyntaxNode::Ptr root(parser._astBuilder.detach());

    {
        mws::ToStringVisitor visitor;
	    root->accept(visitor);
	    stdOut << visitor._result << std::endl;
    }

    {
        mws::AlphabetVisitor alphabetVisitor;
        root->accept(alphabetVisitor);

        const auto rkSet = mws::getDisjointRangeSet(alphabetVisitor._rkVec);

        //mws::NFABuilderVisitor visitor(rkSet);
        //using DFAItem = mws::NFANode;
        
        mws::DFAInfoBuilderVisitor visitor(rkSet);
        using DFAItem = mws::DFAInfo;
        
        //using DFANode = mws::DFANode<DFAItem>;

        root->accept(visitor);

        auto s = visitor.startState();
        auto a = visitor.acceptState();

        mws::DFANode* dfa = mws::convert(s);
        mws::minimize(dfa, rkSet);

        testMatchAndSimulate(dfa, s, a, _C("abc@##^def"));
        testMatchAndSimulate(dfa, s, a, _C("abcaabaaabaaababaaabbb"));

        testMatchAndSimulate(dfa, s, a, _C("abc@##^ddef"), false);
        testMatchAndSimulate(dfa, s, a, _C("abcaabaaabaaabbabaaabbb"), false);
    }

	return 0;
}

