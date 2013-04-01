/*
 * Copyright Â© 2005 Novell, Inc.
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

#include <compiztoolbox/compiztoolbox.h>
#include "compiztoolbox_options.h"

#include <core/abiversion.h>
#include <core/propertywriter.h>

const unsigned short ICON_SIZE = 48;
const unsigned int MAX_ICON_SIZE = 256;

bool openGLAvailable;

class CompizToolboxScreen :
    public PluginClassHandler <CompizToolboxScreen, CompScreen>,
    public CompiztoolboxOptions
{
    public:
	CompizToolboxScreen (CompScreen *);
};

class CompizToolboxPluginVTable :
    public CompPlugin::VTableForScreen <CompizToolboxScreen>
{
    public:
	bool init ();
	void fini ();
};

COMPIZ_PLUGIN_20090315 (compiztoolbox, CompizToolboxPluginVTable)

CompString
getXDGUserDir (XDGUserDir userDir)
{
    std::ifstream userDirsFile;
    CompString userDirsFilePath;
    const char *userDirsPathSuffix = "/user-dirs.dirs";
    const char *varNames[8] =
    {
	"XDG_DESKTOP_DIR",
	"XDG_DOWNLOAD_DIR",
	"XDG_TEMPLATES_DIR",
	"XDG_PUBLICSHARE_DIR",
	"XDG_DOCUMENTS_DIR",
	"XDG_MUSIC_DIR",
	"XDG_PICTURES_DIR",
	"XDG_VIDEOS_DIR"
    };
    const char *varName = varNames[userDir];
    size_t varLength = strlen (varName);

    char *home = getenv ("HOME");
    if (!(home && strlen (home)))
	return "";

    char *configHome = getenv ("XDG_CONFIG_HOME");
    if (configHome && strlen (configHome))
    {
	userDirsFilePath = configHome;
	userDirsFilePath += userDirsPathSuffix;
    }
    else
    {
	userDirsFilePath = home;
	userDirsFilePath =
	    userDirsFilePath + "/.config" + userDirsPathSuffix;
    }
    userDirsFile.open (userDirsFilePath.c_str (), std::ifstream::in);
    if (!userDirsFile.is_open ())
	return "";

    // The user-dirs file has lines like:
    // XDG_DESKTOP_DIR="$HOME/Desktop"
    // Read it line by line until the desired directory is found.
    while (!userDirsFile.eof())
    {
	CompString line;
	getline (userDirsFile, line);

	size_t varPos = line.find (varName);
	if (varPos != CompString::npos) // if found
	{
	    userDirsFile.close ();

	    // Skip the =" part
	    size_t valueStartPos = varPos + varLength + 2;

	    // Ignore the " at the end
	    CompString value = line.substr (valueStartPos,
					    line.length () - valueStartPos - 1);

	    if (value.substr (0, 5) == "$HOME")
		return CompString (home) + value.substr (5);
	    else if (value.substr (0, 7) == "${HOME}")
		return CompString (home) + value.substr (7);
	    else
		return value;
	}
    }
    userDirsFile.close ();
    return "";
}


void
BaseSwitchScreen::setSelectedWindowHint (bool focus)
{
    Window selectedWindowId = None;
    CompOption::Vector opts;
    CompOption::Value  v;

    if (selectedWindow && !selectedWindow->destroyed ())
    {
	selectedWindowId = selectedWindow->id ();

	/* FIXME: Changing the input focus here will
	 * screw up the ordering of windows in
	 * the switcher, so we probably want to avoid that
	 */
	if (focus)
	    selectedWindow->moveInputFocusTo ();
    }

    v = CompOption::Value ((int) selectedWindowId);
    opts = selectWinAtom.getReadTemplate ();
    opts.at (0).set (v);

    selectWinAtom.updateProperty (popupWindow, opts, XA_WINDOW);
}

void
BaseSwitchScreen::getMinimizedAndMatch (bool &minimizedOption,
					CompMatch *&matchOption)
{
    minimizedOption = false;
    matchOption = NULL;
}

bool
BaseSwitchWindow::isSwitchWin (bool removing)
{
    bool minimizedOption;
    CompMatch *matchOption;
    baseScreen->getMinimizedAndMatch (minimizedOption, matchOption);

    if (!removing && window->destroyed ())
	return false;

    if (!removing && (!window->isViewable () || !window->isMapped ()))
    {
	if (minimizedOption)
	{
	    if (!window->minimized () && !window->inShowDesktopMode () &&
		!window->shaded ())
		return false;
	}
	else
	{
	    return false;
	}
    }

    if (!window->isFocussable ())
	return false;

    if (window->overrideRedirect ())
	return false;

    if (baseScreen->selection == Panels)
    {
	if (!(window->type () &
	      (CompWindowTypeDockMask | CompWindowTypeDesktopMask)))
	    return false;
    }
    else
    {
	if (window->wmType () &
	    (CompWindowTypeDockMask | CompWindowTypeDesktopMask))
	    return false;

	if (window->state () & CompWindowStateSkipTaskbarMask)
	    return false;

	if (matchOption && !matchOption->evaluate (window))
	    return false;
    }

    if (!removing && baseScreen->selection == CurrentViewport)
    {
	if (!window->mapNum () || !window->isViewable ())
	{
	    CompWindow::Geometry &sg = window->serverGeometry ();
	    if (sg.x () + sg.width ()  <= 0    ||
		sg.y () + sg.height () <= 0    ||
		sg.x () >= (int) ::screen->width () ||
		sg.y () >= (int) ::screen->height ())
		return false;
	}
	else
	{
	    if (!window->focus ())
		return false;
	}
    }

    return true;
}

void
BaseSwitchScreen::activateEvent (bool activating)
{
    CompOption::Vector o (0);

    o.push_back (CompOption ("root", CompOption::TypeInt));
    o.push_back (CompOption ("active", CompOption::TypeBool));

    o[0].value ().set ((int) ::screen->root ());
    o[1].value ().set (activating);

    ::screen->handleCompizEvent ("switcher", "activate", o);
}

bool
BaseSwitchScreen::compareWindows (CompWindow *w1,
				  CompWindow *w2)
{
    if (w1->mapNum () && !w2->mapNum ())
	return true;

    if (w2->mapNum () && !w1->mapNum ())
	return false;

    return w2->activeNum () < w1->activeNum ();
}

CompWindow *
BaseSwitchScreen::switchToWindow (bool toNext,
				  bool autoChangeVPOption,
				  bool focus)
{
    CompWindow               *w = NULL;
    CompWindowList::iterator it;

    int cur = 0;
    int nextIdx = 0;

    if (!grabIndex)
	return NULL;

    for (it = windows.begin (); it != windows.end (); ++it, ++cur)
    {
	if (*it == selectedWindow)
	    break;
    }

    if (it == windows.end ())
	return NULL;

    if (toNext)
    {
	++it;
	if (it == windows.end ())
	    w = windows.front ();
	else
	    w = *it;
	nextIdx = (cur + 1) % windows.size ();
    }
    else
    {
	if (it == windows.begin ())
	    w = windows.back ();
	else
	    w = *--it;
	nextIdx = (cur + windows.size () - 1) % windows.size ();
    }

    if (w)
    {
	CompWindow *old = selectedWindow;

	if (selection == AllViewports && autoChangeVPOption)
	{
	    XEvent xev;
	    CompPoint pnt = w->defaultViewport ();

	    xev.xclient.type = ClientMessage;
	    xev.xclient.display = ::screen->dpy ();
	    xev.xclient.format = 32;

	    xev.xclient.message_type = Atoms::desktopViewport;
	    xev.xclient.window = ::screen->root ();

	    xev.xclient.data.l[0] = pnt.x () * ::screen->width ();
	    xev.xclient.data.l[1] = pnt.y () * ::screen->height ();
	    xev.xclient.data.l[2] = 0;
	    xev.xclient.data.l[3] = 0;
	    xev.xclient.data.l[4] = 0;

	    XSendEvent (::screen->dpy (), ::screen->root (), false,
			SubstructureRedirectMask | SubstructureNotifyMask,
			&xev);
	}

	lastActiveNum  = w->activeNum ();
	selectedWindow = w;

	if (old != w)
	    handleSelectionChange (toNext, nextIdx);

	if (popupWindow)
	{
	    CompWindow *popup;

	    popup = ::screen->findWindow (popupWindow);
	    if (popup)
		CompositeWindow::get (popup)->addDamage ();

	    setSelectedWindowHint (focus);
	}

	doWindowDamage (w);

	if (old && !old->destroyed ())
	    doWindowDamage (old);
    }

    return w;
}

void
BaseSwitchScreen::doWindowDamage (CompWindow *w)
{
    CompositeWindow::get (w)->addDamage ();
}

Visual *
BaseSwitchScreen::findArgbVisual (Display *dpy, int scr)
{
    XVisualInfo		*xvi;
    XVisualInfo		temp;
    int			nvi;
    int			i;
    XRenderPictFormat	*format;
    Visual		*visual;

    temp.screen  = scr;
    temp.depth   = 32;
    temp.c_class = TrueColor;

    xvi = XGetVisualInfo (dpy,
			  VisualScreenMask |
			  VisualDepthMask  |
			  VisualClassMask,
			  &temp,
			  &nvi);
    if (!xvi)
	return 0;

    visual = 0;
    for (i = 0; i < nvi; i++)
    {
	format = XRenderFindVisualFormat (dpy, xvi[i].visual);
	if (format->type == PictTypeDirect && format->direct.alphaMask)
	{
	    visual = xvi[i].visual;
	    break;
	}
    }

    XFree (xvi);

    return visual;
}

void
BaseSwitchWindow::paintThumb (const GLWindowPaintAttrib &attrib,
			      const GLMatrix            &transform,
			      unsigned int              mask,
			      int                       x,
			      int                       y,
			      int                       width1,
			      int                       height1,
			      int                       width2,
			      int                       height2)
{
    if (!openGLAvailable)
	return;

    GLWindowPaintAttrib  sAttrib (attrib);
    IconMode             iconMode;
    int                  wx, wy;
    float                width, height;
    GLTexture            *icon = NULL;
    CompWindow::Geometry &g = window->geometry ();

    mask |= PAINT_WINDOW_TRANSFORMED_MASK;

    if (gWindow->textures ().empty ())
	iconMode = ShowIconOnly;
    else
	iconMode = getIconMode ();

    if (window->mapNum ())
    {
	if (gWindow->textures ().empty ())
	    gWindow->bind ();
    }

    if (iconMode != ShowIconOnly)
    {
	GLenum   filter;
	GLMatrix wTransform (transform);
	int      ww, wh;
	int      addWindowGeometryIndex =
	    gWindow->glAddGeometryGetCurrentIndex ();

	width  = width1;
	height = height1;

	ww = window->borderRect ().width ();
	wh = window->borderRect ().height ();

	if (ww > width)
	    sAttrib.xScale = width / ww;
	else
	    sAttrib.xScale = 1.0f;

	if (wh > height)
	    sAttrib.yScale = height / wh;
	else
	    sAttrib.yScale = 1.0f;

	if (sAttrib.xScale < sAttrib.yScale)
	    sAttrib.yScale = sAttrib.xScale;
	else
	    sAttrib.xScale = sAttrib.yScale;

	width  = ww * sAttrib.xScale;
	height = wh * sAttrib.yScale;

	updateIconPos (wx, wy, x, y, width, height);

	sAttrib.xTranslate = wx - g.x () +
			     window->border ().left * sAttrib.xScale;
	sAttrib.yTranslate = wy - g.y () +
			     window->border ().top  * sAttrib.yScale;

	if (window->alpha () || sAttrib.opacity != OPAQUE)
	    mask |= PAINT_WINDOW_TRANSLUCENT_MASK;

	wTransform.translate (g.x (), g.y (), 0.0f);
	wTransform.scale (sAttrib.xScale, sAttrib.yScale, 1.0f);
	wTransform.translate (sAttrib.xTranslate / sAttrib.xScale - g.x (),
			      sAttrib.yTranslate / sAttrib.yScale - g.y (),
			      0.0f);

	filter = gScreen->textureFilter ();

	if (baseScreen->getMipmap ())
	    gScreen->setTextureFilter (GL_LINEAR_MIPMAP_LINEAR);

	/* XXX: replacing the addWindowGeometry function like this is
	   very ugly but necessary until the vertex stage has been made
	   fully pluggable. */
	gWindow->glAddGeometrySetCurrentIndex (MAXSHORT);
	gWindow->glDraw (wTransform, sAttrib, infiniteRegion, mask);
	gWindow->glAddGeometrySetCurrentIndex (addWindowGeometryIndex);

	gScreen->setTextureFilter (filter);

	if (iconMode != HideIcon)
	{
	    icon = gWindow->getIcon (MAX_ICON_SIZE, MAX_ICON_SIZE);
	    if (icon)
		updateIconTexturedWindow (sAttrib, wx, wy, x, y, icon);
	}
    }
    else
    {
	width  = width2;
	height = height2;

	/* try to get a matching icon first */
	icon = gWindow->getIcon (width, height);
	/* if none found, try a large one */
	if (!icon)
	    icon = gWindow->getIcon (MAX_ICON_SIZE, MAX_ICON_SIZE);
	if (!icon)
	    icon = gScreen->defaultIcon ();

	if (icon)
	    updateIconNontexturedWindow (sAttrib, wx, wy,
					 width, height, x, y, icon);
    }

    if (icon)
    {
	CompRegion iconReg (g.x (), g.y (), icon->width (), icon->height ());
	GLTexture::MatrixList matrix (1);
	int addWindowGeometryIndex = gWindow->glAddGeometryGetCurrentIndex ();

	mask |= PAINT_WINDOW_BLEND_MASK;

	matrix[0] = icon->matrix ();
	matrix[0].x0 -= (g.x () * matrix[0].xx);
	matrix[0].y0 -= (g.y () * matrix[0].yy);

	sAttrib.xTranslate = wx - g.x ();
	sAttrib.yTranslate = wy - g.y ();

	gWindow->vertexBuffer ()->begin ();

	gWindow->glAddGeometrySetCurrentIndex (MAXSHORT);
	gWindow->glAddGeometry (matrix, iconReg, infiniteRegion);
	gWindow->glAddGeometrySetCurrentIndex (addWindowGeometryIndex);

	if (gWindow->vertexBuffer ()->end ())
	{
	    GLMatrix           wTransform (transform);

	    wTransform.translate (g.x (), g.y (), 0.0f);
	    wTransform.scale (sAttrib.xScale, sAttrib.yScale, 1.0f);
	    wTransform.translate (sAttrib.xTranslate / sAttrib.xScale - g.x (),
				  sAttrib.yTranslate / sAttrib.yScale - g.y (),
				  0.0f);

	    gWindow->glDrawTexture (icon, wTransform, sAttrib, mask);
	}
    }
}

bool
BaseSwitchWindow::damageRect (bool initial, const CompRect &rect)
{
    if (!openGLAvailable)
	return true;

    if (baseScreen->grabIndex)
    {
	CompWindow *popup;

	popup = ::screen->findWindow (baseScreen->popupWindow);
	if (popup)
	{
	    foreach (CompWindow *w, baseScreen->windows)
	    {
		if (window == w)
		{
		    CompositeWindow::get (popup)->addDamage ();
		    break;
		}
	    }
	}
    }

    return cWindow->damageRect (initial, rect);
}

void
BaseSwitchScreen::updateForegroundColor ()
{
    Atom	  actual;
    int		  result, format;
    unsigned long n, left;
    unsigned char *propData;

    if (!popupWindow)
	return;

    result = XGetWindowProperty (::screen->dpy (), popupWindow,
				 selectFgColorAtom, 0L, 4L, false,
				 XA_INTEGER, &actual, &format,
				 &n, &left, &propData);

    if (result == Success && n && propData)
    {
	if (n == 3 || n == 4)
	{
	    long *data = (long *) propData;

	    fgColor[0] = MIN (0xffff, data[0]);
	    fgColor[1] = MIN (0xffff, data[1]);
	    fgColor[2] = MIN (0xffff, data[2]);

	    if (n == 4)
		fgColor[3] = MIN (0xffff, data[3]);
	}

	XFree (propData);
    }
    else
    {
	fgColor[0] = 0;
	fgColor[1] = 0;
	fgColor[2] = 0;
	fgColor[3] = 0xffff;
    }
}

void
BaseSwitchScreen::handleEvent (XEvent *event)
{
    CompWindow *w = NULL;

    switch (event->type) {
	case DestroyNotify:
	    /* We need to get the CompWindow * for event->xdestroywindow.window
	       here because in the ::screen->handleEvent call below, that
	       CompWindow's id will become 1, so findWindowAtDisplay won't be
	       able to find the CompWindow after that. */
	       w = ::screen->findWindow (event->xdestroywindow.window);
	    break;
	default:
	    break;
    }

    ::screen->handleEvent (event);

    switch (event->type) {
	case UnmapNotify:
	    w = ::screen->findWindow (event->xunmap.window);
	    windowRemove (w);
	    break;
	case DestroyNotify:
	    windowRemove (w);
	    break;
	case PropertyNotify:
	    if (event->xproperty.atom == selectFgColorAtom)
	    {
		if (event->xproperty.window == popupWindow)
		    updateForegroundColor ();
	    }
	    break;
	default:
	    break;
    }
}

BaseSwitchScreen::BaseSwitchScreen (CompScreen *screen) :
    popupWindow (None),
    selectedWindow (NULL),
    lastActiveNum (0),
    grabIndex (NULL),
    moreAdjust (false),
    selection (CurrentViewport),
    ignoreSwitcher (false)
{
    CompOption::Vector atomTemplate;
    CompOption::Value v;
    CompOption	      o;

    if (openGLAvailable)
    {
	cScreen = CompositeScreen::get (screen);
	gScreen = GLScreen::get (screen);
    }

    o.setName ("id", CompOption::TypeInt);
    atomTemplate.push_back (o);

    selectWinAtom = PropertyWriter (CompString (DECOR_SWITCH_WINDOW_ATOM_NAME),
    				    atomTemplate);

    selectFgColorAtom =
    	XInternAtom (::screen->dpy (),
    		     DECOR_SWITCH_FOREGROUND_COLOR_ATOM_NAME, 0);

    fgColor[0] = 0;
    fgColor[1] = 0;
    fgColor[2] = 0;
    fgColor[3] = 0xffff;
}

BaseSwitchWindow::BaseSwitchWindow (BaseSwitchScreen *ss, CompWindow *w) :
    baseScreen (ss),
    window (w)
{
    if (openGLAvailable)
    {
	gWindow = GLWindow::get (w);
	cWindow = CompositeWindow::get (w);
	gScreen = GLScreen::get (screen);
    }

}

CompizToolboxScreen::CompizToolboxScreen (CompScreen *screen) :
    PluginClassHandler <CompizToolboxScreen, CompScreen> (screen)
{
}

bool
CompizToolboxPluginVTable::init ()
{
    openGLAvailable = true;

    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
	return false;

    if (!CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) ||
        !CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
	openGLAvailable = false;

    CompPrivate p;
    p.uval = COMPIZ_COMPIZTOOLBOX_ABI;
    screen->storeValue ("compiztoolbox_ABI", p);

    return true;
}

void
CompizToolboxPluginVTable::fini ()
{
    screen->eraseValue ("compiztoolbox_ABI");
}
