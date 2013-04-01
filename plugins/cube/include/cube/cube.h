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

#ifndef _COMPIZ_CUBE_H
#define _COMPIZ_CUBE_H

#include <core/screen.h>
#include <core/output.h>
#include <core/pluginclasshandler.h>

#include <composite/composite.h>
#include <opengl/opengl.h>
#include <opengl/vector.h>
#include <opengl/texture.h>

#define COMPIZ_CUBE_ABI 2

typedef enum {
    BTF = 0,
    FTB
} PaintOrder;

class CubeScreen;
class PrivateCubeScreen;

class CubeScreenInterface :
    public WrapableInterface<CubeScreen, CubeScreenInterface>
{
    public:

	/**
	 * Hookable function to get the current state of rotation
	 *
	 * @param x X rotation
	 * @param v Y Rotation
	 * @param progress
	 */
	virtual void cubeGetRotation (float &x, float &v, float &progress);
	virtual void cubeClearTargetOutput (float xRotate, float vRotate);
	virtual void cubePaintTop (const GLScreenPaintAttrib &sAttrib,
				   const GLMatrix            &transform,
				   CompOutput                *output,
				   int                       size,
				   const GLVector            &normal);
	virtual void cubePaintBottom (const GLScreenPaintAttrib &sAttrib,
				      const GLMatrix            &transform,
				      CompOutput                *output,
				      int                       size,
				      const GLVector            &normal);
	virtual void cubePaintInside (const GLScreenPaintAttrib &sAttrib,
				      const GLMatrix            &transform,
				      CompOutput                *output,
				      int                       size,
				      const GLVector            &normal);
	virtual bool cubeCheckOrientation (const GLScreenPaintAttrib &sAttrib,
					   const GLMatrix            &transform,
					   CompOutput                *output,
					   std::vector<GLVector>     &points);
	virtual void cubePaintViewport (const GLScreenPaintAttrib &sAttrib,
					const GLMatrix            &transform,
					const CompRegion          &region,
					CompOutput                *output,
					unsigned int              mask);
	virtual bool cubeShouldPaintViewport (const GLScreenPaintAttrib &sAttrib,
					      const GLMatrix            &transform,
					      CompOutput                *output,
					      PaintOrder                order);
	virtual bool cubeShouldPaintAllViewports ();

};

extern template class PluginClassHandler<CubeScreen, CompScreen, COMPIZ_CUBE_ABI>;

class CubeScreen :
    public WrapableHandler<CubeScreenInterface, 9>,
    public PluginClassHandler<CubeScreen, CompScreen, COMPIZ_CUBE_ABI>,
    public CompOption::Class
{
    public:

	typedef enum {
	    RotationNone = 0,
	    RotationChange,
	    RotationManual
	} RotationState;
	
	typedef enum {
	    Automatic = 0,
	    MultipleCubes,
	    OneBigCube
	} MultioutputMode;
	

	CubeScreen (CompScreen *s);
	~CubeScreen ();
	
	CompOption::Vector & getOptions ();
	bool setOption (const CompString &name, CompOption::Value &value);

	WRAPABLE_HND (0, CubeScreenInterface, void, cubeGetRotation,
		      float &, float&, float&);
	WRAPABLE_HND (1, CubeScreenInterface, void, cubeClearTargetOutput,
		      float, float);
	WRAPABLE_HND (2, CubeScreenInterface, void, cubePaintTop,
		      const GLScreenPaintAttrib &, const GLMatrix &,
		      CompOutput *, int, const GLVector &);
	WRAPABLE_HND (3, CubeScreenInterface, void, cubePaintBottom,
		      const GLScreenPaintAttrib &, const GLMatrix &,
		      CompOutput *, int, const GLVector &);
	WRAPABLE_HND (4, CubeScreenInterface, void, cubePaintInside,
		      const GLScreenPaintAttrib &, const GLMatrix &,
		      CompOutput *, int, const GLVector &);
	WRAPABLE_HND (5, CubeScreenInterface, bool, cubeCheckOrientation,
		      const GLScreenPaintAttrib &, const GLMatrix &,
		      CompOutput *, std::vector<GLVector> &);
	WRAPABLE_HND (6, CubeScreenInterface, void, cubePaintViewport,
		      const GLScreenPaintAttrib &, const GLMatrix &,
		      const CompRegion &, CompOutput *, unsigned int);
	WRAPABLE_HND (7, CubeScreenInterface, bool, cubeShouldPaintViewport,
		      const GLScreenPaintAttrib &, const GLMatrix &,
		      CompOutput *, PaintOrder);
	WRAPABLE_HND (8, CubeScreenInterface, bool, cubeShouldPaintAllViewports);
	
	int invert () const;

	unsigned short* topColor () const;
	unsigned short* bottomColor () const;
	
	bool unfolded () const;
	
	RotationState rotationState () const;
	void rotationState (RotationState state);

	int xRotations () const;

	int nOutput () const;

	float outputXScale () const;
	float outputYScale () const;
	float outputXOffset () const;
	float outputYOffset () const;

	float distance () const;
	float desktopOpacity () const;
	
	MultioutputMode multioutputMode () const;

	int sourceOutput () const;
	
	PaintOrder paintOrder () const;
	
	void repaintCaps ();


	friend class PrivateCubeWindow;
	friend class PrivateCubeScreen;

    private:
	PrivateCubeScreen *priv;
};

#define CUBE_SCREEN(s) \
    CubeScreen *cs = CubeScreen::get (s)

#endif
