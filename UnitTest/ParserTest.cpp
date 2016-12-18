#include "stdafx.h"
#include <LL1RegexParser/RegexParser.h>
#include <RegexSyntaxTreeLib/SyntaxNode.h>
#include <FA/NFABuilderVisitor.h>
#include <FA/AlphabetVisitor.h>
#include <FA/ToStringVisitor.h>
#include <FA/DFABuilder.h>
#include <FA/DFAFromNFAConvTraits.h>
#include <FA/DFAMinimize.h>
#include <gtest/gtest.h>
#include <utility>

namespace {

using TChar   = mws::CharExt;
using TString = std::basic_string<TChar>;
using Buffer  = mws::common::BufferT<TChar, 64>;
using Parser  = mws::td::LL1::RegexParser<Buffer>;
using Stream  = std::basic_stringstream<TChar>;

#define _TC(_c) _CExt(_c)
#define _TS(_c) _SExt(_c)

template<typename Item>
std::tuple<mws::DFANode*, Item*, Item*> buildDFA(mws::regex::SyntaxNode* root_)
{
    mws::regex::SyntaxNode::Ptr root(root_);

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

 auto toString(mws::regex::SyntaxNode* root_)
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
    ASSERT_NO_THROW(parser.parse());
    auto* root = parser._astBuilder.detach();
    ASSERT_TRUE(root == nullptr);
}

TEST_F(ParserBasicExprTest, parseSingleChar)
{
	Stream is(_TS("a"), std::ios_base::in);
	Parser parser(is);

	ASSERT_NO_THROW(parser.parse());
    auto* root = parser._astBuilder.detach();
    ASSERT_TRUE(root != nullptr);

    ASSERT_EQ(_S("a"), toString(root));

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

	ASSERT_NO_THROW(parser.parse());
    auto* root = parser._astBuilder.detach();
    ASSERT_TRUE(root != nullptr);

    ASSERT_EQ(_S("(a)|(b)"), toString(root));

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

	ASSERT_NO_THROW(parser.parse());
}

TEST_F(ParserBasicExprTest, parseKleeneClosure)
{
	Stream is(_TS("a*"), std::ios_base::in);
	Parser parser(is);

	ASSERT_NO_THROW(parser.parse());
}

TEST_F(ParserBasicExprTest, parseSubExpr)
{
	Stream is(_TS("(a)"), std::ios_base::in);
	Parser parser(is);

	ASSERT_NO_THROW(parser.parse());
}

TEST_F(ParserBasicExprTest, parseComplexExpr)
{
	//Stream is(_TS(R"R(aaabba(aa|b)*([a-z\-A-Z]|xyz)*bb|optional)R"), std::ios_base::in);
    //Stream is(_TS(R"R(aaabba(aa|b)*(x|xyz)*bb|optional)R"), std::ios_base::in);
    Stream is(_TS(R"R([a]b)R"), std::ios_base::in);
	Parser parser(is);

	ASSERT_NO_THROW(parser.parse());
}

class CCParserCorrectTest : public ::testing::TestWithParam<const TChar*>
{
protected:

};

TEST_P(CCParserCorrectTest, parse)
{
	auto param = GetParam();

	Stream is(GetParam(), std::ios_base::in);
	Parser parser(is);

	ASSERT_NO_THROW(parser.parse()) << _TS("cc expr = ") << param;
}

INSTANTIATE_TEST_CASE_P(CC, CCParserCorrectTest, ::testing::Values(
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
	
INSTANTIATE_TEST_CASE_P(CCNegate, CCParserCorrectTest, ::testing::Values(
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

INSTANTIATE_TEST_CASE_P(CCEscape, CCParserCorrectTest, ::testing::Values(
	_TS(R"R([\-])R"),
	_TS(R"R([\]])R"),
	_TS(R"R([\^])R"),

	_TS(R"R([\'])R"),
	_TS(R"R([\"])R"),
	_TS(R"R([\?])R"),
	_TS(R"R([\\])R"),
	_TS(R"R([\a])R"),
	_TS(R"R([\b])R"),
	_TS(R"R([\f])R"),
	_TS(R"R([\n])R"),
	_TS(R"R([\r])R"),
	_TS(R"R([\t])R"),
	_TS(R"R([\v])R"),
	_TS(R"R([\0])R")
	));

class CCParserIncorrectTest : public ::testing::TestWithParam<const TChar*>
{
protected:

};

TEST_P(CCParserIncorrectTest, parse)
{
	const auto param = GetParam();

	Stream is(param, std::ios_base::in);
	Parser parser(is);

	EXPECT_THROW(parser.parse(), mws::common::Exception) << _TS("cc expr = ") << param;
}

INSTANTIATE_TEST_CASE_P(CC, CCParserIncorrectTest, ::testing::Values(
	_TS("[]"),
	_TS("[^]")
	_TS("[\0]")
	));

INSTANTIATE_TEST_CASE_P(CCEscape, CCParserIncorrectTest, ::testing::Values(
	//a sample of some invalid alphabetic esc characters
	_TS(R"R([\c])R"),
	_TS(R"R([\d])R"),
	_TS(R"R([\e])R"), //and so on...
	//eof
	_TS("[\\\0]")
	));
		
class ParserSymbolTest
: 
	public ::testing::TestWithParam<char>
{
protected:

};

class ParserSymbolEscNoTest
: 
	public ::testing::TestWithParam<char>
{
protected:

};

class ParserSymbolEscYesTest
:	
	public ::testing::TestWithParam<char>
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

		ASSERT_NO_THROW(parser.parse());
	}

	{
		char c[2] = { cU, '\0' };
		AsciiStream is(c, std::ios_base::in);
		Parser parser(is);

		ASSERT_NO_THROW(parser.parse());
	}
}

static const auto escYNCharCol = std::string("abcdefghijklmnopqrstuvwxyz");
INSTANTIATE_TEST_CASE_P(SingleChar, ParserSymbolTest, ::testing::ValuesIn(escYNCharCol.begin(), escYNCharCol.end()));

TEST_P(ParserSymbolEscYesTest, parse)
{
	using AsciiStream = std::stringstream;
	using AsciiBuffer = mws::common::BufferT<char, 64>;
	using Parser = mws::td::LL1::RegexParser<AsciiBuffer>;

	char cL = GetParam();
	char cU = cL + ('A' - 'a');

	{
		char c[3] = { '\\', cL, '\0' };
		AsciiStream is(c, std::ios_base::in);
		Parser parser(is);

		EXPECT_NO_THROW(parser.parse());
	}

	//UpperCase versions of c not valid
	{
		char c[3] = { '\\', cU, '\0' };
		AsciiStream is(c, std::ios_base::in);
		Parser parser(is);

		EXPECT_THROW(parser.parse(), mws::common::Exception);
	}
}

static const auto escYCharCol  = std::string("abfnrtv");
INSTANTIATE_TEST_CASE_P(SingleChar, ParserSymbolEscYesTest, ::testing::ValuesIn(escYCharCol.begin(), escYCharCol.end()));

TEST_P(ParserSymbolEscNoTest, parse)
{
	using AsciiStream = std::stringstream;
	using AsciiBuffer = mws::common::BufferT<char, 64>;
	using Parser = mws::td::LL1::RegexParser<AsciiBuffer>;

	char cL = GetParam();
	char cU = cL + ('A' - 'a');

	{
		char c[3] = { '\\', cL, '\0' };
		AsciiStream is(c, std::ios_base::in);
		Parser parser(is);

		EXPECT_THROW(parser.parse(), mws::common::Exception);
	}

	{
		char c[3] = { '\\', cU, '\0' };
		AsciiStream is(c, std::ios_base::in);
		Parser parser(is);

		EXPECT_THROW(parser.parse(), mws::common::Exception);
	}
}

static const auto escNCharCol  = std::string("cdeghijklmopqsuwxyz");
INSTANTIATE_TEST_CASE_P(SingleChar, ParserSymbolEscNoTest, ::testing::ValuesIn(escNCharCol.begin(), escNCharCol.end()));

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

		ASSERT_NO_THROW(parser.parse());
	}

	// without escape none of the special characters is a valid expression on its own
	{
		char c[2] = { GetParam(), '\0' };
		AsciiStream is(c, std::ios_base::in);
		Parser parser(is);

		EXPECT_THROW(parser.parse(), mws::common::Exception) << "cc expr = " << GetParam();
	}
}

INSTANTIATE_TEST_CASE_P(SingleChar, ParserSpecialSymbolTest, ::testing::Values('|', '*', '(', ')', '[', ']', '\\'));

}  // namespace

int main(int argc, char **argv) 
{
    ::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
