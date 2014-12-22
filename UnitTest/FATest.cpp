#include "stdafx.h"
#include <FA/NFA.h>
#include <vector>
#include <gtest/gtest.h>

using namespace mws;

namespace {

class DisjointSetTest : public ::testing::Test
{
protected:
	
};

TEST_F(DisjointSetTest, makeDisjointSet1)
{
    std::vector<RangeKey> rkVec;
	rkVec.push_back(RangeKey('a', 'a'));
    rkVec.push_back(RangeKey('a', 'a'));

    std::set<RangeKey> rkSet = getDisjointRangeSet(rkVec);

    ASSERT_EQ(rkSet.size(), 1);
    auto itr = rkSet.begin();

    ASSERT_EQ(*itr, RangeKey('a', 'a'));
}

TEST_F(DisjointSetTest, makeDisjointSet2)
{
    std::vector<RangeKey> rkVec;
	rkVec.push_back(RangeKey(NFA::E, NFA::E));
    rkVec.push_back(RangeKey(NFA::E, NFA::E));

    std::set<RangeKey> rkSet = getDisjointRangeSet(rkVec);

    ASSERT_EQ(rkSet.size(), 1);
    auto itr = rkSet.begin();

    ASSERT_EQ(*itr, RangeKey(NFA::E, NFA::E));
}

TEST_F(DisjointSetTest, makeDisjointSet3)
{
    std::vector<RangeKey> rkVec;
	rkVec.push_back(RangeKey('a', 'g'));
    rkVec.push_back(RangeKey('d', 'z'));

    std::set<RangeKey> rkSet = getDisjointRangeSet(rkVec);

    ASSERT_EQ(rkSet.size(), 3);
    auto itr = rkSet.begin();

    ASSERT_EQ(*itr, RangeKey('a', 'c'));
    ++itr;
    ASSERT_EQ(*itr, RangeKey('d', 'g'));
    ++itr;
    ASSERT_EQ(*itr, RangeKey('h', 'z'));
}

TEST_F(DisjointSetTest, makeDisjointSet4)
{
    std::vector<RangeKey> rkVec;
	rkVec.push_back(RangeKey('a', 'z'));
    rkVec.push_back(RangeKey('f', 'f'));

    std::set<RangeKey> rkSet = getDisjointRangeSet(rkVec);

    ASSERT_EQ(rkSet.size(), 3);
    auto itr = rkSet.begin();

    ASSERT_EQ(*itr, RangeKey('a', 'e'));
    ++itr;
    ASSERT_EQ(*itr, RangeKey('f', 'f'));
    ++itr;
    ASSERT_EQ(*itr, RangeKey('g', 'z'));
}

TEST_F(DisjointSetTest, makeDisjointSet5)
{
    std::vector<RangeKey> rkVec;
	rkVec.push_back(RangeKey(0, 64));
    rkVec.push_back(RangeKey(91, 96));
    rkVec.push_back(RangeKey(123, 254));

    std::set<RangeKey> rkSet = getDisjointRangeSet(rkVec);

    ASSERT_EQ(rkSet.size(), 3);
    auto itr = rkSet.begin();

    ASSERT_EQ(*itr, RangeKey(0, 64));
    ++itr;
    ASSERT_EQ(*itr, RangeKey(91, 96));
    ++itr;
    ASSERT_EQ(*itr, RangeKey(123, 254));
}

} // NS