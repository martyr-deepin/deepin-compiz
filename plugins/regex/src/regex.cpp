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

#include "regexplugin.h"

#include "core/atoms.h"

#include <regex.h>
#include <limits.h>

COMPIZ_PLUGIN_20090315 (regex, RegexPluginVTable)

class RegexExp : public CompMatch::Expression
{
    public:
	typedef enum {
	    TypeTitle,
	    TypeRole,
	    TypeClass,
	    TypeName,
	} Type;

	RegexExp (const CompString& str, int item);
	virtual ~RegexExp ();

	bool evaluate (CompWindow *w);
	static int matches (const CompString& str);

    private:
	typedef struct {
	    const char   *name;
	    size_t       length;
	    Type         type;
	    unsigned int flags;
	} Prefix;

	static const Prefix prefix[];

	Type    mType;
	regex_t *mRegex;
};

const RegexExp::Prefix RegexExp::prefix[] = {
    { "title=", 6, TypeTitle, 0 },
    { "role=",  5, TypeRole, 0  },
    { "class=", 6, TypeClass, 0 },
    { "name=",  5, TypeName, 0  },
    { "ititle=", 7, TypeTitle, REG_ICASE },
    { "irole=",  6, TypeRole, REG_ICASE  },
    { "iclass=", 7, TypeClass, REG_ICASE },
    { "iname=",  6, TypeName, REG_ICASE  }
};

RegexExp::RegexExp (const CompString& str, int item) :
    mRegex (NULL)
{
    if ((unsigned int) item < sizeof (prefix) / sizeof (prefix[0]))
    {
	int        status;
	CompString value;

	value  = str.substr (prefix[item].length);
	mRegex = new regex_t;
	status = regcomp (mRegex, value.c_str (),
			  REG_NOSUB | prefix[item].flags);

	if (status)
	{
	    char errMsg[1024];

	    regerror (status, mRegex, errMsg, sizeof (errMsg));

	    compLogMessage ("regex", CompLogLevelWarn,
			    "%s = %s", errMsg, value.c_str ());

	    regfree (mRegex);
	    delete mRegex;
	    mRegex = NULL;
	}

	mType = prefix[item].type;
    }
}

RegexExp::~RegexExp ()
{
    if (mRegex)
    {
	regfree (mRegex);
	delete mRegex;
    }
}

bool
RegexExp::evaluate (CompWindow *w)
{
    CompString  *string = NULL;
    RegexWindow *rw = RegexWindow::get (w);

    switch (mType)
    {
	case TypeRole:
	    string = &rw->role;
	    break;
	case TypeTitle:
	    string = &rw->title;
	    break;
	case TypeClass:
	    string = &rw->resClass;
	    break;
	case TypeName:
	    string = &rw->resName;
	    break;
    }

    if (!mRegex || !string)
	return false;

    if (regexec (mRegex, string->c_str (), 0, NULL, 0))
	return false;

    return true;
}

int
RegexExp::matches (const CompString& str)
{
    unsigned int i;

    for (i = 0; i < sizeof (prefix) / sizeof (prefix[0]); i++)
	if (str.compare (0, prefix[i].length, prefix[i].name) == 0)
	    return (int) i;

    return -1;
}

CompMatch::Expression *
RegexScreen::matchInitExp (const CompString& str)
{
    int item = RegexExp::matches (str);

    if (item >= 0)
	return new RegexExp (str, item);

    return screen->matchInitExp (str);
}

bool
RegexWindow::getStringProperty (Atom        nameAtom,
				Atom        typeAtom,
				CompString& string)
{
    Atom	  type;
    unsigned long nItems;
    unsigned long bytesAfter;
    unsigned char *str = NULL;
    int		  format, result;

    result = XGetWindowProperty (screen->dpy (), window->id (), nameAtom, 0,
				 LONG_MAX, false, typeAtom, &type, &format,
				 &nItems, &bytesAfter, (unsigned char **) &str);

    if (result != Success)
	return false;

    if (type != typeAtom)
    {
	XFree (str);
	return false;
    }

    string = (char *) str;

    XFree (str);

    return true;
}

void
RegexWindow::updateRole ()
{
    RegexScreen *rs = RegexScreen::get (screen);

    role = "";
    getStringProperty (rs->roleAtom, XA_STRING, role);
}

void
RegexWindow::updateTitle ()
{
    RegexScreen *rs = RegexScreen::get (screen);

    title = "";

    if (getStringProperty (rs->visibleNameAtom, Atoms::utf8String, title))
	return;

    if (getStringProperty (Atoms::wmName, Atoms::utf8String, title))
	return;

    getStringProperty (XA_WM_NAME, XA_STRING, title);
}

void RegexWindow::updateClass ()
{
    XClassHint classHint;

    resClass = "";
    resName  = "";

    if (!XGetClassHint (screen->dpy (), window->id (), &classHint) != Success)
	return;

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
}

void
RegexScreen::handleEvent (XEvent *event)
{
    CompWindow *w;

    screen->handleEvent (event);

    if (event->type != PropertyNotify)
	return;

    w = screen->findWindow (event->xproperty.window);
    if (!w)
	return;

    if (event->xproperty.atom == XA_WM_NAME)
    {
	RegexWindow::get (w)->updateTitle ();
	screen->matchPropertyChanged (w);
    }
    else if (event->xproperty.atom == roleAtom)
    {
	RegexWindow::get (w)->updateRole ();
	screen->matchPropertyChanged (w);
    }
    else if (event->xproperty.atom == XA_WM_CLASS)
    {
	RegexWindow::get (w)->updateClass ();
	screen->matchPropertyChanged (w);
    }
}

/* It's not safe to call CompScreen::matchExpHandlerChanged
 * from the ::RegexScreen constructor since that could end
 * up calling RegexWindow::get () on windows (which haven't
 * had a RegexWindow struct created for them) through
 * ::matchExpHandlerChanged -> CompMatch::evaluate () ->
 * RegexExp::evaluate () ->  RegexWindow::get ()
 */

bool
RegexScreen::applyInitialActions ()
{
    screen->matchExpHandlerChanged ();
    return false;
}

RegexScreen::RegexScreen (CompScreen *s) :
    PluginClassHandler<RegexScreen, CompScreen> (s)
{
    CompTimer::CallBack cb =
	boost::bind (&RegexScreen::applyInitialActions, this);
    ScreenInterface::setHandler (s);

    roleAtom        = XInternAtom (s->dpy (), "WM_WINDOW_ROLE", 0);
    visibleNameAtom = XInternAtom (s->dpy (), "_NET_WM_VISIBLE_NAME", 0);

    mApplyInitialActionsTimer.setTimes (0, 0);
    mApplyInitialActionsTimer.setCallback (cb);
    mApplyInitialActionsTimer.start ();
}

RegexScreen::~RegexScreen ()
{
    screen->matchInitExpSetEnabled (this, false);
    screen->matchExpHandlerChanged ();
}

RegexWindow::RegexWindow (CompWindow *w) :
    PluginClassHandler<RegexWindow, CompWindow> (w),
    window (w)
{
    updateRole ();
    updateTitle ();
    updateClass ();
}

bool
RegexPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
	return false;

    return true;
}
