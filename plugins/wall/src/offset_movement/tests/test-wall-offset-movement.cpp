#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "offset-movement.h"

class WallOffsetMovementTest :
    public ::testing::Test
{
};

TEST(WallOffsetMovementTest, TestOffsetRight)
{
    CompRect sbr (750, 0, 500, 500);
    CompRegion sr (0, 0, 1000, 1000);

    CompPoint offset = compiz::wall::movementWindowOnScreen (sbr, sr);

    EXPECT_EQ (offset, CompPoint (-250, 0));
}

TEST(WallOffsetMovementTest, TestOffsetLeft)
{
    CompRect sbr (-250, 0, 500, 500);
    CompRegion sr (0, 0, 1000, 1000);

    CompPoint offset = compiz::wall::movementWindowOnScreen (sbr, sr);

    EXPECT_EQ (offset, CompPoint (250, 0));
}

TEST(WallOffsetMovementTest, TestOffsetTop)
{
    CompRect sbr (0, -250, 500, 500);
    CompRegion sr (0, 0, 1000, 1000);

    CompPoint offset = compiz::wall::movementWindowOnScreen (sbr, sr);

    EXPECT_EQ (offset, CompPoint (0, 250));
}

TEST(WallOffsetMovementTest, TestOffsetBottom)
{
    CompRect sbr (0, 750, 500, 500);
    CompRegion sr (0, 0, 1000, 1000);

    CompPoint offset = compiz::wall::movementWindowOnScreen (sbr, sr);

    EXPECT_EQ (offset, CompPoint (0, -250));
}

TEST(WallOffsetMovementTest, TestOffsetRightMMSlice)
{
    CompRect sbr (750, 0, 500, 500);
    CompRegion sr (0, 0, 1000, 1000);

    sr -= CompRegion (400, 0, 200, 0);

    CompPoint offset = compiz::wall::movementWindowOnScreen (sbr, sr);

    EXPECT_EQ (offset, CompPoint (-250, 0));
}

TEST(WallOffsetMovementTest, TestOffsetLeftMMSlice)
{
    CompRect sbr (-250, 0, 500, 500);
    CompRegion sr (0, 0, 1000, 1000);

    sr -= CompRegion (400, 0, 200, 0);

    CompPoint offset = compiz::wall::movementWindowOnScreen (sbr, sr);

    EXPECT_EQ (offset, CompPoint (250, 0));
}

TEST(WallOffsetMovementTest, TestOffsetTopMMSlice)
{
    CompRect sbr (0, -250, 500, 500);
    CompRegion sr (0, 0, 1000, 1000);

    sr -= CompRegion (400, 0, 200, 0);

    CompPoint offset = compiz::wall::movementWindowOnScreen (sbr, sr);

    EXPECT_EQ (offset, CompPoint (0, 250));
}

TEST(WallOffsetMovementTest, TestOffsetBottomMMSlice)
{
    CompRect sbr (0, 750, 500, 500);
    CompRegion sr (0, 0, 1000, 1000);

    sr -= CompRegion (400, 0, 200, 0);

    CompPoint offset = compiz::wall::movementWindowOnScreen (sbr, sr);

    EXPECT_EQ (offset, CompPoint (0, -250));
}
