#include "stdafx.h"
#include <RegexLL1ParserLib/Parser.h>
#include <gtest/gtest.h>

namespace {

typedef mws::td::LL1::Parser Parser;

// The fixture for testing class Foo.
class ParserBasicExprTest : public ::testing::Test
{
protected:
		
};

TEST_F(ParserBasicExprTest, parseEmpty) 
{
	std::stringstream is("", std::ios_base::in);
	Parser parser(is);

	EXPECT_TRUE(parser.parse());
}

TEST_F(ParserBasicExprTest, parseChoice)
{
	std::stringstream is("a|b", std::ios_base::in);
	Parser parser(is);

	EXPECT_TRUE(parser.parse());
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
    mws::td::LL1::init();
	
    ::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
