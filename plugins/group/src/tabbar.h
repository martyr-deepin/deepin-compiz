/**
 *
 * Compiz group plugin
 *
 * tabbar.h
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

#ifndef GROUP_TABBAR_H
#define GROUP_TABBAR_H

#include "group.h"

/* Mask values for groupTabSetVisibility */
#define SHOW_BAR_INSTANTLY_MASK (1 << 0)
#define PERMANENT		(1 << 1)

/* Mask values for tabbing animation */
#define IS_ANIMATED		(1 << 0)
#define FINISHED_ANIMATION	(1 << 1)
#define CONSTRAINED_X		(1 << 2)
#define CONSTRAINED_Y		(1 << 3)
#define DONT_CONSTRAIN		(1 << 4)
#define IS_UNGROUPING           (1 << 5)

class GroupTabBar;

/*
 * GroupTabBarSlot
 */
class GroupTabBarSlot :
    public GLLayer
{
public:
    class List :
        public std::list <GroupTabBarSlot *>,
	public GLLayer
    {
	public:
	    List (const CompSize &size, GroupSelection *g) :
	        GLLayer::GLLayer (size, g) {};
	
	    void paint (const GLWindowPaintAttrib &attrib,
		        const GLMatrix	          &transform,
			const CompRegion	  &region,
		        const CompRegion	  &clipRegion,
			int			  mask);
    };
public:

    virtual ~GroupTabBarSlot ();

    void getDrawOffset (int &hoffset,
			int &voffset);

    void setTargetOpacity (int);
    
    void paint (const GLWindowPaintAttrib &sa,
		const GLMatrix	          &transform,
		const CompRegion	  &paintRegion,
		const CompRegion	  &clipRegion,
		int 			  mask);

public:
    GroupTabBarSlot *mPrev;
    GroupTabBarSlot *mNext;

    CompRegion mRegion;

    CompWindow *mWindow;
    GroupTabBar *mTabBar;

    /* For DnD animations */
    int	  mSpringX;
    int	  mSpeed;
    float mMsSinceLastMove;
    int   mOpacity;
private:

    GroupTabBarSlot (CompWindow *, GroupTabBar *);
    
    friend class GroupTabBar;
};

/*
 * GroupTabBar
 */
class GroupTabBar
{
    public:

	typedef enum {
	    NoTabChange = 0,
	    TabChangeOldOut,
	    TabChangeNewIn
	} TabChangeState;

	/*
	 * Rotation direction for change tab animation
	 */
	typedef enum {
	    RotateUncertain = 0,
	    RotateLeft,
	    RotateRight
	} ChangeAnimationDirection;

    public:

	GroupTabBar (GroupSelection *, CompWindow *);
	~GroupTabBar ();

    public:

	/* Input Prevention */

	void createInputPreventionWindow ();
	void destroyInputPreventionWindow ();

	/* Drawing */

	void paint (const GLWindowPaintAttrib &attrib,
		    const GLMatrix		 &transform,
		    unsigned int		 mask,		
		    CompRegion		 clipRegion);

	void damageRegion ();

	/* Animation */

	bool handleTabBarFade (int msSinceLastPaint);
	bool handleTextFade (int msSinceLastPaint);

	/* Region and position management */

	void moveTabBarRegion (int		   dx,
			       int		   dy,
			       bool		   syncIPW);

	void resizeTabBarRegion (CompRect       &box,
			         bool           syncIPW);

	void recalcTabBarPos (int		  middleX,
			      int		  minX1,
			      int		  maxX2);

	/* Slot management */
	void insertTabBarSlotBefore (GroupTabBarSlot *slot,
				     GroupTabBarSlot *nextSlot);

	void insertTabBarSlotAfter (GroupTabBarSlot *slot,
				    GroupTabBarSlot *prevSlot);

	void insertTabBarSlot (GroupTabBarSlot *slot);

	void unhookTabBarSlot (GroupTabBarSlot *slot,
			       bool            temporary);

	void deleteTabBarSlot (GroupTabBarSlot *slot);

	void createSlot (CompWindow      *w);

	bool applyForces (GroupTabBarSlot *);

	void applySpeeds (int            msSinceLastRepaint);


    public:
	GroupTabBarSlot::List mSlots;

	GroupSelection  *mGroup;

	GroupTabBarSlot* mTopTab;
	GroupTabBarSlot* mPrevTopTab;

	/* needed for untabbing animation */
	CompWindow *mLastTopTab;

	/* Those two are only for the change-tab animation,
	when the tab was changed again during animation.
	Another animation should be started again,
	switching for this window. */
	GroupTabBar::ChangeAnimationDirection mNextDirection;
	GroupTabBarSlot             *mNextTopTab;

	/* check focus stealing prevention after changing tabs */
	bool mCheckFocusAfterTabChange;

	int            mChangeAnimationTime;
	int            mChangeAnimationDirection;
	GroupTabBar::TabChangeState mChangeState;

	GroupTabBarSlot *mHoveredSlot;
	GroupTabBarSlot *mTextSlot;

	TextLayer *mTextLayer;
	BackgroundLayer *mBgLayer;
	SelectionLayer *mSelectionLayer;

	PaintState mState;
	int        mAnimationTime;
	CompRegion mRegion;
	int        mOldWidth;

	CompTimer mTimeoutHandle;

	/* For DnD animations */
	int   mLeftSpringX, mRightSpringX;
	int   mLeftSpeed, mRightSpeed;
	float mLeftMsSinceLastMove, mRightMsSinceLastMove;

	Window mInputPrevention;
	bool   mIpwMapped;
};



#endif
