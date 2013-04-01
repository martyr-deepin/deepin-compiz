/**
 *
 * Compiz group plugin
 *
 * queues.cpp
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

#include "group.h"

/*
 * functions enqueuing pending notifies
 *
 */

/* forward declaration */

void
GroupWindow::enqueueMoveNotify (int  dx,
				     int  dy,
				     bool immediate,
				     bool sync)
{
    GroupWindow::PendingMoves *move;

    GROUP_SCREEN (screen);

    move = new GroupWindow::PendingMoves;
    if (!move)
	return;

    move->w  = window;
    move->dx = dx;
    move->dy = dy;

    move->immediate = immediate;
    move->sync      = sync;
    move->next      = NULL;

    if (gs->mPendingMoves)
    {
	GroupWindow::PendingMoves *temp;
	for (temp = gs->mPendingMoves; temp->next; temp = temp->next);

	temp->next = move;
    }
    else
	gs->mPendingMoves = move;

    if (!gs->mDequeueTimeoutHandle.active ())
    {
	gs->mDequeueTimeoutHandle.start ();
    }
}

void
GroupScreen::dequeueSyncs (GroupWindow::PendingSyncs *syncs)
{
    GroupWindow::PendingSyncs *sync;

    while (syncs)
    {
	sync = syncs;
	syncs = sync->next;
	
	GROUP_WINDOW (sync->w);
	if (gw->mNeedsPosSync)
	{
	    sync->w->syncPosition ();
	    gw->mNeedsPosSync = false;
	}

	delete sync;
    }

}

void
GroupScreen::dequeueMoveNotifies ()
{
    GroupWindow::PendingMoves *move;
    GroupWindow::PendingSyncs *syncs = NULL, *sync;

    mQueued = true;

    while (mPendingMoves)
    {
	move = mPendingMoves;
	mPendingMoves = move->next;

	move->w->move (move->dx, move->dy, move->immediate);
	if (move->sync)
	{
	    sync = new GroupWindow::PendingSyncs;
	    if (sync)
	    {
		GROUP_WINDOW (move->w);

		gw->mNeedsPosSync = true;
		sync->w          = move->w;
		sync->next       = syncs;
		syncs            = sync;
	    }
	}
	delete move;
    }

    if (syncs)
    {
	dequeueSyncs (syncs);
    }

    mQueued = false;
}

void
GroupWindow::enqueueGrabNotify (int          x,
				     int          y,
				     unsigned int state,
				     unsigned int mask)
{
    GroupWindow::PendingGrabs *grab;
    
    GROUP_SCREEN (screen);

    grab = new GroupWindow::PendingGrabs;
    if (!grab)
	return;

    grab->w = window;
    grab->x = x;
    grab->y = y;

    grab->state = state;
    grab->mask  = mask;
    grab->next  = NULL;

    if (gs->mPendingGrabs)
    {
	GroupWindow::PendingGrabs *temp;
	for (temp = gs->mPendingGrabs; temp->next; temp = temp->next);

	temp->next = grab;
    }
    else
	gs->mPendingGrabs = grab;

    if (!gs->mDequeueTimeoutHandle.active ())
    {
	gs->mDequeueTimeoutHandle.start ();
    }
}

void
GroupScreen::dequeueGrabNotifies ()
{
    GroupWindow::PendingGrabs *grab;

    mQueued = true;

    while (mPendingGrabs)
    {
	grab = mPendingGrabs;
	mPendingGrabs = mPendingGrabs->next;

	grab->w->grabNotify (grab->x, grab->y,
			     grab->state, grab->mask);

	delete grab;
    }

    mQueued = false;
}

void
GroupWindow::enqueueUngrabNotify ()
{
    GroupWindow::PendingUngrabs *ungrab;

    GROUP_SCREEN (screen);

    ungrab = new GroupWindow::PendingUngrabs;

    if (!ungrab)
	return;

    ungrab->w    = window;
    ungrab->next = NULL;

    if (gs->mPendingUngrabs)
    {
	GroupWindow::PendingUngrabs *temp;
	for (temp = gs->mPendingUngrabs; temp->next; temp = temp->next);

	temp->next = ungrab;
    }
    else
	gs->mPendingUngrabs = ungrab;

    if (!gs->mDequeueTimeoutHandle.active ())
    {
	gs->mDequeueTimeoutHandle.start ();
    }
}

void
GroupScreen::dequeueUngrabNotifies ()
{
    GroupWindow::PendingUngrabs *ungrab;

    mQueued = true;

    while (mPendingUngrabs)
    {
	ungrab = mPendingUngrabs;
	mPendingUngrabs = mPendingUngrabs->next;

	ungrab->w->ungrabNotify ();

	delete ungrab;
    }

    mQueued = false;
}

bool
GroupScreen::dequeueTimer ()
{
    dequeueMoveNotifies ();
    dequeueGrabNotifies ();
    dequeueUngrabNotifies ();

    return false;
}
