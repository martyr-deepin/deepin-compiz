 /*
 * Copyright © 2005 Novell, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Novell, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Novell, Inc. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * NOVELL, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NOVELL, INC. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <davidr@novell.com>
 *         Mirco Müller <macslow@bangang.de> (Skydome support)
 */

#include <string.h>
#include <math.h>

#include <X11/Xatom.h>
#include <X11/Xproto.h>

#include <privates.h>

class CubePluginVTable :
    public CompPlugin::VTableForScreenAndWindow<CubeScreen, PrivateCubeWindow>
{
    public:

	bool init ();
	void fini ();
};

COMPIZ_PLUGIN_20090315 (cube, CubePluginVTable)

void
CubeScreenInterface::cubeGetRotation (float &x, float &v, float &progress)
    WRAPABLE_DEF (cubeGetRotation, x, v, progress);

void
CubeScreenInterface::cubeClearTargetOutput (float xRotate, float vRotate)
    WRAPABLE_DEF (cubeClearTargetOutput, xRotate, vRotate);

void
CubeScreenInterface::cubePaintTop (const GLScreenPaintAttrib &sAttrib,
		                   const GLMatrix            &transform,
		                   CompOutput                *output,
				   int                       size,
				   const GLVector            &normal)
    WRAPABLE_DEF (cubePaintTop, sAttrib, transform, output, size, normal)
    
void
CubeScreenInterface::cubePaintBottom (const GLScreenPaintAttrib &sAttrib,
				      const GLMatrix            &transform,
				      CompOutput                *output,
				      int                       size,
				      const GLVector            &normal)
    WRAPABLE_DEF (cubePaintBottom, sAttrib, transform, output, size, normal)

void
CubeScreenInterface::cubePaintInside (const GLScreenPaintAttrib &sAttrib,
				      const GLMatrix            &transform,
				      CompOutput                *output,
				      int                       size,
				      const GLVector            &normal)
    WRAPABLE_DEF (cubePaintInside, sAttrib, transform, output, size, normal)

bool
CubeScreenInterface::cubeCheckOrientation (const GLScreenPaintAttrib &sAttrib,
					   const GLMatrix            &transform,
					   CompOutput                *output,
					   std::vector<GLVector>     &points)
    WRAPABLE_DEF (cubeCheckOrientation, sAttrib, transform, output, points)

void
CubeScreenInterface::cubePaintViewport (const GLScreenPaintAttrib &sAttrib,
					const GLMatrix            &transform,
					const CompRegion          &region,
					CompOutput                *output,
					unsigned int              mask)
    WRAPABLE_DEF (cubePaintViewport, sAttrib, transform, region, output, mask)

bool
CubeScreenInterface::cubeShouldPaintViewport (const GLScreenPaintAttrib &sAttrib,
					      const GLMatrix            &transform,
					      CompOutput                *output,
					      PaintOrder                order)
    WRAPABLE_DEF (cubeShouldPaintViewport, sAttrib, transform, output, order)
    
bool
CubeScreenInterface::cubeShouldPaintAllViewports ()
    WRAPABLE_DEF (cubeShouldPaintAllViewports);

int 
CubeScreen::invert () const
{
    return priv->mInvert;
}

unsigned short* 
CubeScreen::topColor () const
{
    return priv->optionGetTopColor ();
}

unsigned short*  
CubeScreen::bottomColor () const
{
    return priv->optionGetBottomColor ();
}

bool 
CubeScreen::unfolded () const
{
    return priv->mUnfolded;
}

CubeScreen::RotationState 
CubeScreen::rotationState () const
{
    return priv->mRotationState;
}

void 
CubeScreen::rotationState (CubeScreen::RotationState state)
{
    priv->mRotationState = state;
}

int 
CubeScreen::xRotations () const
{
    return priv->mXRotations;
}

int 
CubeScreen::nOutput () const
{
    return priv->mNOutput;
}

float
CubeScreen::outputXScale () const
{
    return priv->mOutputXScale;
}

float
CubeScreen::outputYScale () const
{
    return priv->mOutputYScale;
}

float
CubeScreen::outputXOffset () const
{
    return priv->mOutputXOffset;
}

float
CubeScreen::outputYOffset () const
{
    return priv->mOutputYOffset;
}

float
CubeScreen::distance () const
{
    return priv->mDistance;
}

float
CubeScreen::desktopOpacity () const
{
    return priv->mDesktopOpacity;
}

CubeScreen::MultioutputMode 
CubeScreen::multioutputMode () const
{
    switch (priv->optionGetMultioutputMode ())
    {
	case CubeOptions::MultioutputModeOneBigCube:
	    return OneBigCube;
	case CubeOptions::MultioutputModeMultipleCubes:
	    return MultipleCubes;
	default:
	    break;
    }

    return Automatic;
}

int 
CubeScreen::sourceOutput () const
{
    return priv->mSrcOutput;
}

PaintOrder 
CubeScreen::paintOrder () const
{
    return priv->mPaintOrder;
}

bool
CubeScreen::cubeShouldPaintAllViewports ()
{
    WRAPABLE_HND_FUNCTN_RETURN (bool, cubeShouldPaintAllViewports);
    
    return priv->mPaintAllViewports;
}

void
CubeScreen::repaintCaps ()
{
    memset (priv->mCapsPainted, 0, sizeof (Bool) * screen->outputDevs ().size ());
}

bool
PrivateCubeScreen::updateGeometry (int sides, int invert)
{
    GLfloat radius, distance;
    GLfloat *v;
    int     i, n;

    sides *= mNOutput;

    distance = 0.5f / tanf (M_PI / sides);
    radius   = 0.5f / sinf (M_PI / sides);

    n = (sides + 2) * 2;

    if (mNVertices != n)
    {
	v = (GLfloat *) realloc (mVertices, sizeof (GLfloat) * n * 3);
	if (!v)
	    return false;

	mNVertices = n;
	mVertices  = v;
    }
    else
	v = mVertices;

    *v++ = 0.0f;
    *v++ = 0.5 * invert;
    *v++ = 0.0f;

    for (i = 0; i <= sides; i++)
    {
	*v++ = radius * sinf (i * 2 * M_PI / sides + M_PI / sides);
	*v++ = 0.5 * invert;
	*v++ = radius * cosf (i * 2 * M_PI / sides + M_PI / sides);
    }

    *v++ = 0.0f;
    *v++ = -0.5 * invert;
    *v++ = 0.0f;

    for (i = sides; i >= 0; i--)
    {
	*v++ = radius * sinf (i * 2 * M_PI / sides + M_PI / sides);
	*v++ = -0.5 * invert;
	*v++ = radius * cosf (i * 2 * M_PI / sides + M_PI / sides);
    }

    mInvert   = invert;
    mDistance = distance;

    return true;
}

void
PrivateCubeScreen::updateOutputs ()
{
    CompOutput *pBox0, *pBox1;
    unsigned int i, j;
    int    k, x;

    k = 0;

    mFullscreenOutput = true;

    for (i = 0; i < screen->outputDevs ().size (); i++)
    {
	mOutputMask[i] = -1;

	/* dimensions must match first output */
	if (screen->outputDevs ()[i].width ()  != screen->outputDevs ()[0].width () ||
	    screen->outputDevs ()[i].height () != screen->outputDevs ()[0].height ())
	    continue;

	pBox0 = &screen->outputDevs ()[0];
	pBox1 = &screen->outputDevs ()[i];

	/* top and bottom line must match first output */
	if (pBox0->y1 () != pBox1->y1 () || pBox0->y2 () != pBox1->y2 ())
	    continue;

	k++;

	for (j = 0; j < screen->outputDevs ().size (); j++)
	{
	    pBox0 = &screen->outputDevs ()[j];

	    /* must not intersect other output region */
	    if (i != j && pBox0->x2 () > pBox1->x1 () && pBox0->x1 () < pBox1->x2 ())
	    {
		k--;
		break;
	    }
	}
    }

    if (optionGetMultioutputMode () == CubeOptions::MultioutputModeOneBigCube)
    {
	mFullscreenOutput = false;
	mNOutput = 1;
	return;
    }

    if (optionGetMultioutputMode () == CubeOptions::MultioutputModeMultipleCubes)
    {
	mFullscreenOutput = true;
	mNOutput = 1;
	return;
    }

    if ((unsigned int) k != screen->outputDevs ().size ())
    {
	mFullscreenOutput = false;
	mNOutput = 1;
	return;
    }

    /* add output indices from left to right */
    j = 0;
    for (;;)
    {
	x = MAXSHORT;
	k = -1;

	for (i = 0; i < screen->outputDevs ().size (); i++)
	{
	    if (mOutputMask[i] != -1)
		continue;

	    if (screen->outputDevs ()[i].x1 () < x)
	    {
		x = screen->outputDevs ()[i].x1 ();
		k = i;
	    }
	}

	if (k < 0)
	    break;

	mOutputMask[k] = j;
	mOutput[j]     = k;

	j++;
    }

    mNOutput = j;

    if (mNOutput == 1)
    {
	if (screen->outputDevs ()[0].width ()  != screen->width () ||
	    screen->outputDevs ()[0].height () != screen->height ())
	    mFullscreenOutput = true;
    }
}

void
PrivateCubeScreen::updateSkydomeTexture ()
{
    mSky.clear ();

    if (!optionGetSkydome ())
	return;

    CompString imgName = optionGetSkydomeImage ();
    CompString pname = "cube";

    if (optionGetSkydomeImage ().empty () ||
	(mSky = GLTexture::readImageToTexture (imgName, pname, mSkySize)).empty ())
    {
	GLfloat aaafTextureData[128][128][3];
	GLfloat fRStart = (GLfloat) optionGetSkydomeGradientStartColorRed () / 0xffff;
	GLfloat fGStart = (GLfloat) optionGetSkydomeGradientStartColorGreen () / 0xffff;
	GLfloat fBStart = (GLfloat) optionGetSkydomeGradientStartColorBlue () / 0xffff;
	GLfloat fREnd = (GLfloat) optionGetSkydomeGradientEndColorRed () / 0xffff;
	GLfloat fGEnd = (GLfloat) optionGetSkydomeGradientEndColorGreen () / 0xffff;
	GLfloat fBEnd = (GLfloat) optionGetSkydomeGradientEndColorBlue () / 0xffff;
	GLfloat fRStep = (fREnd - fRStart) / 128.0f;
	GLfloat fGStep = (fGEnd - fGStart) / 128.0f;
	GLfloat fBStep = (fBStart - fBEnd) / 128.0f;
	GLfloat fR = fRStart;
	GLfloat fG = fGStart;
	GLfloat fB = fBStart;

	int	iX, iY;

	for (iX = 127; iX >= 0; iX--)
	{
	    fR += fRStep;
	    fG += fGStep;
	    fB -= fBStep;

	    for (iY = 0; iY < 128; iY++)
	    {
		aaafTextureData[iX][iY][0] = fR;
		aaafTextureData[iX][iY][1] = fG;
		aaafTextureData[iX][iY][2] = fB;
	    }
	}

	mSkySize = CompSize (128, 128);

	mSky = GLTexture::imageDataToTexture ((char *) aaafTextureData,
					      mSkySize, GL_RGB, GL_FLOAT);

	mSky[0]->setFilter (GL_LINEAR);
	mSky[0]->setWrap (GL_CLAMP_TO_EDGE);
    }
}

#ifndef USE_GLES
static bool
fillCircleTable (GLfloat   **ppSint,
		 GLfloat   **ppCost,
		 const int n)
{
    const GLfloat angle = 2 * M_PI / (GLfloat) ((n == 0) ? 1 : n);
    const int	  size = abs (n);
    int		  i;

    *ppSint = (GLfloat *) calloc (sizeof (GLfloat), size + 1);
    *ppCost = (GLfloat *) calloc (sizeof (GLfloat), size + 1);

    if (!(*ppSint) || !(*ppCost))
    {
	free (*ppSint);
	free (*ppCost);

	return false;
    }

    (*ppSint)[0] = 0.0;
    (*ppCost)[0] = 1.0;

    for (i = 1; i < size; i++)
    {
	(*ppSint)[i] = sin (angle * i);
	(*ppCost)[i] = cos (angle * i);
    }

    (*ppSint)[size] = (*ppSint)[0];
    (*ppCost)[size] = (*ppCost)[0];

    return true;
}
#endif

void
PrivateCubeScreen::updateSkydomeList (GLfloat fRadius)
{
#ifndef USE_GLES
    GLint   iSlices = 128;
    GLint   iStacks = 64;
    GLfloat afTexCoordX[4];
    GLfloat afTexCoordY[4];
    GLfloat *sint1;
    GLfloat *cost1;
    GLfloat *sint2;
    GLfloat *cost2;
    GLfloat r;
    GLfloat x;
    GLfloat y;
    GLfloat z;
    int	    i;
    int	    j;
    int	    iStacksStart;
    int	    iStacksEnd;
    int	    iSlicesStart;
    int	    iSlicesEnd;
    GLfloat fStepX;
    GLfloat fStepY;

    if (optionGetSkydomeAnimated ())
    {
	iStacksStart = 11; /* min.   0 */
	iStacksEnd = 53;   /* max.  64 */
	iSlicesStart = 0;  /* min.   0 */
	iSlicesEnd = 128;  /* max. 128 */
    }
    else
    {
	iStacksStart = 21; /* min.   0 */
	iStacksEnd = 43;   /* max.  64 */
	iSlicesStart = 21; /* min.   0 */
	iSlicesEnd = 44;   /* max. 128 */
    }

    fStepX = 1.0 / (GLfloat) (iSlicesEnd - iSlicesStart);
    fStepY = 1.0 / (GLfloat) (iStacksEnd - iStacksStart);

    if (!mSky.size ())
	return;

    if (!fillCircleTable (&sint1, &cost1, -iSlices))
	return;

    if (!fillCircleTable (&sint2, &cost2, iStacks * 2))
    {
	free (sint1);
	free (cost1);
	return;
    }

    afTexCoordX[0] = 1.0f;
    afTexCoordY[0] = 1.0f - fStepY;
    afTexCoordX[1] = 1.0f - fStepX;
    afTexCoordY[1] = 1.0f - fStepY;
    afTexCoordX[2] = 1.0f - fStepX;
    afTexCoordY[2] = 1.0f;
    afTexCoordX[3] = 1.0f;
    afTexCoordY[3] = 1.0f;


    if (!mSkyListId)
	mSkyListId = glGenLists (1);

    glNewList (mSkyListId, GL_COMPILE);

    mSky[0]->enable (GLTexture::Good);

    glBegin (GL_QUADS);

    for (i = iStacksStart; i < iStacksEnd; i++)
    {
	afTexCoordX[0] = 1.0f;
	afTexCoordX[1] = 1.0f - fStepX;
	afTexCoordX[2] = 1.0f - fStepX;
	afTexCoordX[3] = 1.0f;

	for (j = iSlicesStart; j < iSlicesEnd; j++)
	{
	    /* bottom-right */
	    z = cost2[i];
	    r = sint2[i];
	    x = cost1[j];
	    y = sint1[j];

	    glTexCoord2f (
		COMP_TEX_COORD_X (mSky[0]->matrix (), afTexCoordX[3] * mSkySize.width ()),
		COMP_TEX_COORD_Y (mSky[0]->matrix (), afTexCoordY[3] * mSkySize.height ()));
	    glVertex3f (x * r * fRadius, y * r * fRadius, z * fRadius);

	    /* top-right */
	    z = cost2[i + 1];
	    r = sint2[i + 1];
	    x = cost1[j];
	    y = sint1[j];

	    glTexCoord2f (
                COMP_TEX_COORD_X (mSky[0]->matrix (), afTexCoordX[0] * mSkySize.width ()),
		COMP_TEX_COORD_Y (mSky[0]->matrix (), afTexCoordY[0] * mSkySize.height ()));
	    glVertex3f (x * r * fRadius, y * r * fRadius, z * fRadius);

	    /* top-left */
	    z = cost2[i + 1];
	    r = sint2[i + 1];
	    x = cost1[j + 1];
	    y = sint1[j + 1];

	    glTexCoord2f (
                COMP_TEX_COORD_X (mSky[0]->matrix (), afTexCoordX[1] * mSkySize.width ()),
		COMP_TEX_COORD_Y (mSky[0]->matrix (), afTexCoordY[1] * mSkySize.height ()));
	    glVertex3f (x * r * fRadius, y * r * fRadius, z * fRadius);

	    /* bottom-left */
	    z = cost2[i];
	    r = sint2[i];
	    x = cost1[j + 1];
	    y = sint1[j + 1];

	    glTexCoord2f (
                COMP_TEX_COORD_X (mSky[0]->matrix (), afTexCoordX[2] * mSkySize.width ()),
		COMP_TEX_COORD_Y (mSky[0]->matrix (), afTexCoordY[2] * mSkySize.height ()));
	    glVertex3f (x * r * fRadius, y * r * fRadius, z * fRadius);

	    afTexCoordX[0] -= fStepX;
	    afTexCoordX[1] -= fStepX;
	    afTexCoordX[2] -= fStepX;
	    afTexCoordX[3] -= fStepX;
	}

	afTexCoordY[0] -= fStepY;
	afTexCoordY[1] -= fStepY;
	afTexCoordY[2] -= fStepY;
	afTexCoordY[3] -= fStepY;
    }

    glEnd ();

    mSky[0]->disable ();

    glEndList ();

    free (sint1);
    free (cost1);
    free (sint2);
    free (cost2);

#endif
}

bool
PrivateCubeScreen::setOption (const CompString &name, CompOption::Value &value)
{

    unsigned int index;

    bool rv = CubeOptions::setOption (name, value);

    if (!rv || !CompOption::findOption (getOptions (), name, &index))
        return false;

    switch (index) {
	case CubeOptions::In:
	    rv = updateGeometry (screen->vpSize ().width (), value.b () ? -1 : 1);
	    break;
	case CubeOptions::Skydome:
	case CubeOptions::SkydomeImage:
	case CubeOptions::SkydomeAnimated:
	case CubeOptions::SkydomeGradientStartColor:
	case CubeOptions::SkydomeGradientEndColor:
	    updateSkydomeTexture ();
	    updateSkydomeList (1.0f);
	    cScreen->damageScreen ();
	    break;
	case CubeOptions::MultioutputMode:
	    updateOutputs ();
	    updateGeometry (screen->vpSize ().width (), mInvert);
	    cScreen->damageScreen ();
	    break;
	default:
	    break;
    }

    return rv;
}

bool
PrivateCubeScreen::adjustVelocity ()
{
    float unfold, adjust, amount;

    if (mUnfolded)
	unfold = 1.0f - mUnfold;
    else
	unfold = 0.0f - mUnfold;

    adjust = unfold * 0.02f * optionGetAcceleration ();
    amount = fabs (unfold);
    if (amount < 1.0f)
	amount = 1.0f;
    else if (amount > 3.0f)
	amount = 3.0f;

    mUnfoldVelocity = (amount * mUnfoldVelocity + adjust) /
	(amount + 2.0f);

    return (fabs (unfold) < 0.002f && fabs (mUnfoldVelocity) < 0.01f);
}

void
PrivateCubeScreen::preparePaint (int msSinceLastPaint)
{
    int   opt;
    float x, progress;
    unsigned short *topColor, *bottomColor;

    if (mGrabIndex)
    {
	int   steps;
	float amount, chunk;

	amount = msSinceLastPaint * 0.2f *
	    optionGetSpeed ();
	steps  = amount / (0.5f * optionGetTimestep ());
	if (!steps) steps = 1;
	chunk  = amount / (float) steps;

	while (steps--)
	{
	    mUnfold += mUnfoldVelocity * chunk;
	    if (mUnfold > 1.0f)
		mUnfold = 1.0f;

	    if (adjustVelocity ())
	    {
		if (mUnfold < 0.5f)
		{
		    if (mGrabIndex)
		    {
			screen->removeGrab (mGrabIndex, NULL);
			mGrabIndex = 0;
		    }

		    mUnfold = 0.0f;
		}
		break;
	    }
	}
    }

    memset (mCleared, 0, sizeof (Bool) * screen->outputDevs ().size ());
    memset (mCapsPainted, 0, sizeof (Bool) * screen->outputDevs ().size ());

    /* Transparency handling */
    if (mRotationState == CubeScreen::RotationManual ||
	(mRotationState == CubeScreen::RotationChange &&
	 !optionGetTransparentManualOnly ()))
    {
	opt = mLastOpacityIndex = CubeOptions::ActiveOpacity;
    }
    else if (mRotationState == CubeScreen::RotationChange)
    {
	opt = mLastOpacityIndex = CubeOptions::InactiveOpacity;
    }
    else
    {
	opt = CubeOptions::InactiveOpacity;
    }

    mToOpacity = (mOptions[opt].value ().f () / 100.0f) * OPAQUE;

    cubeScreen->cubeGetRotation (x, x, progress);

    if (mDesktopOpacity != mToOpacity ||
	(progress > 0.0 && progress < 1.0))
    {
	mDesktopOpacity = 
	    (optionGetInactiveOpacity () - 
	    ((optionGetInactiveOpacity () -
	     mOptions[mLastOpacityIndex].value ().f ()) * progress))
	    / 100.0f * OPAQUE;

    }

    topColor	= optionGetTopColor ();
    bottomColor	= optionGetBottomColor ();

    mPaintAllViewports = (mDesktopOpacity != OPAQUE ||
			  topColor[3] != OPAQUE ||
			  bottomColor[3] != OPAQUE);
 
    cScreen->preparePaint (msSinceLastPaint);
}

void
PrivateCubeScreen::paint (CompOutput::ptrList &outputs, unsigned int mask)
{
    float x, progress;

    cubeScreen->cubeGetRotation (x, x, progress);

    if (optionGetMultioutputMode () == MultioutputModeOneBigCube && 
	screen->outputDevs ().size () &&
        (progress > 0.0f || mDesktopOpacity != OPAQUE))
    {
	outputs.clear ();
	outputs.push_back (&screen->fullscreenOutput ());
    }

    cScreen->paint (outputs, mask);
}

bool
PrivateCubeScreen::glPaintOutput (const GLScreenPaintAttrib &sAttrib,
				  const GLMatrix            &transform,
				  const CompRegion          &region,
				  CompOutput                *output,
				  unsigned int              mask)
{
    if (mGrabIndex || mDesktopOpacity != OPAQUE)
    {
	mask &= ~PAINT_SCREEN_REGION_MASK;
	mask |= PAINT_SCREEN_TRANSFORMED_MASK;
    }

    mSrcOutput = ((unsigned int) output->id () != (unsigned int) ~0) ?
    							      output->id () : 0;
    /* Always use BTF painting on non-transformed screen */
    mPaintOrder = BTF;

    return gScreen->glPaintOutput (sAttrib, transform, region, output, mask);
}

void
PrivateCubeScreen::donePaint ()
{
    if (mGrabIndex || mDesktopOpacity != mToOpacity)
	cScreen->damageScreen ();

    cScreen->donePaint ();
}

bool
CubeScreen::cubeCheckOrientation (const GLScreenPaintAttrib &sAttrib,
				  const GLMatrix            &transform,
				  CompOutput                *output,
				  std::vector<GLVector>     &points)
{
    WRAPABLE_HND_FUNCTN_RETURN (bool, cubeCheckOrientation, sAttrib, transform, output, points)
    GLMatrix sTransform = transform;
    GLMatrix mvp, pm (priv->gScreen->projectionMatrix ()->getMatrix ());
    GLVector pntA, pntB, pntC;
    GLVector vecA, vecB, ortho;
    bool     rv = false;

    priv->gScreen->glApplyTransform (sAttrib, output, &sTransform);
    sTransform.translate (priv->mOutputXOffset, -priv->mOutputYOffset, 0.0f);
    sTransform.scale (priv->mOutputXScale, priv->mOutputYScale, 1.0f);

    mvp = pm * sTransform;

    pntA = mvp * points[0];

    if (pntA[3] < 0.0f)
	rv = !rv;

    pntA.homogenize ();

    pntB = mvp * points[1];

    if (pntB[3] < 0.0f)
	rv = !rv;

    pntB.homogenize ();

    pntC = mvp * points[2];
    pntC.homogenize ();

    vecA = pntC - pntA;
    vecB = pntC - pntB;

    ortho = vecA ^ vecB;

    if (ortho[2] > 0.0f)
	rv = !rv;

    return rv;
}

bool
CubeScreen::cubeShouldPaintViewport (const GLScreenPaintAttrib &sAttrib,
				     const GLMatrix            &transform,
				     CompOutput                *output,
				     PaintOrder                order)
{
    WRAPABLE_HND_FUNCTN_RETURN (bool, cubeShouldPaintViewport, sAttrib, transform, output, order)

    bool  ftb;
    float pointZ;

    pointZ = priv->mInvert * priv->mDistance;
    std::vector<GLVector> vPoints;
    vPoints.push_back (GLVector (-0.5, 0.0, pointZ, 1.0));
    vPoints.push_back (GLVector (0.0, 0.5, pointZ, 1.0));
    vPoints.push_back (GLVector (0.0, 0.0, pointZ, 1.0));

    ftb = cubeCheckOrientation (sAttrib, transform, output, vPoints);

    return (order == FTB && ftb) || (order == BTF && !ftb);
}

void
PrivateCubeScreen::moveViewportAndPaint (const GLScreenPaintAttrib &sAttrib,
					 const GLMatrix            &transform,
					 CompOutput                *outputPtr,
					 unsigned int              mask,
					 PaintOrder                paintOrder,
					 int                       dx)
{
    int output;

    if (!cubeScreen->cubeShouldPaintViewport (sAttrib, transform, outputPtr, 
					      paintOrder))
	return;

    output = ((unsigned int) outputPtr->id () != (unsigned int) ~0)
    							 ? outputPtr->id () : 0;

    mPaintOrder = paintOrder;

    if (mNOutput > 1)
    {
	int cubeOutput, dView;

	/* translate to cube output */
	cubeOutput = mOutputMask[output];

	/* convert from window movement to viewport movement */
	dView = -dx;

	cubeOutput += dView;

	dView      = cubeOutput / mNOutput;
	cubeOutput = cubeOutput % mNOutput;

	if (cubeOutput < 0)
	{
	    cubeOutput += mNOutput;
	    dView--;
	}

	/* translate back to compiz output */
	output = mSrcOutput = mOutput[cubeOutput];

	cScreen->setWindowPaintOffset (-dView * screen->width (), 0);
	
	CompRegion reg (screen->outputDevs () [output]);
	cubeScreen->cubePaintViewport (sAttrib, transform, reg, 
			               &screen->outputDevs () [output], mask);
	cScreen->setWindowPaintOffset (0, 0);
    }
    else
    {
	CompRegion region;

	cScreen->setWindowPaintOffset (dx * screen->width (), 0);

	if (optionGetMultioutputMode () == MultioutputModeMultipleCubes)
	    region = CompRegion (*outputPtr);
	else
	    region = screen->region ();

	cubeScreen->cubePaintViewport (sAttrib, transform, region, outputPtr, mask);

	cScreen->setWindowPaintOffset (0, 0);
    }
}

void
PrivateCubeScreen::paintAllViewports (const GLScreenPaintAttrib &sAttrib,
				      const GLMatrix            &transform,
				      const CompRegion          &region,
				      CompOutput                *outputPtr,
				      unsigned int              mask,
				      int                       xMove,
				      float                     size,
				      int                       hsize,
				      PaintOrder                paintOrder)
{
    GLScreenPaintAttrib sa = sAttrib;

    int i;
    int xMoveAdd;
    int origXMoveAdd = 0; /* dx for the viewport we start
			     painting with (back-most). */
    int iFirstSign;       /* 1 if we do xMove += i first and
			     -1 if we do xMove -= i first. */

    if (mInvert == 1)
    {
	/* xMove ==> dx for the viewport which is the
	   nearest to the viewer in z axis.
	   xMove +/- hsize / 2 ==> dx for the viewport
	   which is the farthest to the viewer in z axis. */

	if ((sa.xRotate < 0.0f && hsize % 2 == 1) ||
	    (sa.xRotate > 0.0f && hsize % 2 == 0))
	{
	    origXMoveAdd = hsize / 2;
	    iFirstSign = 1;
	}
	else
	{
	    origXMoveAdd = -hsize / 2;
	    iFirstSign = -1;
	}
    }
    else
    {
	/* xMove is already the dx for farthest viewport. */
	if (sa.xRotate > 0.0f)
	    iFirstSign = -1;
	else
	    iFirstSign = 1;
    }

    for (i = 0; i <= hsize / 2; i++)
    {
	/* move to the correct viewport (back to front). */
	xMoveAdd = origXMoveAdd;	/* move to farthest viewport. */
	xMoveAdd += iFirstSign * i;	/* move i more viewports to
					   the right / left. */

	/* Needed especially for unfold.
	   We paint the viewports around xMove viewport.
	   Adding or subtracting hsize from xMove has no effect on
	   what viewport we paint, but can make shorter paths. */
	if (xMoveAdd < -hsize / 2)
	    xMoveAdd += hsize;
	else if (xMoveAdd > hsize / 2)
	    xMoveAdd -= hsize;

	/* Paint the viewport. */
	xMove += xMoveAdd;

	sa.yRotate -= mInvert * xMoveAdd * 360.0f / size;
	moveViewportAndPaint (sa, transform, outputPtr, mask,
			      paintOrder, xMove);
	sa.yRotate += mInvert * xMoveAdd * 360.0f / size;

	xMove -= xMoveAdd;

	/* do the same for an equally far viewport. */
	if (i == 0 || i * 2 == hsize)
	    continue;

	xMoveAdd = origXMoveAdd;	/* move to farthest viewport. */
	xMoveAdd -= iFirstSign * i;	/* move i more viewports to the
					   left / right (opposite side
					   from the one chosen first) */

	if (xMoveAdd < -hsize / 2)
	    xMoveAdd += hsize;
	else if (xMoveAdd > hsize / 2)
	    xMoveAdd -= hsize;

	xMove += xMoveAdd;

	sa.yRotate -= mInvert * xMoveAdd * 360.0f / size;
	moveViewportAndPaint (sa, transform, outputPtr, mask,
			      paintOrder, xMove);
	sa.yRotate += mInvert * xMoveAdd * 360.0f / size;

	xMove -= xMoveAdd;
    }
}

void
CubeScreen::cubeGetRotation (float &x, float &v, float &progress)
{
    WRAPABLE_HND_FUNCTN (cubeGetRotation, x, v, progress)

    x        = 0.0f;
    v        = 0.0f;
    progress = 0.0f;
}

void
CubeScreen::cubeClearTargetOutput (float xRotate, float vRotate)
{
    WRAPABLE_HND_FUNCTN (cubeClearTargetOutput, xRotate, vRotate)

    if (priv->mSky.size () > 0)
    {
	priv->gScreen->setLighting (false);
#ifndef USE_GLES
	glPushMatrix ();

	if (priv->optionGetSkydomeAnimated () &&
	    priv->mGrabIndex == 0)
	{
	    glRotatef (vRotate / 5.0f + 90.0f, 1.0f, 0.0f, 0.0f);
	    glRotatef (xRotate, 0.0f, 0.0f, -1.0f);
	}
	else
	{
	    glRotatef (90.0f, 1.0f, 0.0f, 0.0f);
	}

	glCallList (priv->mSkyListId);
	glPopMatrix ();
#endif
    }
    else
    {
	priv->gScreen->clearTargetOutput (GL_COLOR_BUFFER_BIT);
    }
}

void 
CubeScreen::cubePaintTop (const GLScreenPaintAttrib &sAttrib,
			  const GLMatrix            &transform,
			  CompOutput                *output,
			  int                       size,
			  const GLVector            &normal)
{
    WRAPABLE_HND_FUNCTN (cubePaintTop, sAttrib, transform, output, size, normal)

    GLScreenPaintAttrib sa = sAttrib;
    GLMatrix            sTransform = transform;

    unsigned short*	color;
    int			opacity;

    priv->gScreen->setLighting (true);

    color = priv->optionGetTopColor ();
    opacity = priv->mDesktopOpacity * color[3] / 0xffff;

    GLVertexBuffer	  *streamingBuffer = GLVertexBuffer::streamingBuffer ();
    std::vector <GLushort> colorData;

    colorData.push_back (color[0] * opacity / 0xffff);
    colorData.push_back (color[1] * opacity / 0xffff);
    colorData.push_back (color[2] * opacity / 0xffff);
    colorData.push_back (opacity);

    sa.yRotate += (360.0f / size) * (priv->mXRotations + 1);

    priv->gScreen->glApplyTransform (sa, output, &sTransform);

    sTransform.translate (priv->mOutputXOffset, -priv->mOutputYOffset, 0.0f);
    sTransform.scale (priv->mOutputXScale, priv->mOutputYScale, 1.0f);

    if ((priv->mDesktopOpacity != OPAQUE) || (color[3] != OPAQUE))
    {
#ifndef USE_GLES
	priv->gScreen->setTexEnvMode (GL_MODULATE);
#endif
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    bool withTexture = priv->mInvert == 1 && size == 4 && priv->mTexture.size ();

    if (withTexture)
	priv->mTexture[0]->enable (GLTexture::Good);

    streamingBuffer->begin (GL_TRIANGLE_FAN);
    streamingBuffer->addColors (1, &(colorData[0]));
    streamingBuffer->addVertices (priv->mNVertices >> 1, priv->mVertices);
    streamingBuffer->addNormals (1, const_cast <GLfloat *> (&normal[0]));

    if (withTexture)
	streamingBuffer->addTexCoords (0, 2, priv->mTc);

    streamingBuffer->end ();
    streamingBuffer->render (sTransform);

    if (withTexture)
	priv->mTexture[0]->disable ();

    priv->gScreen->setTexEnvMode (GL_REPLACE);
    glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

void 
CubeScreen::cubePaintBottom (const GLScreenPaintAttrib &sAttrib,
			     const GLMatrix            &transform,
			     CompOutput                *output,
			     int                       size,
			     const GLVector            &normal)
{
    WRAPABLE_HND_FUNCTN (cubePaintBottom, sAttrib, transform, output, size, normal)

    GLScreenPaintAttrib sa = sAttrib;
    GLMatrix            sTransform = transform;

    unsigned short*	color;
    int			opacity;

    priv->gScreen->setLighting (true);

    color   = priv->optionGetBottomColor ();
    opacity = priv->mDesktopOpacity * color[3] / 0xffff;

    GLVertexBuffer	  *streamingBuffer = GLVertexBuffer::streamingBuffer ();
    std::vector <GLushort> colorData;

    colorData.push_back (color[0] * opacity / 0xffff);
    colorData.push_back (color[1] * opacity / 0xffff);
    colorData.push_back (color[2] * opacity / 0xffff);
    colorData.push_back (opacity);

    sa.yRotate += (360.0f / size) * (priv->mXRotations + 1);

    priv->gScreen->glApplyTransform (sa, output, &sTransform);

    sTransform.translate (priv->mOutputXOffset, -priv->mOutputYOffset, 0.0f);
    sTransform.scale (priv->mOutputXScale, priv->mOutputYScale, 1.0f);

    if ((priv->mDesktopOpacity != OPAQUE) || (color[3] != OPAQUE))
    {
#ifndef USE_GLES
	priv->gScreen->setTexEnvMode (GL_MODULATE);
#endif
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    streamingBuffer->begin (GL_TRIANGLE_FAN);
    streamingBuffer->addColors (1, &(colorData[0]));
    streamingBuffer->addVertices (priv->mNVertices, priv->mVertices);
    streamingBuffer->addNormals (1, const_cast <GLfloat *> (&normal[0]));
    streamingBuffer->setVertexOffset (priv->mNVertices >> 1);
    streamingBuffer->setMaxVertices (priv->mNVertices >> 1);

    streamingBuffer->end ();
    streamingBuffer->render (sTransform);

    priv->gScreen->setTexEnvMode (GL_REPLACE);
    glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

void
CubeScreen::cubePaintInside (const GLScreenPaintAttrib &sAttrib,
			     const GLMatrix            &transform,
			     CompOutput                *output,
			     int                       size,
			     const GLVector            &normal)
{
    WRAPABLE_HND_FUNCTN (cubePaintInside, sAttrib, transform, output, size, normal)
}

void 
PrivateCubeScreen::glEnableOutputClipping (const GLMatrix &transform, 
					   const CompRegion &region,
					   CompOutput *output)
{
    if (mRotationState != CubeScreen::RotationNone)
    {
	/* FIXME: No output clipping in OpenGL|ES yet */
	#ifndef USE_GLES
	glPushMatrix ();
	glLoadMatrixf (transform.getMatrix ());
	glTranslatef (mOutputXOffset, -mOutputYOffset, 0.0f);
	glScalef (mOutputXScale, mOutputYScale, 1.0f);

	if (mInvert == 1)
	{
	    GLdouble clipPlane0[] = {  1.0, 0.0, 0.5 / mDistance, 0.0 };
	    GLdouble clipPlane1[] = {  -1.0,  0.0, 0.5 / mDistance, 0.0 };
	    GLdouble clipPlane2[] = {  0.0,  -1.0, 0.5 / mDistance, 0.0 };
	    GLdouble clipPlane3[] = { 0.0,  1.0, 0.5 / mDistance, 0.0 };
	    glClipPlane (GL_CLIP_PLANE0, clipPlane0);
	    glClipPlane (GL_CLIP_PLANE1, clipPlane1);
	    glClipPlane (GL_CLIP_PLANE2, clipPlane2);
	    glClipPlane (GL_CLIP_PLANE3, clipPlane3);
	}
	else
	{
	    GLdouble clipPlane0[] = {  -1.0, 0.0, -0.5 / mDistance, 0.0 };
	    GLdouble clipPlane1[] = {  1.0,  0.0, -0.5 / mDistance, 0.0 };
	    GLdouble clipPlane2[] = {  0.0,  1.0, -0.5 / mDistance, 0.0 };
	    GLdouble clipPlane3[] = { 0.0,  -1.0, -0.5 / mDistance, 0.0 };
	    glClipPlane (GL_CLIP_PLANE0, clipPlane0);
	    glClipPlane (GL_CLIP_PLANE1, clipPlane1);
	    glClipPlane (GL_CLIP_PLANE2, clipPlane2);
	    glClipPlane (GL_CLIP_PLANE3, clipPlane3);
	}

	glEnable (GL_CLIP_PLANE0);
	glEnable (GL_CLIP_PLANE1);
	glEnable (GL_CLIP_PLANE2);
	glEnable (GL_CLIP_PLANE3);

	glPopMatrix ();
	#endif
    }
    else
	gScreen->glEnableOutputClipping (transform, region, output);
}

void 
CubeScreen::cubePaintViewport (const GLScreenPaintAttrib &sAttrib,
			       const GLMatrix            &transform,
			       const CompRegion          &region,
			       CompOutput                *output,
			       unsigned int              mask)
{
    WRAPABLE_HND_FUNCTN (cubePaintViewport, sAttrib, transform, region, output, mask)

    priv->gScreen->glPaintTransformedOutput (sAttrib, transform, region, 
					     output, mask);
}

void 
PrivateCubeScreen::glPaintTransformedOutput (const GLScreenPaintAttrib &sAttrib,
					     const GLMatrix            &transform, 
					     const CompRegion          &region,
					     CompOutput                *outputPtr, 
					     unsigned int              mask)
{
    GLScreenPaintAttrib sa = sAttrib;
    float               xRotate, vRotate, progress;
    int                 hsize;
    float               size;
    GLenum              filter = gScreen->textureFilter ();
    PaintOrder          paintOrder;
    bool                wasCulled = false;
    bool                paintCaps;
    int                 cullNorm, cullInv;
    int                 output = 0;

    output = ((unsigned int) outputPtr->id () != (unsigned int) ~0) ?
    							   outputPtr->id () : 0;

    mReversedWindowList = cScreen->getWindowPaintList ();
    mReversedWindowList.reverse ();

    if ((((unsigned int) outputPtr->id () != (unsigned int) ~0) && mRecalcOutput) ||
	(((unsigned int) outputPtr->id () == (unsigned int) ~0) && !mRecalcOutput &&
	 mNOutput > 1))
    {
	mRecalcOutput = ((unsigned int) outputPtr->id () == (unsigned int) ~0);
	mNOutput      = 1;
	updateGeometry (screen->vpSize ().width (), mInvert);
    }

    hsize = screen->vpSize ().width () * mNOutput;
    size  = hsize;

    glGetIntegerv (GL_CULL_FACE_MODE, &cullNorm);
    cullInv   = (cullNorm == GL_BACK)? GL_FRONT : GL_BACK;
    wasCulled = glIsEnabled (GL_CULL_FACE);

    if (!mFullscreenOutput)
    {
	mOutputXScale = (float) screen->width () / outputPtr->width ();
	mOutputYScale = (float) screen->height () / outputPtr->height ();

	mOutputXOffset =
	    (screen->width () / 2.0f -
	     (outputPtr->x1 () + outputPtr->x2 ()) / 2.0f) /
	    (float) outputPtr->width ();

	mOutputYOffset =
	    (screen->height () / 2.0f -
	     (outputPtr->y1 () +
	      outputPtr->y2 ()) / 2.0f) /
	    (float) outputPtr->height ();
    }
    else
    {
	mOutputXScale  = 1.0f;
	mOutputYScale  = 1.0f;
	mOutputXOffset = 0.0f;
	mOutputYOffset = 0.0f;
    }

    cubeScreen->cubeGetRotation (xRotate, vRotate, progress);

    sa.xRotate += xRotate;
    sa.vRotate += vRotate;

    if (!mCleared[output])
    {
	float rRotate;

	rRotate = xRotate - ((screen->vp ().x () *360.0f) / screen->vpSize ().width ());

	cubeScreen->cubeClearTargetOutput (rRotate, vRotate);
	mCleared[output] = true;
    }

    mask &= ~PAINT_SCREEN_CLEAR_MASK;

    if (mGrabIndex)
    {
	sa.vRotate = 0.0f;

	size += mUnfold * 8.0f;
	size += powf (mUnfold, 6) * 64.0;
	size += powf (mUnfold, 16) * 8192.0;

	sa.zTranslate = -mInvert * (0.5f / tanf (M_PI / size));

	/* distance we move the camera back when unfolding the cube.
	   currently hardcoded to 1.5 but it should probably be optional. */
	sa.zCamera -= mUnfold * 1.5f;
    }
    else
    {
	if (vRotate > 100.0f)
	    sa.vRotate = 100.0f;
	else if (vRotate < -100.0f)
	    sa.vRotate = -100.0f;
	else
	    sa.vRotate = vRotate;

	sa.zTranslate = -mInvert * mDistance;
    }

    if (sa.xRotate > 0.0f)
	mXRotations = (int) (hsize * sa.xRotate + 180.0f) / 360.0f;
    else
	mXRotations = (int) (hsize * sa.xRotate - 180.0f) / 360.0f;

    sa.xRotate -= (360.0f * mXRotations) / hsize;
    sa.xRotate *= mInvert;

    sa.xRotate = sa.xRotate / size * hsize;

    if (mGrabIndex && optionGetMipmap ())
	gScreen->setTextureFilter (GL_LINEAR_MIPMAP_LINEAR);

    if (mInvert == 1)
    {
	/* Outside cube - start with FTB faces */
	paintOrder = FTB;
	glCullFace (cullInv);
    }
    else
    {
	/* Inside cube - start with BTF faces */
	paintOrder = BTF;
    }

    if (mInvert == -1 || cubeScreen->cubeShouldPaintAllViewports ())
	paintAllViewports (sa, transform, region, outputPtr,
			   mask, mXRotations, size, hsize, paintOrder);

    glCullFace (cullNorm);

    if (wasCulled && cubeScreen->cubeShouldPaintAllViewports ())
	glDisable (GL_CULL_FACE);

    paintCaps = !mGrabIndex && (hsize > 2) && !mCapsPainted[output] &&
	        (mInvert != 1 || mDesktopOpacity != OPAQUE ||
		 cubeScreen->cubeShouldPaintAllViewports () || sa.vRotate != 0.0f ||
		 sa.yTranslate != 0.0f);

    if (paintCaps)
    {
	Bool topDir, bottomDir, allCaps;

	std::vector<GLVector> top;
	top.push_back (GLVector (0.5, 0.5,  0.0, 1.0));
	top.push_back (GLVector (0.0, 0.5, -0.5, 1.0));
	top.push_back (GLVector (0.0, 0.5,  0.0, 1.0));

	std::vector<GLVector> bottom;
	bottom.push_back (GLVector (0.5, -0.5,  0.0, 1.0));
	bottom.push_back (GLVector (0.0, -0.5, -0.5, 1.0));
	bottom.push_back (GLVector (0.0, -0.5,  0.0, 1.0));

	topDir    = cubeScreen->cubeCheckOrientation (sa, transform, outputPtr, top);
	bottomDir = cubeScreen->cubeCheckOrientation (sa, transform, outputPtr, bottom);

	mCapsPainted[output] = TRUE;

	allCaps = cubeScreen->cubeShouldPaintAllViewports () || mInvert != 1;

	if (topDir && bottomDir)
	{
	    if (allCaps)
	    {
		cubeScreen->cubePaintBottom (sa, transform, outputPtr, hsize, GLVector (0.0f, -1.0f, 0.0f, 1.0f));
		cubeScreen->cubePaintInside (sa, transform, outputPtr, hsize, GLVector (0.0f, 0.0f, -1.0f, 1.0f));
	    }
	    cubeScreen->cubePaintTop (sa, transform, outputPtr, hsize, GLVector (0.0f, -1.0f, 0.0f, 1.0f));
	}
	else if (!topDir && !bottomDir)
	{
	    if (allCaps)
	    {
		cubeScreen->cubePaintTop (sa, transform, outputPtr, hsize, GLVector (0.0f, 1.0f, 0.0f, 1.0f));
		cubeScreen->cubePaintInside (sa, transform, outputPtr, hsize, GLVector (0.0f, 0.0f, -1.0f, 1.0f));
	    }
	    cubeScreen->cubePaintBottom (sa, transform, outputPtr, hsize, GLVector (0.0f, -1.0f, 0.0f, 1.0f));
	}
	else if (allCaps)
	{
	    cubeScreen->cubePaintTop (sa, transform, outputPtr, hsize, GLVector (0.0f, 1.0f, 0.0f, 1.0f));
	    cubeScreen->cubePaintBottom (sa, transform, outputPtr, hsize, GLVector (0.0f, -1.0f, 0.0f, 1.0f));
	    cubeScreen->cubePaintInside (sa, transform, outputPtr, hsize, GLVector (0.0f, 0.0f, -1.0f, 1.0f));
	}
    }

    if (wasCulled)
	glEnable (GL_CULL_FACE);

    if (mInvert == 1)
    {
	/* Outside cube - continue with BTF faces */
	paintOrder = BTF;
    }
    else
    {
	/* Inside cube - continue with FTB faces */
	paintOrder = FTB;
	glCullFace (cullInv);
    }

    if (mInvert == 1 || cubeScreen->cubeShouldPaintAllViewports ())
	paintAllViewports (sa, transform, region, outputPtr, mask, mXRotations,
			   size, hsize, paintOrder);

    glCullFace (cullNorm);

    gScreen->setTextureFilter (filter);
}

bool 
PrivateCubeWindow::glPaint (const GLWindowPaintAttrib &attrib, 
			    const GLMatrix            &transform,
                            const CompRegion          &region, 
			    unsigned int              mask)
{

    if ((window->type () & CompWindowTypeDesktopMask) &&
	(attrib.opacity != cubeScreen->priv->mDesktopOpacity))
    {
	GLWindowPaintAttrib wAttrib = attrib;

	wAttrib.opacity = cubeScreen->priv->mDesktopOpacity;

	return gWindow->glPaint (wAttrib, transform, region, mask);
    }
    else
	return gWindow->glPaint (attrib, transform, region, mask);

}

const CompWindowList &
PrivateCubeScreen::getWindowPaintList ()
{
    if (mPaintOrder == FTB)
	return mReversedWindowList;
    else
        return cScreen->getWindowPaintList ();
}

void 
PrivateCubeScreen::glApplyTransform (const GLScreenPaintAttrib &sAttrib,
				     CompOutput                *output, 
				     GLMatrix                  *transform)
{
    transform->translate (mOutputXOffset, -mOutputYOffset, 0.0f);
    transform->scale (mOutputXScale, mOutputYScale, 1.0f);

    gScreen->glApplyTransform (sAttrib, output, transform);

    transform->scale (1.0f / mOutputXScale, 1.0 / mOutputYScale, 1.0f);
    transform->translate (-mOutputXOffset, mOutputYOffset, 0.0f);
}

bool
PrivateCubeScreen::unfold (CompAction         *action,
		           CompAction::State  state,
			   CompOption::Vector &options)
{
    Window     xid;

    xid = CompOption::getIntOptionNamed (options, "root");

    if (::screen->root () == xid)
    {
	CUBE_SCREEN (screen);

	if (screen->vpSize ().width () * cs->priv->mNOutput < 4)
	    return false;

	if (screen->otherGrabExist ("rotate", "switcher", "cube", NULL))
	    return false;

	if (!cs->priv->mGrabIndex)
	    cs->priv->mGrabIndex = screen->pushGrab (screen->invisibleCursor (), "cube");

	if (cs->priv->mGrabIndex)
	{
	    cs->priv->mUnfolded = true;
	    cs->priv->cScreen->damageScreen ();
	}

	if (state & CompAction::StateInitButton)
	    action->setState (action->state () | CompAction::StateTermButton);

	if (state & CompAction::StateInitKey)
	    action->setState (action->state () | CompAction::StateTermKey);
    }

    return false;
}

bool
PrivateCubeScreen::fold (CompAction         *action,
			 CompAction::State  state,
			 CompOption::Vector &options)
{
    Window     xid;
    xid = CompOption::getIntOptionNamed (options, "root");

    if (!xid || ::screen->root () == xid)
    {
	CUBE_SCREEN (screen);
	
	if (cs->priv->mGrabIndex)
	{
	    cs->priv->mUnfolded = false;
	    cs->priv->cScreen->damageScreen ();
	}
    }

    action->setState (action->state () & ~(CompAction::StateTermButton | CompAction::StateTermKey));

    return false;
}

void
PrivateCubeScreen::outputChangeNotify ()
{
    updateOutputs ();
    updateGeometry (screen->vpSize ().width (), mInvert);

    screen->outputChangeNotify ();
}

bool 
PrivateCubeScreen::setOptionForPlugin (const char *plugin,
				       const char *name,
				       CompOption::Value &v)
{
    bool status;

    status = screen->setOptionForPlugin (plugin, name, v);

    if (status)
    {
	if (strcmp (plugin, "core") == 0 && strcmp (name, "hsize") == 0)
	    updateGeometry (screen->vpSize ().width (), mInvert);
    }

    return status;
}

PrivateCubeScreen::PrivateCubeScreen (CompScreen *s) :
    cScreen (CompositeScreen::get (s)),
    gScreen (GLScreen::get (s)),
    cubeScreen (CubeScreen::get (s))
{

    mPw = 0;
    mPh = 0;

    mInvert = 1;

    for (int i = 0; i < 8; i++)
	mTc[i] = 0.0f;

    mNVertices = 0;
    mVertices  = NULL;

    mGrabIndex = 0;

    mSrcOutput = 0;

    mSkyListId = 0;

    mImgCurFile = 0;

    mUnfolded = false;
    mUnfold   = 0.0f;

    mUnfoldVelocity = 0.0f;

    mPaintAllViewports = false;
    mFullscreenOutput  = true;

    mOutputXScale  = 1.0f;
    mOutputYScale  = 1.0f;
    mOutputXOffset = 0.0f;
    mOutputYOffset = 0.0f;

    mRotationState = CubeScreen::RotationNone;

    mDesktopOpacity = OPAQUE;
    mPaintOrder = BTF;

    mLastOpacityIndex = CubeOptions::InactiveOpacity;


    mRecalcOutput = false;

    memset (mCleared, 0, sizeof (mCleared));

    updateOutputs ();

    updateGeometry (screen->vpSize ().width (), mInvert);

    optionSetUnfoldKeyInitiate (PrivateCubeScreen::unfold);
    optionSetUnfoldKeyTerminate (PrivateCubeScreen::fold);

    ScreenInterface::setHandler (s);
    CompositeScreenInterface::setHandler (cScreen);
    GLScreenInterface::setHandler (gScreen);
}

PrivateCubeScreen::~PrivateCubeScreen ()
{
    if (mVertices)
	free (mVertices);

#ifndef USE_GLES
    if (mSkyListId)
	glDeleteLists (mSkyListId, 1);
#endif
}


template class PluginClassHandler<PrivateCubeWindow, CompWindow, COMPIZ_CUBE_ABI>;

PrivateCubeWindow::PrivateCubeWindow (CompWindow *w) :
    PluginClassHandler<PrivateCubeWindow, CompWindow, COMPIZ_CUBE_ABI> (w),
    window (w),
    cWindow (CompositeWindow::get (w)),
    gWindow (GLWindow::get (w)),
    cubeScreen (CubeScreen::get (screen))
{
    GLWindowInterface::setHandler (gWindow, true);
}

PrivateCubeWindow::~PrivateCubeWindow ()
{
}

template class PluginClassHandler<CubeScreen, CompScreen, COMPIZ_CUBE_ABI>;

CubeScreen::CubeScreen (CompScreen *s) :
    PluginClassHandler<CubeScreen, CompScreen, COMPIZ_CUBE_ABI> (s),
    priv (new PrivateCubeScreen (s))
{
}

CubeScreen::~CubeScreen ()
{
    delete priv;
}


CompOption::Vector &
CubeScreen::getOptions ()
{
    return priv->getOptions ();
}

bool
CubeScreen::setOption (const CompString  &name,
		       CompOption::Value &value)
{
    return priv->setOption (name, value);
}

bool
CubePluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) ||
        !CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) ||
        !CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
	return false;

    CompPrivate p;
    p.uval = COMPIZ_CUBE_ABI;
    screen->storeValue ("cube_ABI", p);

    return true;
}

void
CubePluginVTable::fini ()
{
    screen->eraseValue ("cube_ABI");
}

