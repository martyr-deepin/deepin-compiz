/*
 * Compiz cube reflection and cylinder deformation plugin
 *
 * cubeaddon.h
 *
 * Copyright : (C) 2009 by Dennis Kasprzyk
 * E-mail    : onestone@opencompositing.org
 *
 * includes code from cubecaps.c
 *
 * Copyright : (C) 2007 Guillaume Seguin
 * E-mail    : guillaume@segu.in
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>

#include <core/core.h>
#include <core/pluginclasshandler.h>

#include <composite/composite.h>
#include <opengl/opengl.h>
#include <cube/cube.h>

#include "cubeaddon_options.h"

extern const unsigned short CUBEADDON_GRID_SIZE;
extern const unsigned short CAP_ELEMENTS;
extern const unsigned int   CAP_NVERTEX;
extern const unsigned int   CAP_NIDX;

extern const unsigned int   CAP_NIMGVERTEX;
extern const unsigned int   CAP_NIMGIDX;

extern const float RAD2I1024 = 162.9746617f;

class CubeaddonScreen :
    public CompositeScreenInterface,
    public GLScreenInterface,
    public CubeScreenInterface,
    public PluginClassHandler<CubeaddonScreen, CompScreen>,
    public CubeaddonOptions
{
    public:
	CubeaddonScreen (CompScreen *);
	~CubeaddonScreen ();
	
	bool setOption (const CompString &name, CompOption::Value &value);

	void donePaint ();

	bool glPaintOutput (const GLScreenPaintAttrib&, const GLMatrix&,
			    const CompRegion&, CompOutput *, unsigned int);
	void glPaintTransformedOutput (const GLScreenPaintAttrib&,
				       const GLMatrix&, const CompRegion&,
				       CompOutput *, unsigned int);


	void cubeGetRotation (float &x, float &v, float &progress);
	void cubeClearTargetOutput (float xRotate, float vRotate);
	void cubePaintTop (const GLScreenPaintAttrib &sAttrib,
			   const GLMatrix            &transform,
			   CompOutput                *output,
			   int                       size);
	void cubePaintBottom (const GLScreenPaintAttrib &sAttrib,
			      const GLMatrix            &transform,
			      CompOutput                *output,
			      int                       size);
	bool cubeCheckOrientation (const GLScreenPaintAttrib &sAttrib,
				   const GLMatrix            &transform,
				   CompOutput                *output,
				   std::vector<GLVector>     &points);
	bool cubeShouldPaintViewport (const GLScreenPaintAttrib &sAttrib,
				      const GLMatrix            &transform,
				      CompOutput                *output,
				      PaintOrder                order);
	
	bool cubeShouldPaintAllViewports ();

	class CubeCap
	{
	    public:
		CubeCap ();
		
		void load (bool scale, bool aspect, bool clamp);
		
		int                        mCurrent;
		CompOption::Value::Vector  mFiles;

		bool                       mLoaded;

		GLTexture::List            mTexture;
		GLMatrix                   mTexMat;
	};
	
	friend class CubeaddonWindow;
	
    private:
	bool changeCap (bool top, int change);
	void drawBasicGround ();
	void paintCap (const GLScreenPaintAttrib &sAttrib,
		       const GLMatrix            &transform,
		       CompOutput                *output,
		       int                       size,
		       bool                      top,
		       bool                      adjust);

    private:
	
	CompositeScreen *cScreen;
	GLScreen        *gScreen;
	CubeScreen      *cubeScreen;


	bool mReflection;
	bool mFirst;

	CompOutput *mLast;

	float mYTrans;
	float mZTrans;

	float mBackVRotate;
	float mVRot;

	float mDeform;
	bool  mWasDeformed;

	GLfloat      *mWinNormals;
	unsigned int mWinNormSize;

	GLfloat  mCapFill[CAP_NVERTEX];
	GLfloat  mCapFillNorm[CAP_NVERTEX];
	GLushort mCapFillIdx[CAP_NIDX];
	float    mCapDeform;
	float    mCapDistance;
	int      mCapDeformType;

	CubeCap mTopCap;
	CubeCap mBottomCap;
	
	float mSinT[1024];
	float mCosT[1024];
};

class CubeaddonWindow :
    public GLWindowInterface,
    public PluginClassHandler<CubeaddonWindow, CompWindow>
{
    public:
	CubeaddonWindow (CompWindow *);

	bool glDraw (const GLMatrix&, GLFragment::Attrib&,
		     const CompRegion&, unsigned int);
	void glAddGeometry (const GLTexture::MatrixList&,
			    const CompRegion&, const CompRegion&,
			    unsigned int, unsigned int);
	void glDrawTexture (GLTexture *, GLFragment::Attrib& attrib,
			    unsigned int);

	CompWindow      *window;
	GLWindow        *gWindow;
	CubeaddonScreen *caScreen;
	CubeScreen      *cubeScreen;
};

class CubeaddonPluginVTable :
    public CompPlugin::VTableForScreenAndWindow<CubeaddonScreen, CubeaddonWindow>
{
    public:
	bool init ();
};
