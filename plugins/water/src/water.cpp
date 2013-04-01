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

#include "water.h"

#include <math.h>


COMPIZ_PLUGIN_20090315 (water, WaterPluginVTable)

const unsigned int TEXTURE_SIZE = 256;

const float K = 0.1964f;

static int waterLastPointerX = 0;
static int waterLastPointerY = 0;

GLfloat WaterScreen::vertexData [18] = {
    -1.0f, -1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
     1.0f,  1.0f, 0.0f,
};

GLfloat WaterScreen::textureData [12] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    0.0f, 1.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
};

bool
WaterScreen::fboPrologue (int fIndex)
{
    if (!useFbo)
	return false;

    oldFbo = waterFbo[fIndex]->bind ();
    glGetIntegerv(GL_VIEWPORT,  &oldViewport[0]);
    glViewport (0, 0, texWidth, texHeight);

    return true;
}

void
WaterScreen::fboEpilogue ()
{
    GLFramebufferObject::rebind (oldFbo);
    glViewport (oldViewport[0], oldViewport[1], oldViewport[2], oldViewport[3]);
}

void
WaterScreen::waterUpdate (float dt)
{
    GLfloat fade = 1.0f;

    if (count < 1000)
    {
	if (count > 1)
	    fade = 0.90f + (float) count / 10000.0f;
	else
	    fade = 0.0f;
    }

    if (!fboPrologue (INDEX (this, 1)))
	return;

    glEnable (GL_TEXTURE_2D);

    vertexBuffer[UPDATE]->begin ();
    vertexBuffer[UPDATE]->addVertices  (6, &vertexData[0]);
    vertexBuffer[UPDATE]->addTexCoords (0, 6, &textureData[0]);
    vertexBuffer[UPDATE]->end ();

    /*
     * Cleanup:
     * Use GLTexture facilities here, instead of manually setting active
     * texture, especially when there will be texture unit support
     */
    glActiveTexture (GL_TEXTURE0);
    waterFbo[INDEX (this, 2)]->tex ()->setFilter (GL_NEAREST);
    glBindTexture (GL_TEXTURE_2D, waterFbo[INDEX (this, 2)]->tex ()->name ());

    glActiveTexture (GL_TEXTURE1);
    waterFbo[INDEX (this, 0)]->tex ()->setFilter (GL_NEAREST);
    glBindTexture (GL_TEXTURE_2D, waterFbo[INDEX (this, 0)]->tex ()->name ());

    vertexBuffer[UPDATE]->addUniform ("prevTex", 0);
    vertexBuffer[UPDATE]->addUniform ("currTex", 1);
    vertexBuffer[UPDATE]->addUniform ("timeLapse", dt * K);
    vertexBuffer[UPDATE]->addUniform ("fade", fade);

    GLboolean isBlendingEnabled;
    glGetBooleanv (GL_BLEND, &isBlendingEnabled);
    glDisable (GL_BLEND);
    vertexBuffer[UPDATE]->render ();
    if (isBlendingEnabled)
	glEnable (GL_BLEND);

    glActiveTexture (GL_TEXTURE0);
    glBindTexture (GL_TEXTURE_2D, 0);

    glActiveTexture (GL_TEXTURE1);
    glBindTexture (GL_TEXTURE_2D, 0);

    glDisable (GL_TEXTURE_2D);

    fboEpilogue ();

    /* increment texture index */
    fboIndex = INDEX (this, 1);
}

void
WaterScreen::waterVertices (GLenum type,
			  XPoint *p,
			  int    n,
			  float  v)
{
    if (!fboPrologue (INDEX (this, 0)))
	return;

    glColorMask (GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
    glLineWidth (1.0f);

    if (GL::vboEnabled && GL::shaders)
    {
	vertexBuffer[SET]->begin (type);
	float data[3];
	for (int i = 0; i < n; i++)
	{
	    data[0] = ((float) p->x / (float) screen->width ()) * 2.0f - 1.0f;
	    data[1] = (((float) screen->height () - (float) p->y) /
	                (float) screen->height ()) * 2.0f - 1.0f;
	    data[2] = 0.0f;
	    p++;
	    vertexBuffer[SET]->addVertices  (1, &data[0]);
	}
	vertexBuffer[SET]->end();

	vertexBuffer[SET]->addUniform ("color", v);
	GLboolean isBlendingEnabled;
	glGetBooleanv (GL_BLEND, &isBlendingEnabled);
	glDisable (GL_BLEND);
	vertexBuffer[SET]->render ();
	if (isBlendingEnabled)
	    glEnable (GL_BLEND);
    }

    glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    fboEpilogue ();

    if (count <= 0)
    {
	cScreen->preparePaintSetEnabled (this, true);
	gScreen->glPaintOutputSetEnabled (this, true);
	gScreen->glPaintCompositedOutputSetEnabled (this, true);
	cScreen->donePaintSetEnabled (this, true);
    }

    if (count < 3000)
	count = 3000;
}

bool
WaterScreen::rainTimeout ()
{
    XPoint     p;

    p.x = (int) (screen->width ()  * (rand () / (float) RAND_MAX));
    p.y = (int) (screen->height () * (rand () / (float) RAND_MAX));

    waterVertices (GL_POINTS, &p, 1, 0.8f * (rand () / (float) RAND_MAX));

    cScreen->damageScreen ();

    return true;
}

bool
WaterScreen::wiperTimeout ()
{
    if (count)
    {
	if (wiperAngle == 0.0f)
	    wiperSpeed = 2.5f;
	else if (wiperAngle == 180.0f)
	    wiperSpeed = -2.5f;
    }

    return true;
}

void
WaterScreen::waterSetup ()
{
    int size;
    std::string buffer;

    texHeight = TEXTURE_SIZE;
    texWidth  = (texHeight * screen->width ()) / screen->height ();

#ifdef USE_GLES
    target = GL_TEXTURE_2D;
    tx = ty = 1.0f;
#else
    if (GL::textureNonPowerOfTwo ||
	(POWER_OF_TWO (texWidth) && POWER_OF_TWO (texHeight)))
    {
	target = GL_TEXTURE_2D;
	tx = ty = 1.0f;
    }
    else
    {
	target = GL_TEXTURE_RECTANGLE_NV;
	tx = texWidth;
	ty = texHeight;
    }
#endif

    size = (texWidth + 2) * (texHeight + 2);

    data = calloc (1, (sizeof (float) * size * 2) +
		   (sizeof (GLubyte) * texWidth * texHeight * 4));
    if (!data)
	return;

    d0 = (float *)data;
    d1 = (d0 + (size));
    t0 = (unsigned char *) (d1 + (size));

    if (GL::vboEnabled && GL::shaders)
    {
	char buf[8192];
	program[SET] = new GLProgram (set_water_vertices_vertex_shader,
	                              set_water_vertices_fragment_shader);

	if (target == GL_TEXTURE_2D)
	    sprintf (buf, update_water_vertices_fragment_shader.c_str (),
		     "2D", "2D",
		     1.0f / (float) texWidth,  1.0f / (float) texWidth,
		     1.0f / (float) texHeight, 1.0f / (float) texHeight,
		     "2D", "2D", "2D", "2D");
	else
	    sprintf (buf, update_water_vertices_fragment_shader.c_str (),
		     "RECT", "RECT",
		     1.0f, 1.0f, 1.0f, 1.0f,
		     "RECT", "RECT", "RECT", "RECT");

	buffer.assign (buf);
	program[UPDATE] = new GLProgram (update_water_vertices_vertex_shader,
				         buffer);

	sprintf (buf, paint_water_vertices_fragment_shader.c_str (),
		     screen->width (), screen->height ());

	buffer.assign (buf);
	program[PAINT]  = new GLProgram (paint_water_vertices_vertex_shader,
				         buffer);

	vertexBuffer[SET] = new GLVertexBuffer (GL::DYNAMIC_DRAW);
	vertexBuffer[SET]->setProgram (program[SET]);

	vertexBuffer[UPDATE] = new GLVertexBuffer (GL::STATIC_DRAW);
	vertexBuffer[UPDATE]->setProgram (program[UPDATE]);

	vertexBuffer[PAINT] = new GLVertexBuffer (GL::STATIC_DRAW);
	vertexBuffer[PAINT]->setProgram (program[PAINT]);
    }

    if (GL::fboEnabled)
    {
	CompSize size(texWidth, texHeight);
	for (int i = 0; i < TEXTURE_NUM; i++)
	{
	    waterFbo[i] = new GLFramebufferObject ();
	    waterFbo[i]->allocate (size, (char *) t0,
				   GL_BGRA, GL_UNSIGNED_BYTE);
	    // check if FBOs are working. If not, fallback to software textures
	    oldFbo = waterFbo[i]->bind ();
	    waterFbo[i]->rebind (oldFbo);
	    if (!waterFbo[i]->checkStatus ())
	    {
		useFbo = false;
		delete waterFbo[i];
		break;
	    }
	}
    }
}

void
WaterScreen::glPaintCompositedOutput (const CompRegion    &region,
                                      GLFramebufferObject *fbo,
                                      unsigned int         mask)
{
    if (count)
    {
	if (GL::vboEnabled && GL::shaders)
	{
	    GLFramebufferObject::rebind (oldFbo);
	    glViewport (oldViewport[0], oldViewport[1],
			oldViewport[2], oldViewport[3]);

	    vertexBuffer[PAINT]->begin ();
	    vertexBuffer[PAINT]->addVertices (6, &vertexData[0]);
	    vertexBuffer[PAINT]->addTexCoords (0, 6, &textureData[0]);
	    vertexBuffer[PAINT]->end ();

	    glEnable (GL_TEXTURE_2D);

	    glActiveTexture (GL_TEXTURE0);
	    fbo->tex ()->setFilter (GL_LINEAR);
	    glBindTexture (GL_TEXTURE_2D, fbo->tex ()->name ());
	    vertexBuffer[PAINT]->addUniform ("baseTex", 0);

	    glActiveTexture (GL_TEXTURE1);
	    waterFbo[INDEX (this, 0)]->tex ()->setFilter (GL_LINEAR);
	    glBindTexture (GL_TEXTURE_2D,
			waterFbo[INDEX (this, 0)]->tex ()->name ());
	    vertexBuffer[PAINT]->addUniform ("waveTex", 1);

	    vertexBuffer[PAINT]->addUniform3f ("lightVec",
					lightVec[0],
					lightVec[1],
					lightVec[2]);
	    vertexBuffer[PAINT]->addUniform ("offsetScale", offsetScale);
	    GLboolean isBlendingEnabled;
	    glGetBooleanv (GL_BLEND, &isBlendingEnabled);
	    glDisable (GL_BLEND);
	    vertexBuffer[PAINT]->render ();
	    if (isBlendingEnabled)
		glEnable (GL_BLEND);

	    glBindTexture (GL_TEXTURE_2D, 0);
	    glDisable (GL_TEXTURE_2D);
	}
    }
}

/* TODO: a way to control the speed */
void
WaterScreen::preparePaint (int msSinceLastPaint)
{
    if (count)
    {
	count -= 10;
	if (count < 0)
	    count = 0;

	if (wiperTimer.active ())
	{
	    float  step, angle0, angle1;
	    bool   wipe = false;
	    XPoint p[3];

	    p[1].x = screen->width () / 2;
	    p[1].y = screen->height ();

	    step = wiperSpeed * msSinceLastPaint / 20.0f;

	    if (wiperSpeed > 0.0f)
	    {
		if (wiperAngle < 180.0f)
		{
		    angle0 = wiperAngle;

		    wiperAngle += step;
		    wiperAngle = MIN (wiperAngle, 180.0f);

		    angle1 = wiperAngle;

		    wipe = true;
		}
	    }
	    else
	    {
		if (wiperAngle > 0.0f)
		{
		    angle1 = wiperAngle;

		    wiperAngle += step;
		    wiperAngle = MAX (wiperAngle, 0.0f);

		    angle0 = wiperAngle;

		    wipe = true;
		}
	    }

#define TAN(a) (tanf ((a) * (M_PI / 180.0f)))

	    if (wipe)
	    {
		if (angle0 > 0.0f)
		{
		    p[2].x = screen->width () / 2 -
			     screen->height () / TAN (angle0);
		    p[2].y = 0;
		}
		else
		{
		    p[2].x = 0;
		    p[2].y = screen->height ();
		}

		if (angle1 < 180.0f)
		{
		    p[0].x = screen->width () / 2 -
			     screen->height () / TAN (angle1);
		    p[0].y = 0;
		}
		else
		{
		    p[0].x = screen->width ();
		    p[0].y = screen->height ();
		}

		waterVertices (GL_TRIANGLES, p, 3, 0.0f);
	    }

#undef TAN

	}

	waterUpdate (0.8f);
    }

    cScreen->preparePaint (msSinceLastPaint);
}

void
WaterScreen::donePaint ()
{
    if (count)
	cScreen->damageScreen ();
    else
    {
	cScreen->preparePaintSetEnabled (this, false);
	gScreen->glPaintOutputSetEnabled (this, false);
	gScreen->glPaintCompositedOutputSetEnabled (this, false);
	cScreen->donePaintSetEnabled (this, false);
    }

    cScreen->donePaint ();
}

void
WaterScreen::handleMotionEvent ()
{
    if (grabIndex)
    {
	XPoint p[2];

	p[0].x = waterLastPointerX;
	p[0].y = waterLastPointerY;

	p[1].x = waterLastPointerX = pointerX;
	p[1].y = waterLastPointerY = pointerY;

	waterVertices (GL_LINES, p, 2, 0.2f);

	cScreen->damageScreen ();
    }

}

static bool
waterInitiate (CompAction         *action,
	       CompAction::State  state,
	       CompOption::Vector &options)
{
    unsigned int ui;
    Window	 root, child;
    int	         xRoot, yRoot, i;

    WATER_SCREEN (screen);

    if (!screen->otherGrabExist ("water", NULL))
    {
	if (!ws->grabIndex)
	{
	    ws->grabIndex = screen->pushGrab (None, "water");
	    screen->handleEventSetEnabled (ws, true);
	}

	if (XQueryPointer (screen->dpy (), screen->root (), &root, &child,
			   &xRoot, &yRoot, &i, &i, &ui))
	{
	    XPoint p;

	    p.x = waterLastPointerX = xRoot;
	    p.y = waterLastPointerY = yRoot;

 	    ws->waterVertices (GL_POINTS, &p, 1, 1.0f);

	    ws->cScreen->damageScreen ();
	}
    }

    if (state & CompAction::StateInitButton)
	action->setState (action->state () | CompAction::StateTermButton);

    if (state & CompAction::StateInitKey)
	action->setState (action->state () | CompAction::StateTermKey);

    return false;
}

static bool
waterTerminate (CompAction         *action,
	        CompAction::State  state,
	        CompOption::Vector &options)
{
    WATER_SCREEN (screen);

    if (ws->grabIndex)
    {
	screen->removeGrab (ws->grabIndex, 0);
	ws->grabIndex = 0;
	screen->handleEventSetEnabled (ws, false);
    }

    return false;
}

static bool
waterToggleRain (CompAction         *action,
		 CompAction::State  state,
		 CompOption::Vector &options)
{
    /* Remember StateCancel and StateCommit will be broadcast to all actions
       so we need to verify that we are actually being toggled... */
    if (!(state & CompAction::StateTermKey))
        return false;
    /* And only respond to key taps */
    if (!(state & CompAction::StateTermTapped))
        return false;

    WATER_SCREEN (screen);

    if (!ws->rainTimer.active ())
    {
	int delay;

	delay = ws->optionGetRainDelay ();
	ws->rainTimer.start (delay, (float) delay * 1.2);
    }
    else
    {
	ws->rainTimer.stop ();
    }

    return false;
}

static bool
waterToggleWiper (CompAction         *action,
		  CompAction::State  state,
		  CompOption::Vector &options)
{
    WATER_SCREEN (screen);

    if (!ws->wiperTimer.active ())
    {
	ws->wiperTimer.start (2000, 2400);
    }
    else
    {
	ws->wiperTimer.stop ();
    }

    return false;
}

static bool
waterTitleWave (CompAction         *action,
		CompAction::State  state,
		CompOption::Vector &options)
{
    CompWindow *w;
    int	       xid;

    WATER_SCREEN (screen);

    xid = CompOption::getIntOptionNamed (options, "window",
					 screen->activeWindow ());

    w = screen->findWindow (xid);
    if (w)
    {
	CompWindow::Geometry &g = w->geometry ();
	XPoint p[2];

	p[0].x = g.x () - w->border ().left;
	p[0].y = g.y () - w->border ().top / 2;

	p[1].x = g.x () + g.width () + w->border ().right;
	p[1].y = p[0].y;

	ws->waterVertices (GL_LINES, p, 2, 0.15f);

	ws->cScreen->damageScreen ();
    }

    return false;
}

static bool
waterPoint (CompAction         *action,
	    CompAction::State  state,
	    CompOption::Vector &options)
{
    XPoint p;
    float  amp;

    WATER_SCREEN (screen);

    p.x = CompOption::getIntOptionNamed (options, "x",
					 screen->width () / 2);
    p.y = CompOption::getIntOptionNamed (options, "y",
					 screen->height () / 2);

    amp = CompOption::getFloatOptionNamed (options, "amplitude", 0.5f);

    ws->waterVertices (GL_POINTS, &p, 1, amp);

    ws->cScreen->damageScreen ();

    return false;
}

static bool
waterLine (CompAction         *action,
	   CompAction::State  state,
	   CompOption::Vector &options)
{
    XPoint p[2];
    float  amp;

    WATER_SCREEN (screen);

    p[0].x = CompOption::getIntOptionNamed (options, "x0",
					    screen->width () / 4);
    p[0].y = CompOption::getIntOptionNamed (options, "y0",
					    screen->height () / 2);

    p[1].x = CompOption::getIntOptionNamed (options, "x1",
					    screen->width () -
					    screen->width () / 4);
    p[1].y = CompOption::getIntOptionNamed (options, "y1",
					    screen->height () / 2);

    amp = CompOption::getFloatOptionNamed (options, "amplitude", 0.25f);

    ws->waterVertices (GL_LINES, p, 2, amp);

    ws->cScreen->damageScreen ();

    return false;
}

void
WaterScreen::handleEvent (XEvent *event)
{
    switch (event->type) {
	case ButtonPress:
	    if (event->xbutton.root == screen->root () && grabIndex)
	    {
		XPoint p;

		p.x = pointerX;
		p.y = pointerY;

		waterVertices (GL_POINTS, &p, 1, 0.8f);
		cScreen->damageScreen ();
	    }
	    break;
	case EnterNotify:
	case LeaveNotify:
	    if (event->xcrossing.root == screen->root () && grabIndex)
		handleMotionEvent ();
	    break;
	case MotionNotify:
	    if (event->xmotion.root == screen->root () && grabIndex)
		handleMotionEvent ();
	default:
	    break;
    }

    screen->handleEvent (event);
}

void
WaterScreen::optionChange (WaterOptions::Options num)
{
    switch (num) {
	case WaterOptions::OffsetScale:
	    offsetScale = optionGetOffsetScale () * 10.0f;
	    break;
	case WaterOptions::RainDelay:
	    if (rainTimer.active ())
	    {
		rainTimer.setTimes (optionGetRainDelay (),
				    (float)optionGetRainDelay () * 1.2);
	    }
	    break;
	case WaterOptions::LightVecX:
	    lightVec[0] = optionGetLightVecX();
	    break;
	case WaterOptions::LightVecY:
	    lightVec[1] = optionGetLightVecY();
	    break;
	case WaterOptions::LightVecZ:
	    lightVec[2] = optionGetLightVecZ();
	    break;
	default:
	    break;
    }
}

WaterScreen::WaterScreen (CompScreen *screen) :
    PluginClassHandler<WaterScreen,CompScreen> (screen),
    cScreen (CompositeScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    grabIndex (0),

    oldFbo (NULL),
    fboIndex (0),
    useFbo (true),

    texWidth (0),
    texHeight (0),

    target (0),
    tx (0),
    ty (0),

    count (0),

    data (NULL),
    d0 (NULL),
    d1 (NULL),
    t0 (NULL),

    wiperAngle (0),
    wiperSpeed (0),
    lightVec(GLVector(optionGetLightVecX(),
		      optionGetLightVecY(),
		      optionGetLightVecZ()))
{
    offsetScale = optionGetOffsetScale () * 10.0f;

    wiperTimer.setCallback (boost::bind (&WaterScreen::wiperTimeout, this));
    rainTimer.setCallback (boost::bind (&WaterScreen::rainTimeout, this));

    waterSetup ();

    optionSetOffsetScaleNotify (boost::bind (&WaterScreen::optionChange, this, _2));
    optionSetRainDelayNotify (boost::bind (&WaterScreen::optionChange, this, _2));
    optionSetLightVecXNotify   (boost::bind (&WaterScreen::optionChange, this, _2));
    optionSetLightVecYNotify   (boost::bind (&WaterScreen::optionChange, this, _2));
    optionSetLightVecZNotify   (boost::bind (&WaterScreen::optionChange, this, _2));

    optionSetInitiateKeyInitiate (waterInitiate);
    optionSetInitiateKeyTerminate (waterTerminate);
    optionSetToggleRainKeyTerminate (waterToggleRain);
    optionSetToggleWiperKeyInitiate (waterToggleWiper);
    optionSetTitleWaveInitiate (waterTitleWave);
    optionSetPointInitiate (waterPoint);
    optionSetLineInitiate (waterLine);

    ScreenInterface::setHandler (screen, false);
    CompositeScreenInterface::setHandler (cScreen, false);
    GLScreenInterface::setHandler (gScreen, false);
}

WaterScreen::~WaterScreen ()
{
    if (program[SET])
	delete program[SET];

    if (program[UPDATE])
	delete program[UPDATE];

    if (program[PAINT])
	delete program[PAINT];

    for (int i = 0; i < TEXTURE_NUM; i++)
    {
	if (waterFbo[i])
	    delete waterFbo[i];
    }

    if (data)
	free (data);
}

bool
WaterPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) |
        !CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) |
        !CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
	 return false;

    const char *missing = NULL;
    if (!GL::fboSupported)
	missing = "framebuffer objects";
    if (!GL::vboSupported)
	missing = "vertexbuffer objects";
    if (!GL::shaders)
	missing = "GLSL";
    if (missing)
    {
	compLogMessage ("water", CompLogLevelError,
	    "Missing hardware support for %s", missing);
	return false;
    }

    return true;
}

