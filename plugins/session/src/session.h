/**
 *
 * Compiz session plugin
 *
 * session.c
 *
 * Copyright (c) 2008 Travis Watkins <amaranth@ubuntu.com>
 * Copyright (c) 2008 Danny Baumann <maniac@opencompositing.org>
 * Copyright (c) 2006 Patrick Niklaus
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
 * Authors: Travis Watkins <amaranth@ubuntu.com>
 *          Patrick Niklaus
 **/

#include <core/core.h>
#include <core/pluginclasshandler.h>

#include <X11/Xatom.h>
#include <X11/SM/SM.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xmlsave.h>

#include <iostream>
#include <fstream>

#include "session_options.h"

typedef struct
{
    CompString clientId;
    CompString title;
    CompString resName;
    CompString resClass;
    CompString role;
    CompString command;

    bool     geometrySet;
    CompRect geometry;

    unsigned int state;
    bool         minimized;
    int          workspace;
} SessionItem;

class SessionScreen :
    public ScreenInterface,
    public PluginClassHandler<SessionScreen, CompScreen>,
    public SessionOptions
{
    public:
	SessionScreen (CompScreen *);

	void handleEvent (XEvent *);
	void sessionEvent (CompSession::Event, CompOption::Vector &);

	bool readWindow (CompWindow *);

    private:
	bool getUtf8Property (Window, Atom, CompString&);
	bool getTextProperty (Window, Atom, CompString&);
	bool getWindowTitle (Window, CompString&);
	bool getWindowClass (Window, CompString&, CompString&);
	bool getIsEmbedded (Window);
	bool getClientLeaderProperty (CompWindow *, Atom, CompString&);

	int getIntForProp (xmlNodePtr, const char *);
	CompString getStringForProp (xmlNodePtr, const char *);

	bool isSessionWindow (CompWindow *);
	void addWindowNode (CompWindow *, xmlNodePtr);

	void saveState (const CompString &);
	void readState (xmlNodePtr);
	void loadState (const CompString &);

	bool matchWindowClass (CompWindow *, const SessionItem&);

	CompString getFileName (const CompString &);
	bool createDir (const CompString&);
	
	Atom visibleNameAtom;
	Atom clientIdAtom;
	Atom embedInfoAtom;
	Atom roleAtom;
	Atom commandAtom;

	typedef std::list<SessionItem> ItemList;

	ItemList     items;
	std::fstream file;
};

class SessionWindow :
    public WindowInterface,
    public PluginClassHandler<SessionWindow, CompWindow>
{
    public:
	SessionWindow (CompWindow *);

	bool place (CompPoint &);

	CompWindow *window;
	bool       positionSet;
	CompPoint  position;
};

class SessionPluginVTable :
    public CompPlugin::VTableForScreenAndWindow<SessionScreen, SessionWindow>
{
    public:
	bool init ();
};
