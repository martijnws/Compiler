#include "stdafx.h"
#include <LL1RegexParser/RegexParser.h>
#include <SyntaxTreeLib/SyntaxNode.h>
#include <FA/NFABuilderVisitor.h>
#include <FA/AlphabetVisitor.h>
#include <FA/ToStringVisitor.h>
#include <FA/DFABuilder.h>
#include <FA/DFAFromNFAConvTraits.h>
#include <FA/DFAMinimize.h>
#include <gtest/gtest.h>
#include <utility>

namespace {

using TChar  = mws::CharExt;
using Buffer = mws::common::BufferT<TChar, 64>;
using Parser = mws::td::LL1::RegexParser<Buffer>;
using Stream = std::basic_stringstream<TChar>;

#define _TC(_c) _CExt(_c)
#define _TS(_c) _SExt(_c)

template<typename Item>
std::tuple<mws::DFANode*, Item*, Item*> buildDFA(mws::ast::SyntaxNode* root_)
{
    mws::ast::SyntaxNodePtr root(root_);

    mws::AlphabetVisitor alphabetVisitor;
    root->accept(alphabetVisitor);

    const auto rkSet = mws::getDisjointRangeSet(alphabetVisitor._rkVec);

    mws::NFABuilderVisitor visitor(rkSet);
    root->accept(visitor);

    auto s = visitor.startState();
    auto a = visitor.acceptState();

    auto* dfa = mws::convert(s);
    mws::minimize(dfa, rkSet);

    return std::make_tuple(dfa, a,a);
}

 auto toString(mws::ast::SyntaxNode* root_)
{
    mws::ToStringVisitor toStrVisitor;
	root_->accept(toStrVisitor);
    return toStrVisitor._result;
}

// The fixture for testing class Foo.
class ParserBasicExprTest : public ::testing::Test
{
protected:
		
};

TEST_F(ParserBasicExprTest, parseEmpty) 
{
	Stream is(_TS(""), std::ios_base::in);
	Parser parser(is);
    ASSERT_TRUE(parser.parse());
    auto* root = parser._astBuilder.detach();
    ASSERT_TRUE(root == nullptr);
}

TEST_F(ParserBasicExprTest, parseSingleChar)
{
	Stream is(_TS("a"), std::ios_base::in);
	Parser parser(is);

	EXPECT_TRUE(parser.parse());
    auto* root = parser._astBuilder.detach();
    ASSERT_TRUE(root != nullptr);

    ASSERT_EQ(_C("a"), toString(root));

    mws::DFANode* dfa; mws::NFANode* s,*a;
    ASSERT_NO_THROW({
        auto pair = buildDFA<mws::NFANode>(root);
        dfa = std::get<0>(pair); 
        s = std::get<1>(pair);
        a = std::get<2>(pair);
    });
        
    EXPECT_TRUE(mws::match(dfa, a, _TS("a")));
   
    EXPECT_FALSE(mws::match(dfa, a, _TS("")));
    EXPECT_FALSE(mws::match(dfa, a, _TS("b")));
    EXPECT_FALSE(mws::match(dfa, a, _TS("aa")));
}

TEST_F(ParserBasicExprTest, parseChoice)
{
	Stream is(_TS("a|b"), std::ios_base::in);
	Parser parser(is);

	EXPECT_TRUE(parser.parse());
    auto* root = parser._astBuilder.detach();
    ASSERT_TRUE(root != nullptr);

    ASSERT_EQ(_C("(a)|(b)"), toString(root));

    mws::DFANode* dfa; mws::NFANode* s,*a;
    ASSERT_NO_THROW({
        auto pair = buildDFA<mws::NFANode>(root);
        dfa = std::get<0>(pair); 
        s = std::get<1>(pair);
        a = std::get<2>(pair);
    });
        
    EXPECT_TRUE(mws::match(dfa, a, _TS("a")));
    EXPECT_TRUE(mws::match(dfa, a, _TS("b")));

    EXPECT_FALSE(mws::match(dfa, a, _TS("")));
    EXPECT_FALSE(mws::match(dfa, a, _TS("c")));
    EXPECT_FALSE(mws::match(dfa, a, _TS("aa")));
    EXPECT_FALSE(mws::match(dfa, a, _TS("bb")));
}

TEST_F(ParserBasicExprTest, parseConcat)
{
	Stream is(_TS("aa"), std::ios_base::in);
	Parser parser(is);

	EXPECT_TRUE(parser.parse());
}

TEST_F(ParserBasicExprTest, parseKleeneClosure)
{
	Stream is(_TS("a*"), std::ios_base::in);
	Parser parser(is);

	EXPECT_TRUE(parser.parse());
}

TEST_F(ParserBasicExprTest, parseSubExpr)
{
	Stream is(_TS("(a)"), std::ios_base::in);
	Parser parser(is);

	EXPECT_TRUE(parser.parse());
}

TEST_F(ParserBasicExprTest, parseComplexExpr)
{
	//Stream is(_TS(R"R(aaabba(aa|b)*([a-z\-A-Z]|xyz)*bb|optional)R"), std::ios_base::in);
    //Stream is(_TS(R"R(aaabba(aa|b)*(x|xyz)*bb|optional)R"), std::ios_base::in);
    Stream is(_TS(R"R([a]b)R"), std::ios_base::in);
	Parser parser(is);

	EXPECT_TRUE(parser.parse());
}

class ParserCharClassCorrectTest : public ::testing::TestWithParam<const TChar*>
{
protected:

};

TEST_P(ParserCharClassCorrectTest, parse)
{
	auto param = GetParam();

	Stream is(GetParam(), std::ios_base::in);
	Parser parser(is);

	EXPECT_TRUE(parser.parse()) << _TS("cc expr = ") << param;
}

INSTANTIATE_TEST_CASE_P(CharClass, ParserCharClassCorrectTest, ::testing::Values(
	_TS("[a]"),
	_TS("[ab]"),
	_TS("[a-z]"),
	_TS("[a-zA-Z0-9]"),
	_TS("[-]"),
	_TS("[--]"), //syntacticcaly correct, semantically wrong/redundant
	_TS("[-ab]"),
	_TS("[ab-]"),
	_TS("[a[]"),
	_TS("[a[a]")
	));
	
INSTANTIATE_TEST_CASE_P(CharClassNegate, ParserCharClassCorrectTest, ::testing::Values(
	_TS("[^a]"),
	_TS("[^ab]"),
	_TS("[^a-z]"),
	_TS("[^a-zA-Z0-9]"),
	_TS("[^-]"),
	_TS("[^-ab]"),
	_TS("[^ab-]"),
	_TS("[^[]"),
	_TS("[^a[a]"),
	_TS("[^^]"),
	_TS("[a^]"),
	_TS("[a^a]")
	));

INSTANTIATE_TEST_CASE_P(CharClassEscape, ParserCharClassCorrectTest, ::testing::Values(
	_TS(R"R([\-])R"),
	_TS(R"R([\]])R"),
	_TS(R"R([\\])R"),
	_TS(R"R([\^])R")
	));

class ParserCharClassIncorrectTest : public ::testing::TestWithParam<const TChar*>
{
protected:

};

TEST_P(ParserCharClassIncorrectTest, parse)
{
	const auto param = GetParam();

	Stream is(param, std::ios_base::in);
	Parser parser(is);

	EXPECT_FALSE(parser.parse()) << _TS("cc expr = ") << param;
}

INSTANTIATE_TEST_CASE_P(CharClass, ParserCharClassIncorrectTest, ::testing::Values(
	_TS("[]"),
	_TC("[^]"),
	_TC(R"R([\a])R")
	));
		
class ParserSymbolTest: public ::testing::TestWithParam<char>
{
protected:

};
	
TEST_P(ParserSymbolTest, parse)
{
	using AsciiStream = std::stringstream;
	using AsciiBuffer = mws::common::BufferT<char, 64>;
	using Parser = mws::td::LL1::RegexParser<AsciiBuffer>;

	char cL = GetParam();
	char cU = cL + ('A' - 'a');

	{
		char c[2] = { cL, '\0' };
		AsciiStream is(c, std::ios_base::in);
		Parser parser(is);

		EXPECT_TRUE(parser.parse());
	}

	{
		char c[2] = { cU, '\0' };
		AsciiStream is(c, std::ios_base::in);
		Parser parser(is);

		EXPECT_TRUE(parser.parse());
	}

	// For now escaped alphabetic characters are disallowed.
	{
		char c[3] = { '\\', cL, '\0' };
		AsciiStream is(c, std::ios_base::in);
		Parser parser(is);

		EXPECT_FALSE(parser.parse());
	}

	{
		char c[3] = { '\\', cU, '\0' };
		AsciiStream is(c, std::ios_base::in);
		Parser parser(is);

		EXPECT_FALSE(parser.parse());
	}
}

INSTANTIATE_TEST_CASE_P(SingleChar, ParserSymbolTest, ::testing::Range('a', (char)((int)'z' + 1)));

class ParserSpecialSymbolTest : public ::testing::TestWithParam<char>
{
protected:

};
TEST_P(ParserSpecialSymbolTest, parse)
{
	using AsciiStream = std::stringstream;
	using AsciiBuffer = mws::common::BufferT<char, 64>;
	using Parser = mws::td::LL1::RegexParser<AsciiBuffer>;

	{
		char c[3] = { '\\', GetParam(), '\0' };
		AsciiStream is(c, std::ios_base::in);
		Parser parser(is);

		EXPECT_TRUE(parser.parse());
	}

	// without escape none of the special characters is a valid expression on its own
	{
		char c[2] = { GetParam(), '\0' };
		AsciiStream is(c, std::ios_base::in);
		Parser parser(is);

		EXPECT_FALSE(parser.parse()) << "cc expr = " << GetParam();
	}
}

INSTANTIATE_TEST_CASE_P(SingleChar, ParserSpecialSymbolTest, ::testing::Values('|', '*', '(', ')', '[', ']', '\\'));

}  // namespace

int main(int argc, char **argv) 
{
    ::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
