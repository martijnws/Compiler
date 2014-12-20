#include "stdafx.h"
#include <RegexLL1ParserLib/Parser.h>
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

typedef mws::td::LL1::Parser Parser;

template<typename Item>
std::tuple<mws::DFANode*, Item*, Item*> buildDFA(mws::ast::SyntaxNode* root_)
{
    mws::ast::SyntaxNodePtr root(root_);

    mws::AlphabetVisitor alphabetVisitor;
    root->accept(alphabetVisitor);

    std::set<mws::RangeKey, mws::RangeKey::Less> rkSet = mws::getDisjointRangeSet(alphabetVisitor._rkVec);

    mws::NFABuilderVisitor visitor(rkSet);
    root->accept(visitor);

    auto s = visitor.startState();
    auto a = visitor.acceptState();

    mws::DFANode* dfa = mws::convert(s);
    mws::minimize(dfa, rkSet);

    return std::make_tuple(dfa, a,a);
}

std::string toString(mws::ast::SyntaxNode* root_)
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
	std::stringstream is("", std::ios_base::in);
	Parser parser(is);
    ASSERT_TRUE(parser.parse());
    mws::ast::SyntaxNode* root = parser._astBuilder.detach();
    ASSERT_TRUE(root == nullptr);
}

TEST_F(ParserBasicExprTest, parseSingleChar)
{
	std::stringstream is("a", std::ios_base::in);
	Parser parser(is);

	EXPECT_TRUE(parser.parse());
    mws::ast::SyntaxNode* root = parser._astBuilder.detach();
    ASSERT_TRUE(root != nullptr);

    ASSERT_EQ("a", toString(root));

    mws::DFANode* dfa; mws::NFANode* s,*a;
    ASSERT_NO_THROW({
        auto pair = buildDFA<mws::NFANode>(root);
        dfa = std::get<0>(pair); 
        s = std::get<1>(pair);
        a = std::get<2>(pair);
    });
        
    EXPECT_TRUE(mws::match(dfa, a, "a"));
   
    EXPECT_FALSE(mws::match(dfa, a, ""));
    EXPECT_FALSE(mws::match(dfa, a, "b"));
    EXPECT_FALSE(mws::match(dfa, a, "aa"));
}

TEST_F(ParserBasicExprTest, parseChoice)
{
	std::stringstream is("a|b", std::ios_base::in);
	Parser parser(is);

	EXPECT_TRUE(parser.parse());
    mws::ast::SyntaxNode* root = parser._astBuilder.detach();
    ASSERT_TRUE(root != nullptr);

    ASSERT_EQ("(a)|(b)", toString(root));

    mws::DFANode* dfa; mws::NFANode* s,*a;
    ASSERT_NO_THROW({
        auto pair = buildDFA<mws::NFANode>(root);
        dfa = std::get<0>(pair); 
        s = std::get<1>(pair);
        a = std::get<2>(pair);
    });
        
    EXPECT_TRUE(mws::match(dfa, a, "a"));
    EXPECT_TRUE(mws::match(dfa, a, "b"));

    EXPECT_FALSE(mws::match(dfa, a, ""));
    EXPECT_FALSE(mws::match(dfa, a, "c"));
    EXPECT_FALSE(mws::match(dfa, a, "aa"));
    EXPECT_FALSE(mws::match(dfa, a, "bb"));
}

TEST_F(ParserBasicExprTest, parseConcat)
{
	std::stringstream is("aa", std::ios_base::in);
	Parser parser(is);

	EXPECT_TRUE(parser.parse());
}

TEST_F(ParserBasicExprTest, parseKleeneClosure)
{
	std::stringstream is("a*", std::ios_base::in);
	Parser parser(is);

	EXPECT_TRUE(parser.parse());
}

TEST_F(ParserBasicExprTest, parseSubExpr)
{
	std::stringstream is("(a)", std::ios_base::in);
	Parser parser(is);

	EXPECT_TRUE(parser.parse());
}

TEST_F(ParserBasicExprTest, parseComplexExpr)
{
	std::stringstream is(R"R(aaabba(aa|b)*([a-z\-A-Z]|xyz)*bb|optional)R", std::ios_base::in);
	Parser parser(is);

	EXPECT_TRUE(parser.parse());
}

class ParserCharClassCorrectTest : public ::testing::TestWithParam<const char*>
{
protected:

};

TEST_P(ParserCharClassCorrectTest, parse)
{
	std::stringstream is(GetParam(), std::ios_base::in);
	Parser parser(is);

	EXPECT_TRUE(parser.parse()) << "cc expr = " << GetParam();
}

INSTANTIATE_TEST_CASE_P(CharClass, ParserCharClassCorrectTest, ::testing::Values(
	"[a]",
	"[ab]",
	"[a-z]",
	"[a-zA-Z0-9]",
	"[-]",
	"[--]", //syntacticcaly correct, semantically wrong/redundant
	"[-ab]",
	"[ab-]",
	"[a[]",
	"[a[a]"
	));
	
INSTANTIATE_TEST_CASE_P(CharClassNegate, ParserCharClassCorrectTest, ::testing::Values(
	"[^a]",
	"[^ab]",
	"[^a-z]",
	"[^a-zA-Z0-9]",
	"[^-]",
	"[^-ab]",
	"[^ab-]",
	"[^[]",
	"[^a[a]",
	"[^^]",
	"[a^]",
	"[a^a]"
	));

INSTANTIATE_TEST_CASE_P(CharClassEscape, ParserCharClassCorrectTest, ::testing::Values(
	R"R([\-])R",
	R"R([\]])R",
	R"R([\\])R",
	R"R([\^])R"
	));

class ParserCharClassIncorrectTest : public ::testing::TestWithParam<const char*>
{
protected:

};

TEST_P(ParserCharClassIncorrectTest, parse)
{
	std::stringstream is(GetParam(), std::ios_base::in);
	Parser parser(is);

	EXPECT_FALSE(parser.parse()) << "cc expr = " << GetParam();
}

INSTANTIATE_TEST_CASE_P(CharClass, ParserCharClassIncorrectTest, ::testing::Values(
	"[]",
	"[^]",
	R"R([\a])R"
	));
		
class ParserSymbolTest: public ::testing::TestWithParam<char>
{
protected:

};
	
TEST_P(ParserSymbolTest, parse)
{
	char cL = GetParam();
	char cU = cL + ('A' - 'a');

	{
		char c[2] = { cL, '\0' };
		std::stringstream is(c, std::ios_base::in);
		Parser parser(is);

		EXPECT_TRUE(parser.parse());
	}

	{
		char c[2] = { cU, '\0' };
		std::stringstream is(c, std::ios_base::in);
		Parser parser(is);

		EXPECT_TRUE(parser.parse());
	}

	// For now escaped alphabetic characters are disallowed.
	{
		char c[3] = { '\\', cL, '\0' };
		std::stringstream is(c, std::ios_base::in);
		Parser parser(is);

		EXPECT_FALSE(parser.parse());
	}

	{
		char c[3] = { '\\', cU, '\0' };
		std::stringstream is(c, std::ios_base::in);
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
	{
		char c[3] = { '\\', GetParam(), '\0' };
		std::stringstream is(c, std::ios_base::in);
		Parser parser(is);

		EXPECT_TRUE(parser.parse());
	}

	// without escape none of the special characters is a valid expression on its own
	{
		char c[2] = { GetParam(), '\0' };
		std::stringstream is(c, std::ios_base::in);
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
