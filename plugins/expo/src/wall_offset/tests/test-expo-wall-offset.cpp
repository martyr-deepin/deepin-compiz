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
 * Authored by: Sam Spilsbury <sam.spilsbury@canonical.com>
 */
#include <tr1/tuple>
#include <gtest/gtest.h>
#include "wall-offset.h"

using ::testing::WithParamInterface;
using ::testing::ValuesIn;
using ::testing::Combine;
using ::testing::Range;

class ExpoWallOffsetTest :
    public ::testing::Test
{
    protected:

	float offsetInWorldX;
	float offsetInWorldY;
	float worldScaleFactorX;
	float worldScaleFactorY;
};

namespace
{
    const unsigned int nAnimationsBegin = 0;
    const unsigned int nAnimationSteps = 20;

    struct OffsetAnimationParameters
    {
	float offsetInWorldX;
	float offsetInWorldY;
	float worldScaleFactorX;
	float worldScaleFactorY;
    };

    struct OffsetParameters
    {
	float offsetX;
	float offsetY;
	int   vpSizeWidth;
	int   vpSizeHeight;
	int   screenWidth;
	int   screenHeight;
	int   outputWidth;
	int   outputHeight;
	OffsetAnimationParameters animationParameters[20];
    };

    const OffsetParameters testingOffsetParameters[] =
    {
	{
	    0,
	    0,
	    1,
	    1,
	    100,
	    100,
	    100,
	    100,
	    {
		{ 0, 0, 1.0, 1.0 },
		{ 0, 0, 1.0, 1.0 },
		{ 0, 0, 1.0, 1.0 },
		{ 0, 0, 1.0, 1.0 },
		{ 0, 0, 1.0, 1.0 },
		{ 0, 0, 1.0, 1.0 },
		{ 0, 0, 1.0, 1.0 },
		{ 0, 0, 1.0, 1.0 },
		{ 0, 0, 1.0, 1.0 },
		{ 0, 0, 1.0, 1.0 },
		{ 0, 0, 1.0, 1.0 },
		{ 0, 0, 1.0, 1.0 },
		{ 0, 0, 1.0, 1.0 },
		{ 0, 0, 1.0, 1.0 },
		{ 0, 0, 1.0, 1.0 },
		{ 0, 0, 1.0, 1.0 },
		{ 0, 0, 1.0, 1.0 },
		{ 0, 0, 1.0, 1.0 },
		{ 0, 0, 1.0, 1.0 },
		{ 0, 0, 1.0, 1.0 }
	    }
	},
	/* Monitor 1280x800, Screen: 1280x800, Viewport Layout: 2x2, Offset: 32, 24 */
	{
	    32,
	    24,
	    2,
	    2,
	    1280,
	    800,
	    1280,
	    800,
	    {
		{ 0, 0, 1, 1 },
		{ 0.0025000001769512892, 0.0011718750465661287, 0.99874997138977051, 0.99906247854232788 },
		{ 0.0050000003539025784, 0.0023437500931322575, 0.99750000238418579, 0.99812501668930054 },
		{ 0.0075000002980232239, 0.0035156251396983862, 0.9962499737739563, 0.99718749523162842 },
		{ 0.010000000707805157, 0.0046875001862645149, 0.99500000476837158, 0.9962499737739563 },
		{ 0.012500000186264515, 0.005859375, 0.99374997615814209, 0.99531251192092896 },
		{ 0.015000000596046448, 0.0070312502793967724, 0.99250000715255737, 0.99437499046325684 },
		{ 0.017500000074505806, 0.0082031246274709702, 0.99124997854232788, 0.99343752861022949 },
		{ 0.020000001415610313, 0.0093750003725290298, 0.99000000953674316, 0.99250000715255737 },
		{ 0.022499999031424522, 0.01054687425494194, 0.98874998092651367, 0.99156248569488525 },
		{ 0.02500000037252903, 0.01171875, 0.98750001192092896, 0.99062502384185791 },
		{ 0.027500001713633537, 0.01289062574505806, 0.98624998331069946, 0.98968750238418579 },
		{ 0.030000001192092896, 0.014062500558793545, 0.98500001430511475, 0.98874998092651367 },
		{ 0.032499998807907104, 0.015234374441206455, 0.98374998569488525, 0.98781251907348633 },
		{ 0.035000000149011612, 0.01640624925494194, 0.98250001668930054, 0.98687499761581421 },
		{ 0.037500001490116119, 0.017578125, 0.98124998807907104, 0.98593747615814209 },
		{ 0.040000002831220627, 0.01875000074505806, 0.98000001907348633, 0.98500001430511475 },
		{ 0.042500000447034836, 0.019921876490116119, 0.97874999046325684, 0.98406249284744263 },
		{ 0.044999998062849045, 0.021093748509883881, 0.97750002145767212, 0.98312497138977051 },
		{ 0.047499999403953552, 0.02226562425494194, 0.97624999284744263, 0.98218750953674316 }
	    }
	},
	/* Monitor 1280x1024, Screen: 2560x1024, Viewport Layout: 2x2, Offset: 32, 24 */
	{
	    32,
	    24,
	    2,
	    2,
	    2560,
	    1024,
	    1280,
	    1024,
	    {
		{ 0, 0, 1, 1 },
		{ 0.0050000003539025784, 0.001500000013038516, 0.99874997138977051, 0.99906247854232788 },
		{ 0.010000000707805157, 0.0030000000260770321, 0.99750000238418579, 0.99812501668930054 },
		{ 0.015000000596046448, 0.0045000002719461918, 0.9962499737739563, 0.99718749523162842 },
		{ 0.020000001415610313, 0.0060000000521540642, 0.99500000476837158, 0.9962499737739563 },
		{ 0.02500000037252903, 0.0074999998323619366, 0.99374997615814209, 0.99531251192092896 },
		{ 0.030000001192092896, 0.0090000005438923836, 0.99250000715255737, 0.99437499046325684 },
		{ 0.035000000149011612, 0.010499999858438969, 0.99124997854232788, 0.99343752861022949 },
		{ 0.040000002831220627, 0.012000000104308128, 0.99000000953674316, 0.99250000715255737 },
		{ 0.044999998062849045, 0.013499999418854713, 0.98874998092651367, 0.99156248569488525 },
		{ 0.05000000074505806, 0.014999999664723873, 0.98750001192092896, 0.99062502384185791 },
		{ 0.055000003427267075, 0.016499999910593033, 0.98624998331069946, 0.98968750238418579 },
		{ 0.060000002384185791, 0.018000001087784767, 0.98500001430511475, 0.98874998092651367 },
		{ 0.064999997615814209, 0.019499998539686203, 0.98374998569488525, 0.98781251907348633 },
		{ 0.070000000298023224, 0.020999999716877937, 0.98250001668930054, 0.98687499761581421 },
		{ 0.075000002980232239, 0.022499999031424522, 0.98124998807907104, 0.98593747615814209 },
		{ 0.080000005662441254, 0.024000000208616257, 0.98000001907348633, 0.98500001430511475 },
		{ 0.085000000894069672, 0.025499999523162842, 0.97874999046325684, 0.98406249284744263 },
		{ 0.08999999612569809, 0.026999998837709427, 0.97750002145767212, 0.98312497138977051 },
		{ 0.094999998807907104, 0.028499998152256012, 0.97624999284744263, 0.98218750953674316 }
	    }
	},
    };

    typedef std::tr1::tuple <OffsetParameters, unsigned int> AnimParam;
}

class ExpoWallOffsetTestAnimations :
    public ExpoWallOffsetTest,
    public ::testing::WithParamInterface <AnimParam>
{
    public:

	void
	RecordProperty (const char *name, float value)
	{
	    ::testing::Message message;
	    message << value;
	    Test::RecordProperty (name, message.GetString ().c_str ());
	}
};

TEST_P (ExpoWallOffsetTestAnimations, TestAnimationValues)
{
    const OffsetParameters &offset (std::tr1::get <0> (GetParam ()));
    const unsigned int     &index (std::tr1::get <1> (GetParam ()));

    RecordProperty ("outputWidth", offset.outputWidth);
    RecordProperty ("outputHeight", offset.outputHeight);
    RecordProperty ("screenWidth", offset.screenWidth);
    RecordProperty ("screenHeight", offset.screenHeight);
    RecordProperty ("offsetX", offset.offsetX);
    RecordProperty ("offsetY", offset.offsetY);

    RecordProperty ("expected.offsetInWorldX", offset.animationParameters[index].offsetInWorldX);
    RecordProperty ("expected.offsetInWorldY", offset.animationParameters[index].offsetInWorldY);
    RecordProperty ("expected.worldScaleFactorX", offset.animationParameters[index].worldScaleFactorX);
    RecordProperty ("expected.worldScaleFactorY", offset.animationParameters[index].worldScaleFactorY);

    compiz::expo::calculateWallOffset (CompRect (0,
						 0,
						 offset.outputWidth,
						 offset.outputWidth),
				       CompPoint (offset.offsetX,
						  offset.offsetY),
				       CompPoint (offset.vpSizeWidth,
						  offset.vpSizeHeight),
				       CompSize (offset.screenWidth,
						 offset.screenHeight),
				       offsetInWorldX,
				       offsetInWorldY,
				       worldScaleFactorX,
				       worldScaleFactorY,
				       index / static_cast <float> (nAnimationSteps));

    RecordProperty ("offsetInWorldX", offsetInWorldX);
    RecordProperty ("offsetInWorldY", offsetInWorldY);
    RecordProperty ("worldScaleFactorX", worldScaleFactorX);
    RecordProperty ("worldScaleFactorY", worldScaleFactorY);

    EXPECT_EQ (offsetInWorldX, offset.animationParameters[index].offsetInWorldX);
    EXPECT_EQ (offsetInWorldY, offset.animationParameters[index].offsetInWorldY);
    EXPECT_EQ (worldScaleFactorX, offset.animationParameters[index].worldScaleFactorX);
    EXPECT_EQ (worldScaleFactorY, offset.animationParameters[index].worldScaleFactorY);
}

TEST_F (ExpoWallOffsetTest, TestNoOffsetIfOutputIsNotOrigin)
{
    compiz::expo::calculateWallOffset (CompRect (1,
						 1,
						 100,
						 100),
				       CompPoint (100,
						  100),
				       CompPoint (1,
						  1),
				       CompSize (100,
						 100),
				       offsetInWorldX,
				       offsetInWorldY,
				       worldScaleFactorX,
				       worldScaleFactorY,
				       1.0);

    EXPECT_EQ (offsetInWorldX, 0.0f);
    EXPECT_EQ (offsetInWorldY, 0.0f);
    EXPECT_EQ (worldScaleFactorX, 1.0f);
    EXPECT_EQ (worldScaleFactorY, 1.0f);
}

INSTANTIATE_TEST_CASE_P (ExpoAnimationOffsetTest,
			 ExpoWallOffsetTestAnimations,
			 Combine (ValuesIn (testingOffsetParameters),
				  Range (nAnimationsBegin,
					 nAnimationSteps)));
