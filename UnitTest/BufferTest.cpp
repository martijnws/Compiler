#include "stdafx.h"
#include <CommonLib/Buffer.h>
#include <gtest/gtest.h>

namespace {

template<std::size_t ValueT>
using Buffer = mws::common::BufferT<ValueT>;

template<std::size_t ValueT>
struct Size
{
	static const std::size_t value = ValueT;
};

// The fixture for testing class Foo.
template<typename SizeT>
class BufferBasicTest : public ::testing::Test
{
protected:
	using Size = SizeT;
};

typedef ::testing::Types<Size<1>, Size<2>, Size<3>, Size<4>, Size<5>, Size<6>, Size<7>, Size<8>, Size<9>, Size<16>> BufferSizeList;

TYPED_TEST_CASE(BufferBasicTest, BufferSizeList);

TYPED_TEST(BufferBasicTest, forwardOnly) 
{
	using Size = typename TestFixture::Size;

	std::stringstream is("01234567", std::ios_base::in);
	Buffer<Size::value> buf(is);
	ASSERT_TRUE(buf.valid());
	// load first char
	buf.next();

	for (std::size_t i = 0; i < 8; ++i)
	{
		const char c = static_cast<char>('0' + i);

		ASSERT_TRUE(buf.valid())    << "c := " << c;
		ASSERT_TRUE(buf.cur() == c) << "c := " << c;

		buf.next();
	}

	EXPECT_FALSE(buf.valid());
}

class BufferRetractTest : public ::testing::Test
{
protected:
	
};

TEST_F(BufferRetractTest, retractBeforePos)
{
	std::stringstream is("01234567", std::ios_base::in);
	Buffer<4> buf(is);
	ASSERT_TRUE(buf.valid());
	// load first char
	buf.next();

	ASSERT_TRUE(buf.valid());
	ASSERT_EQ(buf.pos(), 0);
	ASSERT_EQ(buf.cur(), '0');

	EXPECT_THROW(buf.retract(1), mws::common::Exception);
	// operation should have no side affects
	ASSERT_TRUE(buf.valid());
	ASSERT_EQ(buf.pos(), 0);
	ASSERT_EQ(buf.cur(), '0');
	
	buf.next();
	buf.next();
	ASSERT_TRUE(buf.valid());
	ASSERT_EQ(buf.pos(), 2);
	ASSERT_EQ(buf.cur(), '2');

	EXPECT_THROW(buf.retract(3), mws::common::Exception);
}

TEST_F(BufferRetractTest, retractExceedBufferSize)
{
	const std::string input = { "01234567" };
	std::stringstream is(input, std::ios_base::in);
	Buffer<4> buf(is);
	// load first char
	buf.next();

	// move more than 4 positions
	for (std::size_t i = 0; i < 6; ++i)
	{
		ASSERT_TRUE(buf.valid());
		ASSERT_EQ(buf.pos(), i);
		ASSERT_EQ(buf.cur(), input[i]);

		buf.next();
	}
	
	ASSERT_TRUE(buf.valid());
	ASSERT_EQ(buf.pos(), 6);
	ASSERT_EQ(buf.cur(), '6');

	EXPECT_THROW(buf.retract(5), mws::common::Exception);
}

TEST_F(BufferRetractTest, retractMaxSuccess)
{
	const std::string input = { "0123456789ABCDEF" };
	std::stringstream is(input, std::ios_base::in);
	Buffer<4> buf(is);
	// load first char
	buf.next();

	ASSERT_TRUE(buf.valid());

	// move more than BufSize positions
	for (std::size_t i = 0; i < 6; ++i)
	{
		buf.next();
		ASSERT_TRUE(buf.valid());
	}
	
	ASSERT_EQ(buf.pos(), 6);
	ASSERT_EQ(buf.cur(), '6');

	EXPECT_NO_THROW(buf.retract(4));

	// reiterate positions and beyond
	for (std::size_t i = 2; i < 10; ++i)
	{
		ASSERT_TRUE(buf.valid());
		ASSERT_EQ(buf.pos(), i);
		ASSERT_EQ(buf.cur(), input[i]);

		buf.next();
	}

	ASSERT_EQ(buf.pos(), 10);
	ASSERT_EQ(buf.cur(), 'A');

	EXPECT_NO_THROW(buf.retract(4));

	// reiterate positions and beyond
	for (std::size_t i = 6; i < 16; ++i)
	{
		ASSERT_TRUE(buf.valid());
		ASSERT_EQ(buf.pos(), i);
		ASSERT_EQ(buf.cur(), input[i]);

		buf.next();
	}
}

} // NS