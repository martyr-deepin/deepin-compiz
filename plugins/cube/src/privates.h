/*
 * Copyright © 2007 Novell, Inc.
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
 */

#ifndef _CUBE_PRIVATES_H
#define _CUBE_PRIVATES_H

#include <cube/cube.h>
#include "cube_options.h"

class PrivateCubeScreen :
    public ScreenInterface,
    public CompositeScreenInterface,
    public GLScreenInterface,
    public CubeOptions
{
    public:

	PrivateCubeScreen (CompScreen *s);
	~PrivateCubeScreen ();

	CompositeScreen *cScreen;
	GLScreen        *gScreen;
	CubeScreen      *cubeScreen;

	void preparePaint (int);
	void donePaint ();
	
	void paint (CompOutput::ptrList &outputs, unsigned int);

        bool glPaintOutput (const GLScreenPaintAttrib &,
			    const GLMatrix &, const CompRegion &,
			    CompOutput *, unsigned int);
	
	void glPaintTransformedOutput (const GLScreenPaintAttrib &,
				       const GLMatrix &, const CompRegion &,
				       CompOutput *, unsigned int);


	void glEnableOutputClipping (const GLMatrix &, const CompRegion &,
				     CompOutput *);

	void glApplyTransform (const GLScreenPaintAttrib &,
			       CompOutput *, GLMatrix *);

	bool setOptionForPlugin (const char *plugin,
                                 const char *name,
                                 CompOption::Value &v);

	void outputChangeNotify ();

	const CompWindowList & getWindowPaintList ();

	bool updateGeometry (int sides, int invert);
	void updateOutputs ();
	void updateSkydomeTexture ();
	void updateSkydomeList (GLfloat fRadius);

	bool setOption (const CompString &name, CompOption::Value &value);

	bool adjustVelocity ();

	void moveViewportAndPaint (const GLScreenPaintAttrib &sAttrib,
				   const GLMatrix            &transform,
				   CompOutput                *output,
				   unsigned int              mask,
				   PaintOrder                paintOrder,
				   int                       dx);

	void paintAllViewports (const GLScreenPaintAttrib &sAttrib,
				const GLMatrix            &transform,
				const CompRegion          &region,
				CompOutput                *outputPtr,
				unsigned int              mask,
				int                       xMove,
				float                     size,
				int                       hsize,
				PaintOrder                paintOrder);

	static bool unfold (CompAction         *action,
			    CompAction::State  state,
			    CompOption::Vector &options);

	static bool fold (CompAction         *action,
			  CompAction::State  state,
			  CompOption::Vector &options);

    public:

	int       mInvert;
	int       mXRotations;
	PaintOrder mPaintOrder;

	CubeScreen::RotationState mRotationState;

	bool mPaintAllViewports;

	GLfloat  mDistance;
	GLfloat  mTc[12];

	CompScreen::GrabHandle mGrabIndex;

	int mSrcOutput;

	bool    mUnfolded;
	GLfloat mUnfold, mUnfoldVelocity;

	GLfloat  *mVertices;
	int      mNVertices;

	GLuint mSkyListId;

	int		 mPw, mPh;
	CompSize mSkySize;
	GLTexture::List mTexture, mSky;

	int	mImgCurFile;

	int mNOutput;
	int mOutput[64];
	int mOutputMask[64];

	bool mCleared[64];

	bool mCapsPainted[64];

	bool mFullscreenOutput;

	float mOutputXScale;
	float mOutputYScale;
	float mOutputXOffset;
	float mOutputYOffset;

	float mDesktopOpacity;
	float mToOpacity;
	int   mLastOpacityIndex;

	bool mRecalcOutput;
	
	CompWindowList mReversedWindowList;
};

class PrivateCubeWindow;
extern template class PluginClassHandler<PrivateCubeWindow, CompWindow, COMPIZ_CUBE_ABI>;

class PrivateCubeWindow :
    public PluginClassHandler<PrivateCubeWindow, CompWindow, COMPIZ_CUBE_ABI>,
    public GLWindowInterface
{
    public:

	PrivateCubeWindow (CompWindow *w);
	~PrivateCubeWindow ();

	bool glPaint (const GLWindowPaintAttrib &, const GLMatrix &,
                      const CompRegion &, unsigned int);

	CompWindow      *window;
	CompositeWindow *cWindow;
	GLWindow        *gWindow;

	CubeScreen      *cubeScreen;
};

#endif
