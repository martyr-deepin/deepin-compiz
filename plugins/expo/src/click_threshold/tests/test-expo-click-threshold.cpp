/*
 * Copyright © 2012 Canonical Ltd.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Canonical Ltd. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Canonical Ltd. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * CANONICAL, LTD. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL CANONICAL, LTD. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authored by: Renato Araujo Oliveira Filho <renato@canonical.com>
 */


#include <gtest/gtest.h>

#include "click-threshold.h"

class ExpoClickThresholdTest :
    public ::testing::Test
{
};

TEST(ExpoClickThresholdTest, TestNotMove)
{
    EXPECT_TRUE(compiz::expo::clickMovementInThreshold (10, 10, 10, 10));
}

TEST(ExpoClickThresholdTest, TestMoveNearLeft)
{
    EXPECT_TRUE(compiz::expo::clickMovementInThreshold (10, 10, 8, 8));
}

TEST(ExpoClickThresholdTest, TestMoveNearRight)
{
    EXPECT_TRUE(compiz::expo::clickMovementInThreshold (10, 10, 13, 13));
}

TEST(ExpoClickThresholdTest, TestMoveFarLeft)
{
    EXPECT_FALSE(compiz::expo::clickMovementInThreshold (10, 10, 1, 1));
}

TEST(ExpoClickThresholdTest, TestMoveFarRight)
{
    EXPECT_FALSE(compiz::expo::clickMovementInThreshold (10, 10, 30, 30));
}

TEST(ExpoClickThresholdTest, TestMoveNearX)
{
    EXPECT_TRUE(compiz::expo::clickMovementInThreshold (10, 10, 13, 10));
}

TEST(ExpoClickThresholdTest, TestMoveNearY)
{
    EXPECT_TRUE(compiz::expo::clickMovementInThreshold (10, 10, 10, 13));
}

TEST(ExpoClickThresholdTest, TestMoveFarX)
{
    EXPECT_FALSE(compiz::expo::clickMovementInThreshold (10, 10, 30, 10));
}

TEST(ExpoClickThresholdTest, TestMoveFarY)
{
    EXPECT_FALSE(compiz::expo::clickMovementInThreshold (10, 10, 10, 30));
}
