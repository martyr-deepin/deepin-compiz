/*
 * Copyright © 2006 Novell, Inc.
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

#include "imgsvg_options.h"

#include <composite/composite.h>
#include <decoration.h>
#include <core/screen.h>
#include <core/pluginclasshandler.h>

#include <stdlib.h>
#include <string.h>

#include <imgsvg-config.h>

#include <cairo/cairo-xlib.h>
#include <librsvg/rsvg.h>
 
#ifndef HAVE_RSVG_2_36_2
#include <librsvg/rsvg-cairo.h>
#endif

#include <X11/Xatom.h>
#include <X11/extensions/shape.h>
#include <opengl/opengl.h>

#include <iosfwd>


#define SVG_SCREEN(s) SvgScreen *ss = SvgScreen::get (s)
#define SVG_WINDOW(w) SvgWindow *sw = SvgWindow::get (w)

class SvgScreen :
    public ScreenInterface,
    public PluginClassHandler<SvgScreen, CompScreen>,
    public ImgsvgOptions
{
    public:
	SvgScreen (CompScreen *screen);
	~SvgScreen ();

	bool fileToImage (CompString &path, CompSize &size,
			  int &stride, void *&data);
	void handleCompizEvent (const char *plugin, const char *event,
				CompOption::Vector &options);

	CompRect zoom;

    private:
	bool readSvgToImage (const char *file, CompSize &size, void *& data);
};

class SvgWindow :
    public WindowInterface,
    public GLWindowInterface,
    public PluginClassHandler<SvgWindow, CompWindow>
{
    public:
	SvgWindow (CompWindow *window);
	~SvgWindow ();

	bool glDraw (const GLMatrix &transform,
		     const GLWindowPaintAttrib &attrib,
		     const CompRegion &region, unsigned int mask);
	void moveNotify (int dx, int dy, bool immediate);
	void resizeNotify (int dx, int dy, int dwidth, int dheight);

	void setSvg (CompString &data, decor_point_t p[2]);

    private:
	typedef struct {
	    decor_point_t p1;
	    decor_point_t p2;

	    RsvgHandle	      *svg;
	    RsvgDimensionData dimension;
	} SvgSource;

	typedef struct {
	    GLTexture::List       textures;
	    GLTexture::MatrixList matrices;
	    cairo_t           *cr;
	    Pixmap            pixmap;
	    CompSize          size;
	} SvgTexture;

	typedef struct {
	    SvgSource  *source;
	    CompRegion box;
	    SvgTexture texture[2];
	    CompRect   rect;
	    CompSize   size;
	} SvgContext;

	SvgSource  *source;
	SvgContext *context;

	SvgScreen  *sScreen;
	GLScreen   *gScreen;

	CompWindow *window;
	GLWindow   *gWindow;

	void updateSvgMatrix ();
	void updateSvgContext ();

	void renderSvg (SvgSource *source, SvgTexture &texture, CompSize size,
			float x1, float y1, float x2, float y2);
	bool initTexture (SvgSource *source, SvgTexture &texture, CompSize size);
	void finiTexture (SvgTexture &texture);
};

class SvgPluginVTable :
    public CompPlugin::VTableForScreenAndWindow<SvgScreen, SvgWindow>
{
    public:

	bool init ();
	void fini ();
};
