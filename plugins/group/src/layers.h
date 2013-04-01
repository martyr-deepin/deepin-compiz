/**
 *
 * Compiz group plugin
 *
 * layers.h
 *
 * Copyright : (C) 2006-2010 by Patrick Niklaus, Roi Cohen,
 * 				Danny Baumann, Sam Spilsbury
 * Authors: Patrick Niklaus <patrick.niklaus@googlemail.com>
 *          Roi Cohen       <roico.beryl@gmail.com>
 *          Danny Baumann   <maniac@opencompositing.org>
 * 	    Sam Spilsbury   <smspillaz@gmail.com>
 *
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
 **/

#ifndef _GROUP_LAYERS_H
#define _GROUP_LAYERS_H

#include "group.h"

typedef enum {
    PaintOff = 0,
    PaintFadeIn,
    PaintFadeOut,
    PaintOn,
    PaintPermanentOn
} PaintState;

class Layer :
    public CompSize
{
    public:
	Layer (const CompSize &size, GroupSelection *g) :
	    CompSize::CompSize (size),
	    mGroup (g),
	    mState (PaintOff),
	    mAnimationTime (0) {};
	virtual ~Layer () {}
	virtual void damage () {};

	GroupSelection  *mGroup;
	PaintState      mState;
	int             mAnimationTime;
};

class GLLayer :
    public Layer
{
    public:
	GLLayer (const CompSize &size, GroupSelection *g) :
	    Layer::Layer (size, g) {}

    public:

	virtual ~GLLayer () {}

	virtual void paint (const GLWindowPaintAttrib &attrib,
			    const GLMatrix	      &transform,
			    const CompRegion	      &paintRegion,
			    const CompRegion	      &clipRegion,
			    int			      mask) = 0;

};

class TextureLayer :
    public GLLayer
{
    public:
	TextureLayer (const CompSize &size, GroupSelection *g) :
	    GLLayer::GLLayer (size, g),
	    mPaintWindow (NULL) {}

    public:

	void setPaintWindow (CompWindow *);
	virtual void paint (const GLWindowPaintAttrib &attrib,
			    const GLMatrix	      &transform,
			    const CompRegion	      &paintRegion,
			    const CompRegion	      &clipRegion,
			    int			      mask);
    public:

	GLTexture::List mTexture;
	CompWindow	*mPaintWindow; /* the window we are going to
					* paint with geometry */

};

class CairoLayer :
    public TextureLayer
{
    public:

	virtual ~CairoLayer ();

    public:

	void clear ();
	virtual void render () = 0;
	virtual void paint (const GLWindowPaintAttrib &attrib,
			    const GLMatrix	      &transform,
			    const CompRegion	      &paintRegion,
			    const CompRegion	      &clipRegion,
			    int			      mask) = 0;

    public:

	/* used if layer is used for cairo drawing */
	unsigned char   *mBuffer;
	cairo_surface_t *mSurface;
	cairo_t	        *mCairo;
	bool	        mFailed;

    protected:
	CairoLayer (const CompSize &size, GroupSelection *group);
};

class BackgroundLayer :
    public CairoLayer
{
    public:

	typedef enum {
	    AnimationNone = 0,
	    AnimationPulse,
	    AnimationReflex
	} AnimationType;

    public:

	virtual ~BackgroundLayer () {}

	static BackgroundLayer * create (CompSize, GroupSelection *);
	static BackgroundLayer * rebuild (BackgroundLayer *,
				     CompSize);

	void render ();
	void paint (const GLWindowPaintAttrib &attrib,
		    const GLMatrix	      &transform,
		    const CompRegion	      &paintRegion,
		    const CompRegion	      &clipRegion,
		    int			      mask);

	bool handleAnimation (int msSinceLastPaint);

    public:

	/* For animations */
	int           mBgAnimationTime;
	AnimationType mBgAnimation;

    private:
	BackgroundLayer (const CompSize &size, GroupSelection *group);
};

class SelectionLayer :
    public CairoLayer
{
    public:

	virtual ~SelectionLayer () {}

	static SelectionLayer * create (CompSize, GroupSelection *);
	static SelectionLayer * rebuild (SelectionLayer *,
					 CompSize);

	void render ();
	void paint (const GLWindowPaintAttrib &attrib,
		    const GLMatrix	      &transform,
		    const CompRegion	      &paintRegion,
		    const CompRegion	      &clipRegion,
		    int			      mask);

    private:
	SelectionLayer (const CompSize &size, GroupSelection *group) :
	    CairoLayer::CairoLayer (size, group) {}
};

class TextLayer :
    public TextureLayer
{
    public:

	virtual ~TextLayer () {}

	static TextLayer *
	create (CompSize &, GroupSelection *);

	static TextLayer *
	rebuild (TextLayer *);

	void paint (const GLWindowPaintAttrib &attrib,
		    const GLMatrix	      &transform,
		    const CompRegion	      &paintRegion,
		    const CompRegion	      &clipRegion,
		    int			      mask);

	void render ();

    private:

	TextLayer (const CompSize &size, GroupSelection *g) :
	    TextureLayer::TextureLayer (size, g),
	    mPixmap (None) {}

    public:

	/* used if layer is used for text drawing */
	Pixmap mPixmap;
};

#endif
