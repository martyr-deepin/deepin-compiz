#include "3d.h"

COMPIZ_PLUGIN_20090315 (td, TdPluginVTable);

const double PI = 3.14159265359f;

void
setFunctions (bool enabled)
{
    TD_SCREEN (screen);

    tds->gScreen->glPaintOutputSetEnabled (tds, enabled);
    tds->gScreen->glApplyTransformSetEnabled (tds, enabled);
    tds->cScreen->donePaintSetEnabled (tds, enabled);
    tds->cubeScreen->cubePaintViewportSetEnabled (tds, enabled);
    tds->cubeScreen->cubeShouldPaintViewportSetEnabled (tds, enabled);
    tds->cubeScreen->cubeShouldPaintAllViewportsSetEnabled (tds, enabled);

    foreach (CompWindow *w, screen->windows ())
    {
	TD_WINDOW (w);

	tdw->gWindow->glPaintSetEnabled (tdw, enabled);
    }
}

bool
TdWindow::is3D ()
{
    if (window->overrideRedirect ())
	return false;

    if (!window->isViewable () || window->shaded ())
	return false;

    if (window->state () & (CompWindowStateSkipPagerMask |
		    CompWindowStateSkipTaskbarMask))
	return false;

    if (!TdScreen::get (screen)->optionGetWindowMatch ().evaluate (window))
	return false;

    return true;
}

void
TdScreen::preparePaint (int msSinceLastPaint)
{
    bool       active;

    CUBE_SCREEN (screen);

    active = (cs->rotationState () != CubeScreen::RotationNone) && screen->vpSize ().width () > 2 &&
	     !(optionGetManualOnly () && (cs->rotationState () != CubeScreen::RotationManual));

    if (active || mBasicScale != 1.0)
    {
	float maxDiv = (float) optionGetMaxWindowSpace () / 100;
	float minScale = (float) optionGetMinCubeSize () / 100;
	float x, progress;

	cs->cubeGetRotation (x, x, progress);

	mMaxDepth = 0;
	foreach (CompWindow *w, screen->windows ())
	{
	    TD_WINDOW (w);
	    tdw->mIs3D = false;
	    tdw->mDepth = 0;

	    if (!tdw->is3D ())
		continue;

	    tdw->mIs3D = true;
	    mMaxDepth++;
	    tdw->mDepth = mMaxDepth;

	}

	minScale =  MAX (minScale, 1.0 - (mMaxDepth * maxDiv));
	mBasicScale = 1.0 - ((1.0 - minScale) * progress);
	mDamage = (progress != 0.0 && progress != 1.0);
    }
    else
    {
	mBasicScale = 1.0;
    }

    /* comparing float values with != is error prone, so better cache
       the comparison and allow a small difference */
    mActive       = (fabs (mBasicScale - 1.0f) > 1e-4);
    mCurrentScale = mBasicScale;

    cScreen->preparePaint (msSinceLastPaint);

    setFunctions (mActive);
}

#define DOBEVEL(corner) (tds->optionGetBevel##corner () ? bevel : 0)

#define SET_V								       \
    for (int i = 0; i < 4; i++)						       \
	v[i] = tPoint[i];

#define ADDQUAD(x1,y1,x2,y2)                                      \
	point[GLVector::x] = x1; point[GLVector::y] = y1;                               \
	tPoint = transform * point;       \
	SET_V \
	glVertex4fv (v);                                  \
	point[GLVector::x] = x2; point[GLVector::y] = y2;                               \
	tPoint = transform * point;        \
	SET_V \
	glVertex4fv (v);                                   \
	tPoint = tds->mBTransform * point; \
	SET_V \
	glVertex4fv (v);                                \
	point[GLVector::x] = x1; point[GLVector::y] = y1;                               \
	tPoint = tds->mBTransform * point; \
	SET_V \
	glVertex4fv (v);                                   \

#define ADDBEVELQUAD(x1,y1,x2,y2,m1,m2)             \
	point[GLVector::x] = x1; point[GLVector::y] = y1;                 \
	tPoint = m1 * point; \
	SET_V \
	glVertex4fv (v);                     \
	tPoint = m2 * point; \
	SET_V \
	glVertex4fv (v);                     \
	point[GLVector::x] = x2; point[GLVector::y] = y2;                 \
	tPoint = m2 * point; \
	SET_V \
	glVertex4fv (v);                     \
	tPoint = m1 * point; \
	SET_V \
	glVertex4fv (v);                     \

bool
TdWindow::glPaintWithDepth (const GLWindowPaintAttrib &attrib,
			    const GLMatrix	      &transform,
			    const CompRegion	      &region,
			    unsigned int	      mask)
{
    bool status;
    int            wx, wy, ww, wh;
    int            bevel, cull, cullInv;
    GLVector       point, tPoint;
    unsigned short *c;

    TD_SCREEN (screen);
    CUBE_SCREEN (screen);

    glIsEnabled (GL_CULL_FACE);

    wx = window->x () - window->input ().left;
    wy = window->y () - window->input ().top;

    ww = window->width () + window->input ().left + window->input ().right;
    wh = window->height () + window->input ().top + window->input ().bottom;

    bevel = tds->optionGetBevel ();

    glGetIntegerv (GL_CULL_FACE_MODE, &cull);
    cullInv = (cull == GL_BACK)? GL_FRONT : GL_BACK;

    if (ww && wh && !(mask & PAINT_WINDOW_OCCLUSION_DETECTION_MASK) &&
	((cs->paintOrder () == FTB && mFtb) ||
	(cs->paintOrder () == BTF && !mFtb)))
    {
        float v[4];
	int   temp;

	/* Paint window depth. */
	glPushMatrix ();
	glLoadIdentity ();

	if (cs->paintOrder () == BTF)
	    glCullFace (cullInv);

	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (window->id () == screen->activeWindow ())
	    c = tds->optionGetWidthColor ();
	else
	    c = tds->optionGetWidthColorInactive ();

	temp = c[3] * gWindow->paintAttrib ().opacity;
	temp /= 0xffff;
	glColor4us (c[0], c[1], c[2], temp);

	point[GLVector::z] = 0.0f;
	point[GLVector::w] = 1.0f;

	glBegin (GL_QUADS);

	/* Top */
	ADDQUAD (wx + ww - DOBEVEL (Topleft), wy + 0.01,
		 wx + DOBEVEL (Topright), wy + 0.01);

	/* Bottom */
	ADDQUAD (wx + DOBEVEL (Bottomleft), wy + wh - 0.01,
		 wx + ww - DOBEVEL (Bottomright), wy + wh - 0.01);

	/* Left */
	ADDQUAD (wx + 0.01, wy + DOBEVEL (Topleft),
		 wx + 0.01, wy + wh - DOBEVEL (Bottomleft));

	/* Right */
	ADDQUAD (wx + ww - 0.01, wy + wh - DOBEVEL (Topright),
		 wx + ww - 0.01, wy + DOBEVEL (Bottomright));

	/* Top left bevel */
	if (tds->optionGetBevelTopleft ())
	{
	    ADDBEVELQUAD (wx + bevel / 2.0f,
			  wy + bevel - bevel / 1.2f,
			  wx, wy + bevel,
			  tds->mBTransform, transform);

	    ADDBEVELQUAD (wx + bevel / 2.0f,
			  wy + bevel - bevel / 1.2f,
			  wx + bevel, wy,
			  transform, tds->mBTransform);

	}

	/* Bottom left bevel */
	if (tds->optionGetBevelBottomleft ())
	{
	    ADDBEVELQUAD (wx + bevel / 2.0f,
			  wy + wh - bevel + bevel / 1.2f,
			  wx, wy + wh - bevel,
			  transform, tds->mBTransform);

	    ADDBEVELQUAD (wx + bevel / 2.0f,
			  wy + wh - bevel + bevel / 1.2f,
			  wx + bevel, wy + wh,
			  tds->mBTransform, transform);
	}

	/* Bottom right bevel */
	if (tds->optionGetBevelBottomright ())
	{
	    ADDBEVELQUAD (wx + ww - bevel / 2.0f,
			  wy + wh - bevel + bevel / 1.2f,
			  wx + ww - bevel, wy + wh,
			  transform, tds->mBTransform);

	    ADDBEVELQUAD (wx + ww - bevel / 2.0f,
			  wy + wh - bevel + bevel / 1.2f,
			  wx + ww, wy + wh - bevel,
			  tds->mBTransform, transform);


	}

	/* Top right bevel */
	if (tds->optionGetBevelTopright ())
	{
	    ADDBEVELQUAD (wx + ww - bevel, wy,
			  wx + ww - bevel / 2.0f,
			  wy + bevel - bevel / 1.2f,
			  transform, tds->mBTransform);

	    ADDBEVELQUAD (wx + ww, wy + bevel,
			  wx + ww - bevel / 2.0f,
			  wy + bevel - bevel / 1.2f,
			  tds->mBTransform, transform);
	}
	glEnd ();

	glColor4usv (defaultColor);
	glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glPopMatrix ();

	if (cs->paintOrder () == BTF)
	    glCullFace (cull);
    }

    if (cs->paintOrder () == BTF)
	status = gWindow->glPaint (attrib, transform, region, mask);
    else
	status = gWindow->glPaint (attrib, tds->mBTransform, region,
				   mask | PAINT_WINDOW_TRANSFORMED_MASK);

    return status;
}

bool
TdWindow::glPaint (const GLWindowPaintAttrib &attrib,
		   const GLMatrix	     &transform,
		   const CompRegion	     &region,
		   unsigned int		     mask)
{
    bool           status;

    TD_SCREEN (screen);

    if (mDepth != 0.0f && !tds->mPainting3D && tds->mActive)
	mask |= PAINT_WINDOW_NO_CORE_INSTANCE_MASK;

    if (tds->mPainting3D && tds->optionGetWidth () && (mDepth != 0.0f) &&
	tds->mWithDepth)
    {
	status = glPaintWithDepth (attrib, transform, region, mask);
    }
    else
    {
	status = gWindow->glPaint (attrib, transform, region, mask);
    }

    return status;
}

void
TdScreen::glApplyTransform (const GLScreenPaintAttrib &attrib,
			    CompOutput *output,
			    GLMatrix *transform)
{

    gScreen->glApplyTransform (attrib, output, transform);

    transform->scale (mCurrentScale, mCurrentScale, mCurrentScale);
}


void
TdScreen::cubePaintViewport (const GLScreenPaintAttrib &attrib,
			     const GLMatrix	       &transform,
			     const CompRegion	       &region,
			     CompOutput		       *output,
			     unsigned int	       mask)
{
    CUBE_SCREEN (screen);

    if (cs->paintOrder () == BTF)
    {
        cs->cubePaintViewport (attrib, transform,region, output, mask);
    }

    if (mActive)
    {
	GLMatrix      mTransform; // NOT a member variable
	GLMatrix      screenSpace;
	GLMatrix      screenSpaceOffset;
	TdWindow      *tdw;
	CompWindowList                   pl;
	CompWindowList::iterator it;
	float         wDepth = 0.0;
	float         pointZ = cs->invert () * cs->distance ();
	unsigned int  newMask;

	std::vector<GLVector> vPoints;
	vPoints.push_back (GLVector (-0.5, 0.0, pointZ, 1.0));
	vPoints.push_back (GLVector (0.0, 0.5, pointZ, 1.0));
	vPoints.push_back (GLVector (0.0, 0.0, pointZ, 1.0));

	if (mWithDepth)
	    wDepth = -MIN((optionGetWidth ()) / 30, (1.0 - mBasicScale) /
			  mMaxDepth);

	if (wDepth != 0.0)
	{
	    /* all BTF windows in normal order */
	    foreach (CompWindow *w, screen->windows ())
	    {
		tdw = TdWindow::get (w);

		if (!tdw->mIs3D)
		    continue;

		mCurrentScale = mBasicScale +
		                    (tdw->mDepth * ((1.0 - mBasicScale) /
				    mMaxDepth));

		tdw->mFtb = cs->cubeCheckOrientation (attrib, transform, output,
						  vPoints);
	    }
	}

	mCurrentScale = mBasicScale;
	mPainting3D   = true;

	gScreen->setLighting (true);

	screenSpace.reset ();
	screenSpace.toScreenSpace (output, -attrib.zTranslate);

	glPushMatrix ();

	pl = cScreen->getWindowPaintList ();

	/* paint all windows from bottom to top */
	for (it = pl.begin (); it != pl.end (); ++it)
	{
	    CompWindow *w = (*it);
	    tdw = TdWindow::get (w);

	    if (w->destroyed ())
		continue;

	    if (!w->shaded () || !w->isViewable ())
	    {
		if (!tdw->cWindow->damaged ())
		    continue;
	    }

	    mTransform = transform;
	    newMask = PAINT_WINDOW_ON_TRANSFORMED_SCREEN_MASK;

	    if (tdw->mDepth != 0.0f)
	    {
		mCurrentScale = mBasicScale +
		                    (tdw->mDepth * ((1.0 - mBasicScale) /
						   mMaxDepth));

		if (wDepth != 0.0)
		{
		    mCurrentScale += wDepth;
		    mBTransform   = transform;
		    gScreen->glApplyTransform (attrib, output, &mBTransform);
		    mCurrentScale -= wDepth;
		}

		gScreen->glApplyTransform (attrib, output, &mTransform);

		gScreen->glEnableOutputClipping (mTransform, region, output);

		if ((cScreen->windowPaintOffset ().x () != 0 ||
		     cScreen->windowPaintOffset ().y () != 0) &&
		    !w->onAllViewports ())
		{
		    CompPoint moveOffset;
		    moveOffset = w->getMovementForOffset (
		    			cScreen->windowPaintOffset ());

		    screenSpaceOffset = screenSpace;
		    screenSpaceOffset.translate (moveOffset.x (),
		    				 moveOffset.y (), 0);

		    if (wDepth != 0.0)
		        mBTransform = mBTransform * screenSpaceOffset;
		    mTransform = mTransform * screenSpaceOffset;

		    newMask |= PAINT_WINDOW_WITH_OFFSET_MASK;
		}
		else
		{
		    if (wDepth != 0.0)
			mBTransform = mBTransform * screenSpace;
		    mTransform = mTransform * screenSpace;
		}

		glLoadMatrixf (mTransform.getMatrix ());

		tdw->gWindow->glPaint (tdw->gWindow->paintAttrib (), mTransform,
				  infiniteRegion, newMask);

		gScreen->glDisableOutputClipping ();

	    }
	}

	glPopMatrix ();

	mPainting3D   = false;
	mCurrentScale = mBasicScale;

    }


    if (cs->paintOrder () == FTB)
    {
        cs->cubePaintViewport (attrib, transform, region, output, mask);
    }
}

bool
TdScreen::cubeShouldPaintAllViewports ()
{
    return true;
}

bool
TdScreen::cubeShouldPaintViewport (const GLScreenPaintAttrib &attrib,
				   const GLMatrix	     &transform,
				   CompOutput		     *outputPtr,
				   PaintOrder		     order)
{
    bool rv = false;

    CUBE_SCREEN (screen);

    rv = cs->cubeShouldPaintViewport (attrib, transform, outputPtr, order);

    if (mActive)
    {
	float pointZ = cs->invert () * cs->distance ();
	bool  ftb1, ftb2;

	std::vector<GLVector> vPoints;

	vPoints.push_back (GLVector (-0.5, 0.0, pointZ, 1.0));
	vPoints.push_back (GLVector (0.0, 0.5, pointZ, 1.0));
	vPoints.push_back (GLVector (0.0, 0.0, pointZ, 1.0));

	mCurrentScale = 1.0;

	ftb1 = cs->cubeCheckOrientation (attrib, transform, outputPtr, vPoints);

	mCurrentScale = mBasicScale;

	ftb2 = cs->cubeCheckOrientation (attrib, transform, outputPtr, vPoints);

	return (order == FTB && (ftb1 || ftb2)) ||
	       (order == BTF && (!ftb1 || !ftb2)) || rv;
    }

    return true;
}

bool
TdScreen::glPaintOutput (const GLScreenPaintAttrib &attrib,
			 const GLMatrix		   &transform,
			 const CompRegion	   &region,
			 CompOutput		   *output,
			 unsigned int		   mask)
{
    bool status;

    if (mActive)
    {
	CompPlugin *p;

	mask |= PAINT_SCREEN_TRANSFORMED_MASK |
	        PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS_MASK |
		PAINT_SCREEN_NO_OCCLUSION_DETECTION_MASK;

	mWithDepth = true;

	p = CompPlugin::find ("cubeaddon");
	if (p)
	{
	    CompOption::Vector &options = p->vTable->getOptions ();
	    CompOption option;

	    mWithDepth = (CompOption::getIntOptionNamed
					      (options, "deformation", 0) == 0);
	}
    }


    status = gScreen->glPaintOutput (attrib, transform, region, output, mask);

    return status;
}

void
TdScreen::donePaint ()
{
    if (mActive && mDamage)
    {
	mDamage = false;
	cScreen->damageScreen ();
    }

    cScreen->donePaint ();
}

TdScreen::TdScreen (CompScreen *screen) :
    PluginClassHandler <TdScreen, CompScreen> (screen),
    cScreen (CompositeScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    cubeScreen (CubeScreen::get (screen)),
    mActive (false),
    mPainting3D (false),
    mCurrentScale (1.0f),
    mBasicScale (1.0f)
{
    CompositeScreenInterface::setHandler (cScreen, false);
    GLScreenInterface::setHandler (gScreen, false);
    CubeScreenInterface::setHandler (cubeScreen, false);

    cScreen->preparePaintSetEnabled (this, true);
}

TdScreen::~TdScreen ()
{
}

TdWindow::TdWindow (CompWindow *window) :
    PluginClassHandler <TdWindow, CompWindow> (window),
    window (window),
    cWindow (CompositeWindow::get (window)),
    gWindow (GLWindow::get (window)),
    mIs3D (false),
    mDepth (0.0f)
{
    GLWindowInterface::setHandler (gWindow, false);
}

TdWindow::~TdWindow ()
{
}

bool
TdPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) ||
    	!CompPlugin::checkPluginABI ("cube", COMPIZ_CUBE_ABI) ||
        !CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) ||
        !CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
        return false;

    return true;
}
