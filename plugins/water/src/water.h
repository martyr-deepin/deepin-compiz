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
 *         Frederic Plourde <frederic.plourde@collabora.co.uk>
 */

#include "water_options.h"
#include <core/screen.h>
#include <core/pluginclasshandler.h>

#include <composite/composite.h>
#include <opengl/opengl.h>
#include <opengl/framebufferobject.h>
#include "shaders.h"


#define WATER_SCREEN(s) \
    WaterScreen *ws = WaterScreen::get (s)

extern const unsigned int TEXTURE_SIZE;

extern const float K;

#define TEXTURE_NUM 3
#define PROG_NUM 3

#define INDEX(ws, i) (((ws)->fboIndex + (i)) % TEXTURE_NUM)

enum programTypes { SET, UPDATE, PAINT};

class WaterScreen :
    public ScreenInterface,
    public GLScreenInterface,
    public CompositeScreenInterface,
    public PluginClassHandler<WaterScreen,CompScreen>,
    public WaterOptions
{
    public:

	WaterScreen (CompScreen *screen);
	~WaterScreen ();

	void optionChange (WaterOptions::Options num);

	void handleEvent (XEvent *);

	void glPaintCompositedOutput (const CompRegion    &region,
				      GLFramebufferObject *fbo,
				      unsigned int         mask);
	void preparePaint (int);
	void donePaint ();

	bool fboPrologue (int fboIndex);
	void fboEpilogue ();
	bool fboUpdate (float dt, float fade);

	void waterUpdate (float dt);
	void waterVertices (GLenum type, XPoint *p, int n, float v);

	bool rainTimeout ();
	bool wiperTimeout ();

	void waterSetup ();

	void handleMotionEvent ();

	CompositeScreen *cScreen;
	GLScreen        *gScreen;

	float offsetScale;

	CompScreen::GrabHandle grabIndex;

	GLProgram      *program[PROG_NUM];
	GLVertexBuffer *vertexBuffer[PROG_NUM];

	static GLfloat vertexData[18];

	static GLfloat textureData[12];

	GLFramebufferObject *waterFbo[TEXTURE_NUM];

	GLFramebufferObject *oldFbo;
	GLint oldViewport[4];
	int    fboIndex;
	bool   useFbo;

	int texWidth, texHeight;
	GLenum  target;
	GLfloat tx, ty;

	int count;

	void          *data;
	float         *d0;
	float         *d1;
	unsigned char *t0;

	CompTimer rainTimer;
	CompTimer wiperTimer;

	float wiperAngle;
	float wiperSpeed;

	GLVector lightVec;
};

class WaterPluginVTable :
    public CompPlugin::VTableForScreen<WaterScreen>
{
    public:

	bool init ();
};

