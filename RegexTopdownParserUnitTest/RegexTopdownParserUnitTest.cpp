// RegexTopdownParserUnitTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../RegexTopdownParser/Parser.h"
#include <gtest/gtest.h>

namespace {

	// The fixture for testing class Foo.
	class ParserTest : public ::testing::Test 
	{
	protected:
		// You can remove any or all of the following functions if its body
		// is empty.

		ParserTest() 
		{
			// You can do set-up work for each test here.
		}

		virtual ~ParserTest() 
		{
			// You can do clean-up work that doesn't throw exceptions here.
		}

		// If the constructor and destructor are not enough for setting up
		// and cleaning up each test, you can define the following methods:

		virtual void SetUp() 
		{
			// Code here will be called immediately after the constructor (right
			// before each test).
		}

		virtual void TearDown() 
		{
			// Code here will be called immediately after each test (right
			// before the destructor).
		}

		// Objects declared here can be used by all tests in the test case for Foo.
	};

	TEST_F(ParserTest, parseEmptyString) 
	{
		std::stringstream is("", std::ios_base::in);

		Parser parser(is);
		EXPECT_EQ(true, parser.parse());
	}

	TEST_F(ParserTest, parseBasicRegex)
	{
		std::stringstream is("aa(a|b)*bb", std::ios_base::in);

		Parser parser(is);
		EXPECT_EQ(true, parser.parse());
	}


}  // namespace

int main(int argc, char **argv) 
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}