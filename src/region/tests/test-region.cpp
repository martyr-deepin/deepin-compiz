#include "core/region.h"

#undef Bool
#undef None

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <iostream>

namespace { // DEBUG stuff
}

namespace {

  int const x1(13);
  int const y1(11);
  int const width1(97);
  int const height1(93);

  int const x2(53);
  int const y2(47);
  int const width2(147);
  int const height2(157);

  CompRect rect1(x1, y1, width1, height1);
  CompRect rect2(x2, y2, width2, height2);

  int const dx(3);
  int const dy(5);
}

TEST(RegionTest, create_and_destroy)
{
    CompRegion default_ctor;
    CompRegion xywh_ctor(x1, y1, width1, height1);
    EXPECT_TRUE(default_ctor.isEmpty());
    EXPECT_FALSE(xywh_ctor.isEmpty());
}

TEST(RegionTest, create_from_points_and_destroy)
{
    CompRegion rect_ctor(rect1);
}

TEST(RegionTest, create_compare_destroy1)
{
    CompRegion expect(x1, y1, width1, height1);
    CompRegion actual(x1, y1, width1, height1);

    EXPECT_EQ(expect.boundingRect (), actual.boundingRect ());
    EXPECT_EQ(expect.rects (), actual.rects ());
    EXPECT_EQ(expect.isEmpty (), actual.isEmpty ());
    EXPECT_EQ(expect.numRects (), actual.numRects ());

    ASSERT_EQ(expect, actual);
}

TEST(RegionTest, create_compare_destroy2)
{
    CompRegion expect(rect1);
    CompRegion actual(rect1);

    EXPECT_EQ(expect.boundingRect (), actual.boundingRect ());
    EXPECT_EQ(expect.rects (), actual.rects ());
    EXPECT_EQ(expect.isEmpty (), actual.isEmpty ());
    EXPECT_EQ(expect.numRects (), actual.numRects ());

    ASSERT_EQ(expect, actual);
}

TEST(RegionTest, create_copy_compare_destroy1)
{
    CompRegion test(x1, y1, width1, height1);
    CompRegion copy(test);

    EXPECT_EQ(test.boundingRect (), copy.boundingRect ());
    EXPECT_EQ(test.rects (), copy.rects ());
    EXPECT_EQ(test.isEmpty (), copy.isEmpty ());
    EXPECT_EQ(test.numRects (), copy.numRects ());

    //ASSERT_EQ(test, copy);	// TODO: find out why this doesn't work
}

TEST(RegionTest, create_copy_compare_destroy2)
{
    CompRegion test(rect1);
    CompRegion copy(test);

    EXPECT_EQ(test.boundingRect (), copy.boundingRect ());
    EXPECT_EQ(test.rects (), copy.rects ());
    EXPECT_EQ(test.isEmpty (), copy.isEmpty ());
    EXPECT_EQ(test.numRects (), copy.numRects ());

    ASSERT_EQ(test, copy);
}

TEST(RegionTest, create_assign_compare_destroy1)
{
    CompRegion test(x1, y1, width1, height1);
    CompRegion copy;
    copy = test;

    EXPECT_EQ(test.boundingRect (), copy.boundingRect ());
    EXPECT_EQ(test.rects (), copy.rects ());
    EXPECT_EQ(test.isEmpty (), copy.isEmpty ());
    EXPECT_EQ(test.numRects (), copy.numRects ());

    ASSERT_EQ(test, copy);
}

TEST(RegionTest, create_assign_compare_destroy2)
{
    CompRegion test(rect1);
    CompRegion copy;
    copy = test;

    EXPECT_EQ(test.boundingRect (), copy.boundingRect ());
    EXPECT_EQ(test.rects (), copy.rects ());
    EXPECT_EQ(test.isEmpty (), copy.isEmpty ());
    EXPECT_EQ(test.numRects (), copy.numRects ());

    ASSERT_EQ(test, copy);
}

TEST(RegionTest, contains_points)
{
    CompRegion r1(x1, y1, width1, height1);

    EXPECT_TRUE(r1.contains(CompPoint(x1, y1)));
    EXPECT_TRUE(r1.contains(CompPoint(x1, y1+height1-1)));
    EXPECT_TRUE(r1.contains(CompPoint(x1+width1-1, y1)));
    EXPECT_TRUE(r1.contains(CompPoint(x1+width1-1, y1+height1-1)));
    EXPECT_TRUE(r1.contains(CompPoint(x1+width1/2, y1+height1/2)));

    EXPECT_FALSE(r1.contains(CompPoint(x1-1, y1)));
    EXPECT_FALSE(r1.contains(CompPoint(x1, y1-1)));
    EXPECT_FALSE(r1.contains(CompPoint(x1+width1, y1)));
    EXPECT_FALSE(r1.contains(CompPoint(x1, y1+height1)));
    EXPECT_FALSE(r1.contains(CompPoint(x1+width1, y1+height1)));

    CompRegion r2(rect1);

    EXPECT_TRUE(r2.contains(CompPoint(x1, y1)));
    EXPECT_TRUE(r2.contains(CompPoint(x1, y1+height1-1)));
    EXPECT_TRUE(r2.contains(CompPoint(x1+width1-1, y1)));
    EXPECT_TRUE(r2.contains(CompPoint(x1+width1-1, y1+height1-1)));
    EXPECT_TRUE(r2.contains(CompPoint(x1+width1/2, y1+height1/2)));

    EXPECT_FALSE(r2.contains(CompPoint(x1-1, y1)));
    EXPECT_FALSE(r2.contains(CompPoint(x1, y1-1)));
    EXPECT_FALSE(r2.contains(CompPoint(x1+width1, y1)));
    EXPECT_FALSE(r2.contains(CompPoint(x1, y1+height1)));
    EXPECT_FALSE(r2.contains(CompPoint(x1+width1, y1+height1)));
}

TEST(RegionTest, contains_rects)
{
    CompRegion r1(x1, y1, width1, height1);

    EXPECT_TRUE(r1.contains(CompRect(x1,   y1,   width1,   height1)));
    EXPECT_TRUE(r1.contains(CompRect(x1,   y1+1, width1,   height1-1)));
    EXPECT_TRUE(r1.contains(CompRect(x1+1, y1,   width1-1, height1)));
    EXPECT_TRUE(r1.contains(CompRect(x1,   y1,   width1-1, height1)));
    EXPECT_TRUE(r1.contains(CompRect(x1,   y1,   width1,   height1-1)));

    EXPECT_FALSE(r1.contains(CompRect(x1+1, y1,   width1,   height1)));
    EXPECT_FALSE(r1.contains(CompRect(x1,   y1+1, width1,   height1)));
    EXPECT_FALSE(r1.contains(CompRect(x1,   y1,   width1+1, height1)));
    EXPECT_FALSE(r1.contains(CompRect(x1,   y1,   width1,   height1+1)));
    EXPECT_FALSE(r1.contains(CompRect(x1-1, y1,   width1,   height1)));
    EXPECT_FALSE(r1.contains(CompRect(x1,   y1-1, width1,   height1)));

    CompRegion r2(rect1);

    EXPECT_TRUE(r2.contains(CompRect(x1,   y1,   width1,   height1)));
    EXPECT_TRUE(r2.contains(CompRect(x1,   y1+1, width1,   height1-1)));
    EXPECT_TRUE(r2.contains(CompRect(x1+1, y1,   width1-1, height1)));
    EXPECT_TRUE(r2.contains(CompRect(x1,   y1,   width1-1, height1)));
    EXPECT_TRUE(r2.contains(CompRect(x1,   y1,   width1,   height1-1)));

    EXPECT_FALSE(r2.contains(CompRect(x1+1, y1,   width1,   height1)));
    EXPECT_FALSE(r2.contains(CompRect(x1,   y1+1, width1,   height1)));
    EXPECT_FALSE(r2.contains(CompRect(x1,   y1,   width1+1, height1)));
    EXPECT_FALSE(r2.contains(CompRect(x1,   y1,   width1,   height1+1)));
    EXPECT_FALSE(r2.contains(CompRect(x1-1, y1,   width1,   height1)));
    EXPECT_FALSE(r2.contains(CompRect(x1,   y1-1, width1,   height1)));
}

TEST(RegionTest, contains_xywh)
{
    CompRegion r1(x1, y1, width1, height1);

    EXPECT_TRUE(r1.contains(x1,   y1,   width1,   height1));
    EXPECT_TRUE(r1.contains(x1,   y1+1, width1,   height1-1));
    EXPECT_TRUE(r1.contains(x1+1, y1,   width1-1, height1));
    EXPECT_TRUE(r1.contains(x1,   y1,   width1-1, height1));
    EXPECT_TRUE(r1.contains(x1,   y1,   width1,   height1-1));

    EXPECT_FALSE(r1.contains(x1+1, y1,   width1,   height1));
    EXPECT_FALSE(r1.contains(x1,   y1+1, width1,   height1));
    EXPECT_FALSE(r1.contains(x1,   y1,   width1+1, height1));
    EXPECT_FALSE(r1.contains(x1,   y1,   width1,   height1+1));
    EXPECT_FALSE(r1.contains(x1-1, y1,   width1,   height1));
    EXPECT_FALSE(r1.contains(x1,   y1-1, width1,   height1));

    CompRegion r2(rect1);

    EXPECT_TRUE(r2.contains(x1,   y1,   width1,   height1));
    EXPECT_TRUE(r2.contains(x1,   y1+1, width1,   height1-1));
    EXPECT_TRUE(r2.contains(x1+1, y1,   width1-1, height1));
    EXPECT_TRUE(r2.contains(x1,   y1,   width1-1, height1));
    EXPECT_TRUE(r2.contains(x1,   y1,   width1,   height1-1));

    EXPECT_FALSE(r2.contains(x1+1, y1,   width1,   height1));
    EXPECT_FALSE(r2.contains(x1,   y1+1, width1,   height1));
    EXPECT_FALSE(r2.contains(x1,   y1,   width1+1, height1));
    EXPECT_FALSE(r2.contains(x1,   y1,   width1,   height1+1));
    EXPECT_FALSE(r2.contains(x1-1, y1,   width1,   height1));
    EXPECT_FALSE(r2.contains(x1,   y1-1, width1,   height1));
}

TEST(RegionTest, equivalent_creates_compare_equal)
{
    CompRegion expect(x1, y1, width1, height1);
    CompRegion actual(rect1);

    EXPECT_EQ(expect.boundingRect (), actual.boundingRect ());
    EXPECT_EQ(expect.rects (), actual.rects ());
    EXPECT_EQ(expect.isEmpty (), actual.isEmpty ());
    EXPECT_EQ(expect.numRects (), actual.numRects ());

    ASSERT_EQ(expect, actual);
}

TEST(RegionTest, different_creates_compare_unequal)
{
    CompRegion r1(x1, y1, width1, height1);
    CompRegion r2(x2, y2, width2, height2);

    EXPECT_NE(r1.boundingRect (), r2.boundingRect ());
    EXPECT_NE(r1.rects (), r2.rects ());
    EXPECT_EQ(r1.isEmpty (), r2.isEmpty ());
    EXPECT_EQ(r1.numRects (), r2.numRects ());

    ASSERT_NE(r1, r2);

    CompRegion r3(rect1);
    CompRegion r4(rect2);

    EXPECT_NE(r3.boundingRect (), r4.boundingRect ());
    EXPECT_NE(r3.rects (), r4.rects ());
    EXPECT_EQ(r3.isEmpty (), r4.isEmpty ());
    EXPECT_EQ(r3.numRects (), r4.numRects ());

    ASSERT_NE(r3, r4);
}

TEST(RegionTest, intersection)
{
    CompRegion r1(x1, y1, width1, height1);
    CompRegion r2(x2, y2, width2, height2);

    EXPECT_TRUE(r1.intersects(r1));
    EXPECT_TRUE(r1.intersects(r2));
    EXPECT_TRUE(r2.intersects(r1));
    EXPECT_TRUE(r2.intersects(r2));

    EXPECT_TRUE(r1.intersects(rect1));
    EXPECT_TRUE(r2.intersects(rect1));
    EXPECT_TRUE(r1.intersects(rect2));
    EXPECT_TRUE(r2.intersects(rect2));

    CompRegion r1_self(r1.intersected(r1));
    EXPECT_EQ(r1, r1_self);

    CompRegion r2_self(r2.intersected(r2));
    EXPECT_EQ(r2, r2_self);

    CompRegion r1_r2(r1.intersected(r2));
    CompRegion r2_r1(r2.intersected(r1));
    EXPECT_EQ(r1_r2, r2_r1);

    EXPECT_TRUE(r1_r2.intersects(rect1));
    EXPECT_TRUE(r1_r2.intersects(rect2));
    EXPECT_TRUE(r2_r1.intersects(rect1));
    EXPECT_TRUE(r2_r1.intersects(rect2));

    CompRegion r1_rect1(r1.intersected(rect1));
    EXPECT_EQ(r1, r1_rect1);

    CompRegion r2_rect2(r2.intersected(rect2));
    EXPECT_EQ(r2, r2_rect2);

    CompRegion r1_rect2(r1.intersected(rect2));
    CompRegion r2_rect1(r2.intersected(rect1));
    EXPECT_EQ(r1_r2, r1_rect2);
    EXPECT_EQ(r1_r2, r2_rect1);

    CompRect expect(x2, y2, width1-(x2-x1), height1-(y2-y1));
    EXPECT_EQ(expect, r1_r2.boundingRect());
}

TEST(RegionTest, subtract)
{
    CompRegion r1(x1, y1, width1, height1);
    CompRegion r2(x2, y2, width2, height2);

    CompRegion r1_r2(r1.subtracted(r2));
    CompRegion r2_r1(r2.subtracted(r1));

    EXPECT_FALSE(r1_r2.intersects(r2_r1));

    EXPECT_NE(r1, r1_r2);
    EXPECT_NE(r2, r2_r1);

    EXPECT_EQ(r1.boundingRect(), r1_r2.boundingRect());
    EXPECT_EQ(r2.boundingRect(), r2_r1.boundingRect());

    EXPECT_TRUE(r1_r2.contains(CompPoint(x1, y1)));
    EXPECT_TRUE(r1_r2.contains(CompPoint(x1+width1-1, y1)));
    EXPECT_TRUE(r1_r2.contains(CompPoint(x1, y1+height1-1)));
    EXPECT_FALSE(r1_r2.contains(CompPoint(x2, y2)));
    EXPECT_FALSE(r1_r2.contains(rect1));
    EXPECT_FALSE(r1_r2.contains(rect2));

    EXPECT_EQ(2, r1_r2.numRects());

    typedef std::vector<CompRect> vcr;
    {
	vcr const& rects = r1_r2.rects();
	EXPECT_EQ(2, rects.size());

	for(vcr::const_iterator i = rects.begin(); i !=rects.end(); ++i)
	{
	    EXPECT_TRUE(r1.contains(*i));
	    EXPECT_FALSE(r2.contains(*i));
	}
    }

    // Much the same with rects instead of regions
    CompRegion r1_rect2(r1.subtracted(rect2));
    CompRegion r2_rect1(r2.subtracted(rect1));

    EXPECT_FALSE(r1_rect2.intersects(r2_rect1));

    EXPECT_NE(r1, r1_rect2);
    EXPECT_NE(r2, r2_rect1);

    EXPECT_EQ(r1.boundingRect(), r1_rect2.boundingRect());
    EXPECT_EQ(r2.boundingRect(), r2_rect1.boundingRect());

    EXPECT_TRUE(r1_rect2.contains(CompPoint(x1, y1)));
    EXPECT_TRUE(r1_rect2.contains(CompPoint(x1+width1-1, y1)));
    EXPECT_TRUE(r1_rect2.contains(CompPoint(x1, y1+height1-1)));
    EXPECT_FALSE(r1_rect2.contains(CompPoint(x2, y2)));
    EXPECT_FALSE(r1_rect2.contains(rect1));
    EXPECT_FALSE(r1_rect2.contains(rect2));

    EXPECT_EQ(2, r1_rect2.numRects());

    typedef std::vector<CompRect> vcr;
    {
	vcr const& rects = r1_rect2.rects();
	EXPECT_EQ(2, rects.size());

	for(vcr::const_iterator i = rects.begin(); i !=rects.end(); ++i)
	{
	    EXPECT_TRUE(r1.contains(*i));
	    EXPECT_FALSE(r2.contains(*i));
	}
    }
}

TEST(RegionTest, translate)
{
    CompRegion r1(CompRegion(x1, y1, width1, height1).subtracted(rect2));

    CompRegion r2(r1.translated(dx, dy));
    EXPECT_NE(r1, r2);

    // Bloody daft to use a "point" as a displacement
    CompRegion r3(r1.translated(CompPoint(dx, dy)));
    EXPECT_NE(r1, r3);
    EXPECT_EQ(r2, r3);

    r2.translate(-dx, -dy);
    EXPECT_EQ(r1, r2);

    // Bloody daft to use a "point" as a displacement
    r3.translate(CompPoint(-dx, -dy));
    EXPECT_EQ(r1, r3);
}

TEST(RegionTest, shrink)
{
    CompRegion r1(CompRegion(x1, y1, width1, height1).subtracted(rect2));

    CompRegion r2(r1.shrinked(dx, dy));
    EXPECT_NE(r1, r2);

    // Bloody daft to use a "point" as a boundary delta
    CompRegion r3(r1.shrinked(CompPoint(dx, dy)));
    EXPECT_NE(r1, r3);
    EXPECT_EQ(r2, r3);

    r2.shrink(-dx, -dy);
    EXPECT_EQ(r1, r2);

    // Bloody daft to use a "point" as a boundary delta
    r3.shrink(CompPoint(-dx, -dy));
    EXPECT_EQ(r1, r3);
}

TEST(RegionTest, unite)
{
    CompRegion r1(x1, y1, width1, height1);
    CompRegion r2(rect2);

    CompRegion r1_self(r1.united(r1));
    EXPECT_EQ(r1, r1_self);

    CompRegion r2_self(r2.united(r2));
    EXPECT_EQ(r2, r2_self);

    CompRegion r1_r2(r1.united(r2));
    CompRegion r2_r1(r2.united(r1));
    EXPECT_EQ(r1_r2, r2_r1);

    EXPECT_TRUE(r1_r2.contains(x1, y1, width1, height1));
    EXPECT_TRUE(r1_r2.contains(rect2));

    EXPECT_TRUE(r1_r2.contains(CompPoint(x1, y1)));
    EXPECT_TRUE(r1_r2.contains(CompPoint(x1+width1-1, y1)));
    EXPECT_TRUE(r1_r2.contains(CompPoint(x1, y1+height1-1)));
    EXPECT_TRUE(r1_r2.contains(CompPoint(x1+width1-1, y1+height1-1)));

    EXPECT_TRUE(r1_r2.contains(CompPoint(x2, y2)));

    EXPECT_FALSE(r1_r2.contains(CompPoint(x1, y2+height2-1)));
    EXPECT_FALSE(r1_r2.contains(CompPoint(x2+width2-1, y1)));
}

TEST(RegionTest, xored)
{
    CompRegion r1(x1, y1, width1, height1);
    CompRegion r2(rect2);

    CompRegion r1_self(r1.xored(r1));
    EXPECT_NE(r1, r1_self);
    EXPECT_TRUE(r1_self.isEmpty());

    CompRegion r2_self(r2.xored(r2));
    EXPECT_NE(r2, r2_self);
    EXPECT_TRUE(r2_self.isEmpty());

    CompRegion r1_r2(r1.xored(r2));
    CompRegion r2_r1(r2.xored(r1));
    EXPECT_EQ(r1_r2, r2_r1);

    EXPECT_FALSE(r1_r2.contains(x1, y1, width1, height1));
    EXPECT_FALSE(r1_r2.contains(rect2));

    EXPECT_TRUE(r1_r2.contains(CompPoint(x1, y1)));
    EXPECT_TRUE(r1_r2.contains(CompPoint(x1+width1-1, y1)));
    EXPECT_TRUE(r1_r2.contains(CompPoint(x1, y1+height1-1)));

    EXPECT_FALSE(r1_r2.contains(CompPoint(x1+width1-1, y1+height1-1)));
    EXPECT_FALSE(r1_r2.contains(CompPoint(x2, y2)));
    EXPECT_FALSE(r1_r2.contains(CompPoint(x1, y2+height2-1)));
    EXPECT_FALSE(r1_r2.contains(CompPoint(x2+width2-1, y1)));

    EXPECT_TRUE(r1_r2.contains(CompPoint(x2, y2+height2-1)));
    EXPECT_TRUE(r1_r2.contains(CompPoint(x2+width2-1, y2)));
    EXPECT_TRUE(r1_r2.contains(CompPoint(x2+width2-1, y2+height2-1)));
}

// The operators should be non-members and syntactic sugar. But...
TEST(RegionTest, operators)
{
    CompRegion r1(rect1);
    CompRegion r2(rect2);

    { 	// operator&() varients
	CompRegion expect(r1.intersected(rect2));

	EXPECT_EQ(expect, r1 & r2);
	EXPECT_EQ(expect, r1 & rect2);
	EXPECT_EQ(expect, CompRegion(r1) &= r2);
	EXPECT_EQ(expect, CompRegion(r1) &= rect2);
    }

    { 	// operator+() varients
	CompRegion expect(r1.united(rect2));

	EXPECT_EQ(expect, r1 + r2);
	EXPECT_EQ(expect, r1 + rect2);
	EXPECT_EQ(expect, CompRegion(r1) += r2);
	EXPECT_EQ(expect, CompRegion(r1) += rect2);

	// operator|() varients
	EXPECT_EQ(expect, r1 | r2);
	EXPECT_EQ(expect, r1 | rect2);
	EXPECT_EQ(expect, CompRegion(r1) |= r2);
	EXPECT_EQ(expect, CompRegion(r1) |= rect2);
    }

    { 	// operator-() varients
	CompRegion expect(r1.subtracted(rect2));

	EXPECT_EQ(expect, r1 - r2);
	EXPECT_EQ(expect, r1 - rect2);
	EXPECT_EQ(expect, CompRegion(r1) -= r2);
	EXPECT_EQ(expect, CompRegion(r1) -= rect2);
    }

    { 	// operator^() varients
	CompRegion expect(r1.xored(rect2));

	EXPECT_EQ(expect, r1 ^ r2);
	EXPECT_EQ(expect, r1 ^ rect2);
	EXPECT_EQ(expect, CompRegion(r1) ^= r2);
	EXPECT_EQ(expect, CompRegion(r1) ^= rect2);
    }
}

TEST(RegionTest, external_refs)
{
    CompRegion r1(rect1);
    CompRegion r2(rect2);
    CompRegionRef rr1(r1.handle());
    CompRegionRef rr2(r2.handle());

    {
	// Verify that the refs don't free the underlying Region. If they
	// do then the following EXPECT_EQ's should crash.
	CompRegionRef tmp1(r1.handle());
	CompRegionRef tmp2(r2.handle());
    }

    EXPECT_EQ(r1, rr1);
    EXPECT_EQ(r1.handle(), rr1.handle());

    EXPECT_EQ(r2, rr2);
    EXPECT_EQ(r2.handle(), rr2.handle());

    EXPECT_EQ(r1 & r2, rr1 & rr2);
    EXPECT_EQ(r1 & r2, rr1 & r2);
    EXPECT_EQ(r1 & rr2, rr1 & r2);

    EXPECT_EQ(r1 | r2, rr1 | rr2);
    EXPECT_EQ(r1 | r2, rr1 | r2);
    EXPECT_EQ(r1 | rr2, rr1 | r2);

    EXPECT_EQ(r1 - r2, rr1 - rr2);
    EXPECT_EQ(r1 - r2, rr1 - r2);
    EXPECT_EQ(r1 - rr2, rr1 - r2);

    EXPECT_EQ(r1 ^ r2, rr1 ^ rr2);
    EXPECT_EQ(r1 ^ r2, rr1 ^ r2);
    EXPECT_EQ(r1 ^ rr2, rr1 ^ r2);

    CompRegion *p = new CompRegion(r1);
    CompRegionRef *rp = new CompRegionRef(p->handle());
    ASSERT_EQ(*rp, r1);
    delete rp;
    ASSERT_EQ(*p, r1);
    delete p;
}
