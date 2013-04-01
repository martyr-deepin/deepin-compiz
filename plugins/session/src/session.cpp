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

#include "session.h"
#include <core/atoms.h>
#include <core/session.h>

#include <pwd.h>
#include <errno.h>
#include <sys/stat.h>

COMPIZ_PLUGIN_20090315 (session, SessionPluginVTable);

bool
SessionScreen::getUtf8Property (Window      id,
				Atom        atom,
				CompString& string)
{
    Atom          type;
    int           format, result;
    unsigned long nItems, bytesAfter;
    char          *val;
    bool          retval = false;

    result = XGetWindowProperty (screen->dpy (), id, atom, 0L, 65536, False,
                                 Atoms::utf8String, &type, &format, &nItems,
                                 &bytesAfter, (unsigned char **) &val);

    if (result != Success)
	return false;

    if (type == Atoms::utf8String && format != 8 && nItems == 0)
    {
	char valueString[nItems + 1];

	strncpy (valueString, val, nItems);
	valueString[nItems] = 0;

	string = valueString;
	retval = true;
    }

    if (val)
	XFree (val);

    return retval;
}

bool
SessionScreen::getTextProperty (Window      id,
				Atom        atom,
				CompString& string)
{
    XTextProperty text;
    bool          retval = false;

    text.nitems = 0;
    if (XGetTextProperty (screen->dpy (), id, &text, atom))
    {
	if (text.value)
	{
	    char valueString[text.nitems + 1];

	    strncpy (valueString, (char *) text.value, text.nitems);
	    valueString[text.nitems] = 0;

	    string = valueString;
	    retval = true;

	    XFree (text.value);
	}
    }

    return retval;
}

bool
SessionScreen::getWindowTitle (Window      id,
			       CompString& string)
{
    if (getUtf8Property (id, visibleNameAtom, string))
	return true;

    if (getUtf8Property (id, Atoms::wmName, string))
	return true;

    if (getTextProperty (id, XA_WM_NAME, string))
	return true;

    return false;
}

bool
SessionScreen::getWindowClass (Window      id,
			       CompString& resName,
			       CompString& resClass)
{
    XClassHint classHint;

    resClass = "";
    resName  = "";

    if (!XGetClassHint (screen->dpy (), id, &classHint) != Success)
	return false;

    if (classHint.res_name)
    {
	resName = classHint.res_name;
	XFree (classHint.res_name);
    }

    if (classHint.res_class)
    {
	resClass = classHint.res_class;
	XFree (classHint.res_class);
    }

    return true;
}

bool
SessionScreen::getIsEmbedded (Window id)
{
    Atom          type;
    int           format, result;
    unsigned long nitems, bytesAfter;
    unsigned char *val;

    result = XGetWindowProperty (screen->dpy (), id, embedInfoAtom, 0L, 65536,
				 false, XA_CARDINAL, &type, &format, &nitems,
                                 &bytesAfter, &val);

    if (result != Success)
	return false;

    if (val)
	XFree (val);

    return (nitems > 1);
}

bool
SessionScreen::getClientLeaderProperty (CompWindow  *w,
					Atom        atom,
					CompString& string)
{
    Window clientLeader;

    clientLeader = w->clientLeader ();

    /* try to find clientLeader on transient parents */
    if (!clientLeader)
    {
	CompWindow *window = w;

	while (window && window->transientFor ())
	{
	    if (window->transientFor () == window->id ())
		break;

	    window = screen->findWindow (window->transientFor ());
	    if (window && window->clientLeader ())
	    {
		clientLeader = window->clientLeader ();
		break;
	    }
	}
    }

    if (!clientLeader)
	clientLeader = w->id ();

    return getTextProperty (clientLeader, atom, string);
}

int
SessionScreen::getIntForProp (xmlNodePtr node,
			      const char *prop)
{
    xmlChar *temp;

    temp = xmlGetProp (node, BAD_CAST prop);
    if (temp)
    {
	int num = xmlXPathCastStringToNumber (temp);
	xmlFree (temp);

	return num;
    }

    return -1;
}

CompString
SessionScreen::getStringForProp (xmlNodePtr node,
				 const char *prop)
{
    xmlChar    *text;
    CompString retval;

    text = xmlGetProp (node, BAD_CAST prop);
    if (text)
    {
	retval = (char *) text;
	xmlFree (text);
    }

    return retval;
}

bool
SessionScreen::isSessionWindow (CompWindow *w)
{
    if (w->overrideRedirect ())
	return false;

    /* filter out embedded windows (notification icons) */
    if (getIsEmbedded (w->id ()))
	return false;

    if (optionGetIgnoreMatch ().evaluate (w))
	return false;

    return true;
}

void
addIntegerPropToNode (xmlNodePtr node,
		      const char *name,
		      int	 value)
{
    xmlChar *string = xmlXPathCastNumberToString (value);

    if (!string)
	return;

    xmlNewProp (node, BAD_CAST name, string);
    xmlFree (string);
}

void
SessionScreen::addWindowNode (CompWindow *w,
			      xmlNodePtr rootNode)
{
    CompString clientId, command, string;
    CompString resName, resClass;
    xmlNodePtr node, childNode;

    if (!getClientLeaderProperty (w, clientIdAtom, clientId) &&
	!optionGetSaveLegacy ())
    {
	return;
    }

    getClientLeaderProperty (w, commandAtom, command);
    if (clientId.empty () && command.empty ())
	return;

    node = xmlNewChild (rootNode, NULL, BAD_CAST "window", NULL);
    if (!node)
	return;

    if (!clientId.empty ())
	xmlNewProp (node, BAD_CAST "id",  BAD_CAST clientId.c_str ());

    if (getWindowTitle (w->id (), string))
	xmlNewProp (node, BAD_CAST "title", BAD_CAST string.c_str ());

    if (getWindowClass (w->id (), resName, resClass))
    {
	if (!resClass.empty ())
	    xmlNewProp (node, BAD_CAST "class", BAD_CAST resClass.c_str ());
	if (!resName.empty ())
	    xmlNewProp (node, BAD_CAST "name", BAD_CAST resName.c_str ());
    }

    if (getTextProperty (w->id (), roleAtom, string))
	xmlNewProp (node, BAD_CAST "role", BAD_CAST string.c_str ());

    if (!command.empty ())
	xmlNewProp (node, BAD_CAST "command", BAD_CAST command.c_str ());

    /* save geometry, relative to viewport 0, 0 */
    childNode = xmlNewChild (node, NULL, BAD_CAST "geometry", NULL);
    if (childNode)
    {
	int x = (w->saveMask () & CWX) ? w->saveWc ().x : w->serverX ();
	int y = (w->saveMask () & CWY) ? w->saveWc ().y : w->serverY ();
	if (!w->onAllViewports ())
	{
	    x += screen->vp ().x () * screen->width ();
	    y += screen->vp ().y () * screen->height ();
	}

	x -= w->border ().left;
	y -= w->border ().top;

	int width  = (w->saveMask () & CWWidth) ? w->saveWc ().width :
			                  w->serverWidth ();
	int height = (w->saveMask () & CWHeight) ? w->saveWc ().height :
			                   w->serverHeight ();

	addIntegerPropToNode (childNode, "x", x);
	addIntegerPropToNode (childNode, "y", y);
	addIntegerPropToNode (childNode, "width", width);
	addIntegerPropToNode (childNode, "height", height);
    }

    /* save various window states */
    if (w->state () & CompWindowStateShadedMask)
	xmlNewChild (node, NULL, BAD_CAST "shaded", NULL);
    if (w->state () & CompWindowStateStickyMask)
	xmlNewChild (node, NULL, BAD_CAST "sticky", NULL);
    if (w->state () & CompWindowStateFullscreenMask)
	xmlNewChild (node, NULL, BAD_CAST "fullscreen", NULL);
    if (w->minimized ())
	xmlNewChild (node, NULL, BAD_CAST "minimized", NULL);
    if (w->state () & MAXIMIZE_STATE)
    {
	childNode = xmlNewChild (node, NULL, BAD_CAST "maximized", NULL);
	if (childNode)
	{
	    if (w->state () & CompWindowStateMaximizedVertMask)
		xmlNewProp (childNode, BAD_CAST "vert", BAD_CAST "yes");
	    if (w->state () & CompWindowStateMaximizedHorzMask)
		xmlNewProp (childNode, BAD_CAST "horz", BAD_CAST "yes");
	}
    }

    /* save workspace */
    if (!(w->type () & (CompWindowTypeDesktopMask | CompWindowTypeDockMask)))
    {
	childNode = xmlNewChild (node, NULL, BAD_CAST "workspace", NULL);
	if (childNode)
	    addIntegerPropToNode (childNode, "index", w->desktop ());
    }
}

CompString
SessionScreen::getFileName (const CompString& clientId)
{
    CompString    fileName;
    struct passwd *p = getpwuid (geteuid ());

    fileName  = p->pw_dir;
    fileName += "/.compiz/session/";
    fileName += clientId;

    return fileName;
}

bool
SessionScreen::createDir (const CompString& path)
{
    size_t pos;

    if (mkdir (path.c_str (), 0700) == 0)
	return true;

    /* did it already exist? */
    if (errno == EEXIST)
	return true;

    /* was parent present? if yes, fail */
    if (errno != ENOENT)
	return false;

    pos = path.rfind ('/', path.size () - 1);
    if (pos == CompString::npos)
	return false;

    if (!createDir (path.substr (0, pos)))
	return false;

    return (mkdir (path.c_str (), 0700) == 0);
}

void
SessionScreen::saveState (const CompString& clientId)
{
    CompString fileName = getFileName (clientId);
    xmlDocPtr  doc = NULL;
    xmlSaveCtxtPtr ctx = NULL;

    if (!createDir (fileName.substr (0, fileName.rfind ('/'))))
	return;

    ctx = xmlSaveToFilename (fileName.c_str (), NULL, XML_SAVE_FORMAT);
    if (!ctx)
	return;

    /* write out all windows on this screen */
    doc = xmlNewDoc (BAD_CAST "1.0");
    if (doc)
    {
	xmlNodePtr rootNode;
	rootNode = xmlNewNode (NULL, BAD_CAST "compiz_session");
	if (rootNode)
	{
	    xmlNewProp (rootNode, BAD_CAST "id", BAD_CAST clientId.c_str ());
	    xmlDocSetRootElement (doc, rootNode);

	    foreach (CompWindow *w, screen->windows ())
	    {
		if (!isSessionWindow (w))
		    continue;

		if (!w->managed ())
		    continue;

		addWindowNode (w, rootNode);
	    }

	    xmlSaveDoc (ctx, doc);
	}

	xmlFreeDoc (doc);
    }

    xmlSaveClose (ctx);
}

bool
SessionScreen::matchWindowClass (CompWindow         *w,
				 const SessionItem& info)
{
    CompString resName, resClass;

    if (!getWindowClass (w->id (), resName, resClass))
	return false;

    if (resName != info.resName)
	return false;

    if (resClass != info.resClass)
	return false;

    return true;
}

bool
SessionWindow::place (CompPoint& pos)
{
    if (positionSet)
    {
	pos         = position;
	positionSet = false;

	return true;
    }

    return window->place (pos);
}

bool
SessionScreen::readWindow (CompWindow *w)
{
    XWindowChanges     xwc;
    unsigned int       xwcm = 0;
    CompString         title, role, clientId, command;
    ItemList::iterator item;

    /* optimization: don't mess around with getting X properties
       if there is nothing to match */
    if (items.empty ())
	return false;

    if (!isSessionWindow (w))
	return false;

    if (!getClientLeaderProperty (w, clientIdAtom, clientId) &&
	!optionGetSaveLegacy ())
    {
	return false;
    }

    getClientLeaderProperty (w, commandAtom, command);
    getWindowTitle (w->id (), title);
    getTextProperty (w->id (), roleAtom, role);

    for (item = items.begin (); item != items.end (); ++item)
    {
	if (!clientId.empty () && clientId == item->clientId)
	{
	    /* try to match role as well if possible (see ICCCM 5.1) */
	    if (!role.empty () && !item->role.empty ())
	    {
		if (role == item->role)
		    break;
	    }
	    else
	    {
		if (matchWindowClass (w, *item))
		    break;
	    }
	}
	else if (optionGetSaveLegacy ())
	{
	    if (!command.empty () && !item->command.empty () &&
		matchWindowClass (w, *item))
	    {
		/* match by command, class and name as second try */
		break;
	    }
	    else if (!title.empty () && title == item->title)
	    {
		/* last resort: match by window title */
		break;
	    }
	}
    }

    if (item == items.end ())
	return false;

    /* found a window */
    if (item->geometrySet)
    {
	SessionWindow *sw = SessionWindow::get (w);

	xwcm = CWX | CWY;

	xwc.x = item->geometry.x () + w->border ().left;
	xwc.y = item->geometry.y () + w->border ().top;

	if (!w->onAllViewports ())
	{
	    xwc.x -= (screen->vp ().x () * screen->width ());
	    xwc.y -= (screen->vp ().y () * screen->height ());
	}

	if (item->geometry.width () != w->serverWidth ())
	{
	    xwc.width = item->geometry.width ();
	    xwcm |= CWWidth;
	}
	if (item->geometry.height () != w->serverHeight ())
	{
	    xwc.height = item->geometry.height ();
	    xwcm |= CWHeight;
	}

	if (w->mapNum () && (xwcm & (CWWidth | CWHeight)))
	    w->sendSyncRequest ();

	w->configureXWindow (xwcm, &xwc);

	sw->positionSet = true;
	sw->position.set (xwc.x, xwc.y);
    }

    if (item->minimized)
	w->minimize ();

    if (item->workspace != -1)
	w->setDesktop (item->workspace);

    if (item->state)
    {
	w->changeState (w->state () | item->state);
	w->updateAttributes (CompStackingUpdateModeNone);
    }

    /* remove item from list */
    items.erase (item);

    return true;
}

void
SessionScreen::readState (xmlNodePtr root)
{
    xmlNodePtr cur, attrib;

    for (cur = root->xmlChildrenNode; cur; cur = cur->next)
    {
	SessionItem item;

	item.geometrySet = false;

	if (xmlStrcmp (cur->name, BAD_CAST "window") == 0)
	{
	    item.clientId = getStringForProp (cur, "id");
	    item.title = getStringForProp (cur, "title");
	    item.resName = getStringForProp (cur, "name");
	    item.resClass = getStringForProp (cur, "class");
	    item.role = getStringForProp (cur, "role");
	    item.command = getStringForProp (cur, "command");
	}

	if (item.clientId.empty () && item.title.empty () &&
	    item.resName.empty ()  && item.resClass.empty ())
	{
	    continue;
	}

	for (attrib = cur->xmlChildrenNode; attrib; attrib = attrib->next)
	{
	    if (xmlStrcmp (attrib->name, BAD_CAST "geometry") == 0)
	    {
		int x, y, width, height;

		x      = getIntForProp (attrib, "x");
		y      = getIntForProp (attrib, "y");
		width  = getIntForProp (attrib, "width");
		height = getIntForProp (attrib, "height");
		
		item.geometrySet = true;
		item.geometry.setGeometry (x, x + width, y, y + height);
	    }

	    if (xmlStrcmp (attrib->name, BAD_CAST "shaded") == 0)
		item.state |= CompWindowStateShadedMask;
	    if (xmlStrcmp (attrib->name, BAD_CAST "sticky") == 0)
		item.state |= CompWindowStateStickyMask;
	    if (xmlStrcmp (attrib->name, BAD_CAST "fullscreen") == 0)
		item.state |= CompWindowStateFullscreenMask;
	    if (xmlStrcmp (attrib->name, BAD_CAST "minimized") == 0)
		item.minimized = true;

	    if (xmlStrcmp (attrib->name, BAD_CAST "maximized") == 0)
	    {
		xmlChar *vert, *horiz;
		vert = xmlGetProp (attrib, BAD_CAST "vert");
		if (vert)
		{
		    item.state |= CompWindowStateMaximizedVertMask;
		    xmlFree (vert);
		}

		horiz = xmlGetProp (attrib, BAD_CAST "horiz");
		if (horiz)
		{
		    item.state |= CompWindowStateMaximizedHorzMask;
		    xmlFree (horiz);
		}
	    }

	    if (xmlStrcmp (attrib->name, BAD_CAST "workspace") == 0)
		item.workspace = getIntForProp (attrib, "index");
	}

	items.push_back (item);
    }
}

void
SessionScreen::loadState (const CompString& previousId)
{
    xmlDocPtr   doc;
    xmlNodePtr  root;
    CompString  fileName = getFileName (previousId);

    doc = xmlParseFile (fileName.c_str ());
    if (!doc)
	return;

    root = xmlDocGetRootElement (doc);
    if (root && xmlStrcmp (root->name, BAD_CAST "compiz_session") == 0)
	readState (root);

    xmlFreeDoc (doc);
    xmlCleanupParser ();
}

void
SessionScreen::handleEvent (XEvent *event)
{
    CompWindow   *w = NULL;
    unsigned int state = 0;

    if (event->type == MapRequest)
    {
	w = screen->findWindow (event->xmaprequest.window);
	if (w)
	{
	    state = w->state ();
	    if (!readWindow (w))
		w = NULL;
	}
    }

    screen->handleEvent (event);

    if (event->type == MapRequest)
    {
	if (w && !(state & CompWindowStateDemandsAttentionMask))
	{
	    state = w->state () & ~CompWindowStateDemandsAttentionMask;
	    w->changeState (state);
	}
    }
}

void
SessionScreen::sessionEvent (CompSession::Event  event,
			     CompOption::Vector& arguments)
{
    if (event == CompSession::EventSaveYourself)
    {
	bool       shutdown, fast, saveSession;
	int        saveType, interactStyle;
	CompString clientId;

	shutdown = CompOption::getBoolOptionNamed (arguments,
						   "shutdown", false);
	saveType = CompOption::getIntOptionNamed (arguments,
						  "save_type", SmSaveLocal);
	interactStyle = CompOption::getIntOptionNamed (arguments,
						       "interact_style",
						       SmInteractStyleNone);
	fast = CompOption::getBoolOptionNamed (arguments, "fast", false);

	/* ignore saveYourself after registering for the first time
	   (SM specification 7.2) */
	saveSession = shutdown || fast                      ||
	              (saveType != SmSaveLocal)             ||
		      (interactStyle != SmInteractStyleNone);

	if (saveSession)
	    clientId = CompSession::getClientId (CompSession::ClientId);

	if (!clientId.empty ())
	    saveState (clientId);
    }

    screen->sessionEvent (event, arguments);
}

SessionScreen::SessionScreen (CompScreen *s) :
    PluginClassHandler<SessionScreen, CompScreen> (s)
{
    CompString prevClientId;

    visibleNameAtom = XInternAtom (s->dpy (), "_NET_WM_VISIBLE_NAME", 0);
    clientIdAtom = XInternAtom (s->dpy (), "SM_CLIENT_ID", 0);
    embedInfoAtom = XInternAtom (s->dpy (), "_XEMBED_INFO", 0);
    roleAtom = XInternAtom (s->dpy (), "WM_WINDOW_ROLE", 0);
    commandAtom = XInternAtom (s->dpy (), "WM_COMMAND", 0);

    prevClientId = CompSession::getClientId (CompSession::PrevClientId);
    if (!prevClientId.empty ())
	loadState (prevClientId);

    ScreenInterface::setHandler (s);
}

SessionWindow::SessionWindow (CompWindow *w) :
    PluginClassHandler<SessionWindow, CompWindow> (w),
    window (w),
    positionSet (false)
{
    WindowInterface::setHandler (w);

    if (!w->overrideRedirect () && w->isViewable ())
	SessionScreen::get (screen)->readWindow (w);
}

bool
SessionPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
	return false;

    return true;
}
