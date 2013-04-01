/*
 *
 * Compiz title bar information extension plugin
 *
 * titleinfo.cpp
 *
 * Copyright : (C) 2009 by Danny Baumann
 * E-mail    : maniac@compiz.org
 *
 * Ported to Compiz 0.9 by:
 * Copyright : (C) 2009 Sam Spilsbury
 * E-mail    : smspillaz@gmail.com
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

#include "titleinfo.h"


COMPIZ_PLUGIN_20090315 (titleinfo, TitleinfoPluginVTable);

void
TitleinfoWindow::updateVisibleName ()
{
    CompString  text, f_machine;
    CompString  root, f_title;

    TITLEINFO_SCREEN (screen);

    f_title = title.size () ? title : "";

    if (ts->optionGetShowRoot () && owner == 0)
	root = "ROOT: ";

    if (ts->optionGetShowRemoteMachine () && remoteMachine.size ())
    {
	char hostname[256];

	if (gethostname (hostname, 256) || strcmp (hostname, remoteMachine.c_str ()))
	    f_machine = remoteMachine;
    }

    if (f_machine.size ())
	text = root + f_title + "(@" + f_machine + ")";
    else if (root.size ())
	text = root + f_title;

    if (text.size ())
    {
	XChangeProperty (screen->dpy (), window->id (), ts->visibleNameAtom,
			 Atoms::utf8String, 8, PropModeReplace,
			 (unsigned char *) text.c_str (), text.size ());
	text.clear ();
    }
    else
    {
	XDeleteProperty (screen->dpy (), window->id (), ts->visibleNameAtom);
    }
}

void
TitleinfoWindow::updatePid ()
{
    int           pid = -1;
    Atom          type;
    int           result, format;
    unsigned long nItems, bytesAfter;
    unsigned char *propVal;

    TITLEINFO_SCREEN (screen);

    owner = -1;

    result = XGetWindowProperty (screen->dpy (), window->id (), ts->wmPidAtom,
				 0L, 1L, False, XA_CARDINAL, &type,
				 &format, &nItems, &bytesAfter, &propVal);

    if (result == Success && propVal)
    {
	if (nItems)
	{
	    unsigned long value;

	    memcpy (&value, propVal, sizeof (unsigned long));
	    pid = value;
	}

	XFree (propVal);
    }

    if (pid >= 0)
    {
	char        path[512];
	struct stat fileStat;

	snprintf (path, 512, "/proc/%d", pid);
	if (!lstat (path, &fileStat))
	    owner = fileStat.st_uid;
    }

    if (ts->optionGetShowRoot ())
	updateVisibleName ();
}

CompString
TitleinfoScreen::getUtf8Property (Window      id,
				  Atom        atom)
{
    Atom          type;
    int           result, format;
    unsigned long nItems, bytesAfter;
    char          *val = NULL,   *retval_c = NULL;
    CompString    retval;

    result = XGetWindowProperty (screen->dpy (), id, atom, 0L, 65536, False,
				 Atoms::utf8String, &type, &format, &nItems,
				 &bytesAfter, (unsigned char **) &val);

    if (result != Success)
	return retval;

    if (type == Atoms::utf8String && format == 8 && val && nItems > 0)
    {
	retval_c = (char *) malloc (sizeof (char) * (nItems + 1));
	if (retval_c)
	{
	    strncpy (retval_c, val, nItems);
	    retval_c[nItems] = 0;
	}
    }

    if (retval_c)
	retval = CompString (retval_c);

    if (val)
	XFree (val);

    return retval;
}

CompString
TitleinfoScreen::getTextProperty (Window      id,
				  Atom        atom)
{
    XTextProperty text;
    char          *retval_c = NULL;
    CompString    retval;

    text.nitems = 0;
    if (XGetTextProperty (screen->dpy (), id, &text, atom))
    {
        if (text.value)
	{
	    retval_c = (char *) malloc (sizeof (char) * (text.nitems + 1));
	    if (retval_c)
	    {
		strncpy (retval_c, (char *) text.value, text.nitems);
		retval_c[text.nitems] = 0;
	    }

	    XFree (text.value);
	}
    }

    if (retval_c)
	retval = CompString (retval_c);

    return retval;
}

void
TitleinfoWindow::updateTitle ()
{
    CompString f_title;

    TITLEINFO_SCREEN (screen);

    f_title = ts->getUtf8Property (window->id (), Atoms::wmName);

    if (f_title.empty ())
	title = ts->getTextProperty (window->id (), XA_WM_NAME);\

    title = f_title;
    updateVisibleName ();
}


void
TitleinfoWindow::updateMachine ()
{
    TITLEINFO_SCREEN (screen);

    if (remoteMachine.size ())
	remoteMachine.clear ();

    remoteMachine = ts->getTextProperty (window->id (),
					 XA_WM_CLIENT_MACHINE);

    if (ts->optionGetShowRemoteMachine ())
	updateVisibleName ();
}

void
TitleinfoScreen::addSupportedAtoms (std::vector<Atom> &atoms)
{
    screen->addSupportedAtoms (atoms);
    
    atoms.push_back (visibleNameAtom);
    atoms.push_back (wmPidAtom);
}

void
TitleinfoScreen::handleEvent (XEvent *event)
{

    screen->handleEvent (event);

    if (event->type == PropertyNotify)
    {
	CompWindow *w;

	if (event->xproperty.atom == XA_WM_CLIENT_MACHINE)
	{
	    w = screen->findWindow (event->xproperty.window);
	    if (w)
	    {
		TITLEINFO_WINDOW (w);
		tw->updateMachine ();
	    }
	}
	else if (event->xproperty.atom == wmPidAtom)
	{
	    w = screen->findWindow (event->xproperty.window);
	    if (w)
	    {
		TITLEINFO_WINDOW (w);
		tw->updatePid ();
	    }
	}
	else if (event->xproperty.atom == Atoms::wmName ||
		 event->xproperty.atom == XA_WM_NAME)
	{
	    w = screen->findWindow (event->xproperty.window);
	    if (w)
	    {
		TITLEINFO_WINDOW (w);
		tw->updateTitle ();
	    }
	}
    }
}

TitleinfoScreen::TitleinfoScreen (CompScreen *screen) :
    PluginClassHandler <TitleinfoScreen, CompScreen> (screen),
    visibleNameAtom (XInternAtom (screen->dpy (), "_NET_WM_VISIBLE_NAME", 0)),
    wmPidAtom (XInternAtom (screen->dpy (), "_NET_WM_PID", 0))
{
    ScreenInterface::setHandler (screen);
    
    screen->updateSupportedWmHints ();
};

TitleinfoScreen::~TitleinfoScreen ()
{
    screen->addSupportedAtomsSetEnabled (this, false);
    
    screen->updateSupportedWmHints ();
}

TitleinfoWindow::TitleinfoWindow (CompWindow *window) :
    PluginClassHandler <TitleinfoWindow, CompWindow> (window),
    window (window),
    owner (-1)
{
    updateTitle ();
    updateMachine ();
    updatePid ();
    updateVisibleName ();
}

bool
TitleinfoPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
	return false;

    return true;
};
