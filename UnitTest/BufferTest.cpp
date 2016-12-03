#include "stdafx.h"
#include <CommonLib/Buffer.h>
#include <gtest/gtest.h>

namespace {

//Use max Char size so that Buffer size >= MaxCodecSizePerCP for any size
template<std::size_t ValueT>
using Buffer = mws::common::BufferT<char32_t, ValueT>;
using Stream = std::basic_stringstream<char32_t>;
using String = std::basic_string<char32_t>;

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

	Stream is(U"01234567", std::ios_base::in);
	Buffer<Size::value> buf(is);
	ASSERT_TRUE(buf.valid());
	// load first char
	auto cp = buf.next();

	for (std::size_t i = 0; i < 8; ++i)
	{
		const char c = static_cast<char>('0' + i);

		ASSERT_TRUE(buf.valid())    << "c := " << c;
		ASSERT_TRUE(cp == c) << "c := " << c;

		cp = buf.next();
	}

	EXPECT_FALSE(buf.valid());
}

class BufferRetractTest : public ::testing::Test
{
protected:
	
};

TEST_F(BufferRetractTest, retractBeforePos)
{
	Stream is(U"01234567", std::ios_base::in);
	Buffer<4> buf(is);
	ASSERT_TRUE(buf.valid());
	// load first char
	auto cp = buf.next();

	ASSERT_TRUE(buf.valid());
	ASSERT_EQ(buf.pos(), 0 + 1);
	ASSERT_EQ(cp, CP('0'));

	EXPECT_THROW(buf.retract(2), mws::common::Exception);
	// operation should have no side affects
	ASSERT_TRUE(buf.valid());
	ASSERT_EQ(buf.pos(), 0 + 1);
	
	cp = buf.next();
	ASSERT_EQ(cp, CP('1'));
	cp = buf.next();
	ASSERT_TRUE(buf.valid());
	ASSERT_EQ(buf.pos(), 2 + 1);
	ASSERT_EQ(cp, CP('2'));

	EXPECT_THROW(buf.retract(4), mws::common::Exception);
}

TEST_F(BufferRetractTest, retractExceedBufferSize)
{
	const String input = { U"01234567" };
	Stream is(input, std::ios_base::in);
	Buffer<4> buf(is);
	// load first char
	auto cp = buf.next();

	// move more than 4 positions
	for (auto i = 0ul; i < 6; ++i)
	{
		ASSERT_TRUE(buf.valid());
		ASSERT_EQ(buf.pos(), i + 1);
		ASSERT_EQ(cp, input[i]);

		cp = buf.next();
	}
	
	ASSERT_TRUE(buf.valid());
	ASSERT_EQ(buf.pos(), 6 + 1);
	ASSERT_EQ(cp, CP('6'));

	EXPECT_THROW(buf.retract(5), mws::common::Exception);
}

TEST_F(BufferRetractTest, retractMaxSuccess)
{
	const String input = { U"0123456789ABCDEF" };
	Stream is(input, std::ios_base::in);
	Buffer<4> buf(is);
	// load first char
	auto cp = buf.next();

	ASSERT_TRUE(buf.valid());

	// move more than BufSize positions
	for (auto i = 0ul; i < 6; ++i)
	{
		cp = buf.next();
		ASSERT_TRUE(buf.valid());
	}
	
	ASSERT_EQ(buf.pos(), 6 + 1);
	ASSERT_EQ(cp, CP('6'));

	EXPECT_NO_THROW(buf.retract(4));

	// reiterate positions and beyond
	for (auto i = 2ul; i < 10; ++i)
	{
		ASSERT_TRUE(buf.valid());
		ASSERT_EQ(buf.pos(), i + 1);

		cp = buf.next();
		ASSERT_EQ(cp, input[i + 1]);
	}

	ASSERT_EQ(buf.pos(), 10 + 1);
	ASSERT_EQ(cp, CP('A'));

	EXPECT_NO_THROW(buf.retract(4));

	// reiterate positions and beyond
	for (auto i = 7ul; i < 15; ++i)
	{
		cp = buf.next();
		ASSERT_TRUE(buf.valid());
		ASSERT_EQ(buf.pos(), i + 1);
		ASSERT_EQ(cp, input[i]);
	}
}

} // NS