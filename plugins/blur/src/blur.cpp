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

#include <blur.h>

COMPIZ_PLUGIN_20090315 (blur, BlurPluginVTable)

const unsigned short BLUR_GAUSSIAN_RADIUS_MAX = 15;

const unsigned short BLUR_STATE_CLIENT = 0;
const unsigned short BLUR_STATE_DECOR  = 1;
const unsigned short BLUR_STATE_NUM    = 2;

/* pascal triangle based kernel generator */
static int
blurCreateGaussianLinearKernel (int   radius,
				float strength,
				float *amp,
				float *pos,
				int   *optSize)
{
    float factor = 0.5f + (strength / 2.0f);
    float buffer1[BLUR_GAUSSIAN_RADIUS_MAX * 3];
    float buffer2[BLUR_GAUSSIAN_RADIUS_MAX * 3];
    float *ar1 = buffer1;
    float *ar2 = buffer2;
    float *tmp;
    float sum = 0;
    int   size = (radius * 2) + 1;
    int   mySize = ceil (radius / 2.0f);
    int   i, j;

    ar1[0] = 1.0;
    ar1[1] = 1.0;

    for (i = 3; i <= size; i++)
    {
	ar2[0] = 1;

	for (j = 1; j < i - 1; j++)
	    ar2[j] = (ar1[j - 1] + ar1[j]) * factor;

	ar2[i - 1] = 1;

	tmp = ar1;
	ar1 = ar2;
	ar2 = tmp;
    }

    /* normalize */
    for (i = 0; i < size; i++)
	sum += ar1[i];

    if (sum != 0.0f)
	sum = 1.0f / sum;

    for (i = 0; i < size; i++)
	ar1[i] *= sum;

    i = 0;
    j = 0;

    if (radius & 1)
    {
	pos[i] = radius;
	amp[i] = ar1[i];
	i = 1;
	j = 1;
    }

    for (; i < mySize; i++)
    {
	pos[i]  = radius - j;
	pos[i] -= ar1[j + 1] / (ar1[j] + ar1[j + 1]);
	amp[i]  = ar1[j] + ar1[j + 1];

	j += 2;
    }

    pos[mySize] = 0.0;
    amp[mySize] = ar1[radius];

    *optSize = mySize;

    return radius;
}

void
BlurScreen::updateFilterRadius ()
{
    switch (optionGetFilter ()) {
	case BlurOptions::Filter4xbilinear:
	    filterRadius = 2;
	    break;
	case BlurOptions::FilterGaussian: {
	    int   radius   = optionGetGaussianRadius ();
	    float strength = optionGetGaussianStrength ();

	    blurCreateGaussianLinearKernel (radius, strength, amp, pos,
					    &numTexop);

	    filterRadius = radius;
	} break;
	case BlurOptions::FilterMipmap: {
	    float lod = optionGetMipmapLod ();

	    filterRadius = powf (2.0f, ceilf (lod));
	} break;
    }
}


void
BlurScreen::blurReset ()
{
    updateFilterRadius ();

    foreach (BlurFunction &bf, srcBlurFunctions)
	GLFragment::destroyFragmentFunction (bf.id);
    srcBlurFunctions.clear ();
    foreach (BlurFunction &bf, dstBlurFunctions)
	GLFragment::destroyFragmentFunction (bf.id);
    dstBlurFunctions.clear ();

    width = height = 0;

    if (program)
    {
	GL::deletePrograms (1, &program);
	program = 0;
    }
}

static CompRegion
regionFromBoxes (std::vector<BlurBox> boxes,
		 int	 width,
		 int	 height)
{
    CompRegion region;
    int        x1, x2, y1, y2;

    foreach (BlurBox &box, boxes)
    {
	decor_apply_gravity (box.p1.gravity, box.p1.x, box.p1.y,
			     width, height,
			     &x1, &y1);


	decor_apply_gravity (box.p2.gravity, box.p2.x, box.p2.y,
			     width, height,
			     &x2, &y2);

	if (x2 > x1 && y2 > y1)
	    region += CompRect (x1, y1, x2 - x1, y2 - y1);
    }

    return region;
}

void
BlurWindow::updateRegion ()
{
    CompRegion region;

    if (state[BLUR_STATE_DECOR].threshold)
    {
	region += CompRect (-window->output ().left,
			    -window->output ().top,
			    window->width () + window->output ().right,
			    window->height () + window->output ().bottom);

	region -= CompRect (0, 0, window->width (), window->height ());

	state[BLUR_STATE_DECOR].clipped = false;

	if (!state[BLUR_STATE_DECOR].box.empty ())
	{
	    CompRegion q = regionFromBoxes (state[BLUR_STATE_DECOR].box,
					    window->width (),
					    window->height ());
	    if (!q.isEmpty ())
	    {
		q &= region;
		if (q != region)
		{
		    region = q;
		    state[BLUR_STATE_DECOR].clipped = true;
		}
	    }
	}
    }

    if (state[BLUR_STATE_CLIENT].threshold)
    {
	CompRegion r (0, 0, window->width (), window->height ());

	state[BLUR_STATE_CLIENT].clipped = false;

	if (!state[BLUR_STATE_CLIENT].box.empty ())
	{
	    CompRegion q = regionFromBoxes (state[BLUR_STATE_CLIENT].box,
					    window->width (),
					    window->height ());
	    if (!q.isEmpty ())
	    {
		q &= r;

		if (q != r)
		    state[BLUR_STATE_CLIENT].clipped = true;

		region += q;
	    }
	}
	else
	{
	    region += r;
	}
    }

    this->region = region;
    if (!region.isEmpty ())
	this->region.translate (window->x (), window->y ());
}

void
BlurWindow::setBlur (int                  state,
		     int                  threshold,
		     std::vector<BlurBox> box)
{

    this->state[state].threshold = threshold;
    this->state[state].box       = box;

    updateRegion ();

    cWindow->addDamage ();
}

void
BlurWindow::updateAlphaMatch ()
{
    if (!propSet[BLUR_STATE_CLIENT])
    {
	CompMatch *match;

	match = &bScreen->optionGetAlphaBlurMatch ();
	if (match->evaluate (window))
	{
	    if (!state[BLUR_STATE_CLIENT].threshold)
		setBlur (BLUR_STATE_CLIENT, 4, std::vector<BlurBox> ());
	}
	else
	{
	    if (state[BLUR_STATE_CLIENT].threshold)
		setBlur (BLUR_STATE_CLIENT, 0, std::vector<BlurBox> ());
	}
    }
}

void
BlurWindow::updateMatch ()
{
    CompMatch *match;
    bool      focus;

    updateAlphaMatch ();

    match = &bScreen->optionGetFocusBlurMatch ();

    focus = GL::fragmentProgram && match->evaluate (window);
    if (focus != focusBlur)
    {
	focusBlur = focus;
	cWindow->addDamage ();
    }
}



void
BlurWindow::update (int state)
{
    Atom	  actual;
    int		  result, format;
    unsigned long n, left;
    unsigned char *propData;
    int		  threshold = 0;
    std::vector<BlurBox> boxes;

    result = XGetWindowProperty (screen->dpy (), window->id (),
				 bScreen->blurAtom[state], 0L, 8192L, false,
				 XA_INTEGER, &actual, &format,
				 &n, &left, &propData);

    if (result == Success && n && propData)
    {
	propSet[state] = true;

	if (n >= 2)
	{
	    long *data = (long *) propData;
	    BlurBox box;

	    threshold = data[0];

	    if ((n - 2) / 6)
	    {
		unsigned int i;

		data += 2;

		for (i = 0; i < (n - 2) / 6; i++)
		{
		    box.p1.gravity = *data++;
		    box.p1.x       = *data++;
		    box.p1.y       = *data++;
		    box.p2.gravity = *data++;
		    box.p2.x       = *data++;
		    box.p2.y       = *data++;

		    boxes.push_back (box);
		}

	    }
	}

	XFree (propData);
    }
    else
    {
	propSet[state] = false;
    }

    setBlur (state, threshold, boxes);

    updateAlphaMatch ();
}

void
BlurScreen::preparePaint (int msSinceLastPaint)
{
    if (moreBlur)
    {
	int	    steps;
	bool        focus = optionGetFocusBlur ();
	bool        focusBlur;

	steps = (msSinceLastPaint * 0xffff) / blurTime;
	if (steps < 12)
	    steps = 12;

	moreBlur = false;

	foreach (CompWindow *w, screen->windows ())
	{
	    BLUR_WINDOW (w);

	    focusBlur = bw->focusBlur && focus;

	    if (!bw->pulse &&
		(!focusBlur || w->id () == screen->activeWindow ()))
	    {
		if (bw->blur)
		{
		    bw->blur -= steps;
		    if (bw->blur > 0)
			moreBlur = true;
		    else
			bw->blur = 0;
		}
	    }
	    else
	    {
		if (bw->blur < 0xffff)
		{
		    if (bw->pulse)
		    {
			bw->blur += steps * 2;

			if (bw->blur >= 0xffff)
			{
			    bw->blur = 0xffff - 1;
			    bw->pulse = false;
			}

			moreBlur = true;
		    }
		    else
		    {
			bw->blur += steps;
			if (bw->blur < 0xffff)
			    moreBlur = true;
			else
			    bw->blur = 0xffff;
		    }
		}
	    }
	}
    }

    cScreen->preparePaint (msSinceLastPaint);

    if (cScreen->damageMask () & COMPOSITE_SCREEN_DAMAGE_REGION_MASK)
    {
	/* walk from bottom to top and expand damage */
	if (alphaBlur)
	{
	    int	       x1, y1, x2, y2;
	    int	       count = 0;
	    CompRegion damage (cScreen->currentDamage ());

	    foreach (CompWindow *w, screen->windows ())
	    {
		BLUR_WINDOW (w);

		if (!w->isViewable () || !CompositeWindow::get (w)->damaged ())
		    continue;

		if (!bw->region.isEmpty ())
		{
		    CompRect r = bw->region.boundingRect ();
		    CompRect d = damage.boundingRect ();
		    x1 = r.x1 () - filterRadius;
		    y1 = r.y1 () - filterRadius;
		    x2 = r.x2 () + filterRadius;
		    y2 = r.y2 () + filterRadius;

		    if (x1 < d.x2 () &&
			y1 < d.y2 () &&
			x2 > d.x1 () &&
			y2 > d.y1 ())
		    {
			damage.shrink (-filterRadius, -filterRadius);
			count++;
		    }
		}
	    }

	    if (count)
		cScreen->damageRegion (damage);

	    this->count = count;
	}
    }
}

bool
BlurScreen::glPaintOutput (const GLScreenPaintAttrib &sAttrib,
			   const GLMatrix &transform, const CompRegion &region,
			   CompOutput *output, unsigned int mask)
{
    bool status;

    if (alphaBlur)
    {
	stencilBox = region.boundingRect ();
	this->region = region;

	if (mask & PAINT_SCREEN_REGION_MASK)
	{
	    /* we need to redraw more than the screen region being updated */
	    if (count)
	    {
		this->region.shrink (-filterRadius * 2, -filterRadius * 2);

		this->region &= screen->region ();
	    }
	}
    }

    if (!blurOcclusion)
    {
	occlusion = CompRegion ();

	foreach (CompWindow *w, screen->windows ())
	    BlurWindow::get (w)->clip = CompRegion ();
    }

    this->output = output;

    if (alphaBlur)
	status = gScreen->glPaintOutput (sAttrib, transform, this->region, output, mask);
    else
	status = gScreen->glPaintOutput (sAttrib, transform, region, output, mask);

    return status;
}

void
BlurScreen::glPaintTransformedOutput (const GLScreenPaintAttrib &sAttrib,
				      const GLMatrix &transform,
				      const CompRegion &region,
				      CompOutput *output, unsigned int mask)
{
    if (!blurOcclusion)
    {
	occlusion = CompRegion ();

	foreach (CompWindow *w, screen->windows ())
	    BlurWindow::get (w)->clip = CompRegion ();
    }

    gScreen->glPaintTransformedOutput (sAttrib, transform, region, output, mask);
}

void
BlurScreen::donePaint ()
{
    if (moreBlur)
    {
	foreach (CompWindow *w, screen->windows ())
	{
	    BLUR_WINDOW (w);

	    if (bw->blur > 0 && bw->blur < 0xffff)
		bw->cWindow->addDamage ();
	}
    }

    cScreen->donePaint ();
}

bool
BlurWindow::glPaint (const GLWindowPaintAttrib &attrib,
		     const GLMatrix &transform,
		     const CompRegion &region, unsigned int mask)
{

    bool status = gWindow->glPaint (attrib, transform, region, mask);

    if (!bScreen->blurOcclusion &&
	(mask & PAINT_WINDOW_OCCLUSION_DETECTION_MASK))
    {
	clip = bScreen->occlusion;

	if (!(gWindow->lastMask () & PAINT_WINDOW_NO_CORE_INSTANCE_MASK) &&
	    !(gWindow->lastMask () & PAINT_WINDOW_TRANSFORMED_MASK) &&
	    !this->region.isEmpty ())
	    bScreen->occlusion += this->region;
    }

    return status;
}

GLFragment::FunctionId
BlurScreen::getSrcBlurFragmentFunction (GLTexture *texture,
					int       param)
{
    GLFragment::FunctionData data;
    BlurFunction             function;
    int		             target;

    if (texture->target () == GL_TEXTURE_2D)
	target = COMP_FETCH_TARGET_2D;
    else
	target = COMP_FETCH_TARGET_RECT;

    foreach (BlurFunction &bf, srcBlurFunctions)
	if (bf.param == param && bf.target == target)
	    return bf.id;

    if (data.status ())
    {
	static const char *temp[] = { "offset0", "offset1", "sum" };
	unsigned int      i;

	for (i = 0; i < sizeof (temp) / sizeof (temp[0]); i++)
	    data.addTempHeaderOp (temp[i]);

	data.addDataOp (
	    "MUL offset0, program.env[%d].xyzw, { 1.0, 1.0, 0.0, 0.0 };"
	    "MUL offset1, program.env[%d].zwww, { 1.0, 1.0, 0.0, 0.0 };",
	    param, param);


	switch (optionGetFilter ()) {
	    case BlurOptions::Filter4xbilinear:
	    default:
		data.addFetchOp ("output", "offset0", target);
		data.addDataOp ("MUL sum, output, 0.25;");
		data.addFetchOp ("output", "-offset0", target);
		data.addDataOp ("MAD sum, output, 0.25, sum;");
		data.addFetchOp ("output", "offset1", target);
		data.addDataOp ("MAD sum, output, 0.25, sum;");
		data.addFetchOp ("output", "-offset1", target);
		data.addDataOp ("MAD output, output, 0.25, sum;");
		break;
	}

	if (!data.status ())
	    return 0;

	function.id     = data.createFragmentFunction ("blur");
	function.target = target;
	function.param  = param;
	function.unit   = 0;

	srcBlurFunctions.push_back (function);

	return function.id;
    }

    return 0;
}

GLFragment::FunctionId
BlurScreen::getDstBlurFragmentFunction (GLTexture *texture,
				        int       param,
				        int       unit,
				        int       numITC,
				        int       startTC)
{
    BlurFunction             function;
    GLFragment::FunctionData data;
    int                      target;
    char                     *targetString;

    if (texture->target () == GL_TEXTURE_2D)
    {
	target	     = COMP_FETCH_TARGET_2D;
	targetString = (char *) "2D";
    }
    else
    {
	target	     = COMP_FETCH_TARGET_RECT;
	targetString = (char *) "RECT";
    }

    foreach (BlurFunction &function, dstBlurFunctions)
	if (function.param   == param  &&
	    function.target  == target &&
	    function.unit    == unit   &&
	    function.numITC  == numITC &&
	    function.startTC == startTC)
	    return function.id;

    if (data.status ())
    {
	static const char *temp[] = { "fCoord", "mask", "sum", "dst" };
	int	          i, j;
	char              str[1024];
	int               saturation = optionGetSaturation ();
	int               numIndirect;
	int               numIndirectOp;
	int               base, end, ITCbase;

	for (i = 0; (unsigned int) i < sizeof (temp) / sizeof (temp[0]); i++)
	    data.addTempHeaderOp (temp[i]);

	if (saturation < 100)
	    data.addTempHeaderOp ("sat");

	switch (optionGetFilter ()) {
	    case BlurOptions::Filter4xbilinear: {
		static const char *filterTemp[] = {
		    "t0", "t1", "t2", "t3",
		    "s0", "s1", "s2", "s3"
		};

		for (i = 0;
		     (unsigned int) i < sizeof (filterTemp) / sizeof (filterTemp[0]);
		     i++)
		    data.addTempHeaderOp (filterTemp[i]);

		data.addFetchOp ("output", NULL, target);
		data.addColorOp ("output", "output");

		data.addDataOp (
		    "MUL fCoord, fragment.position, program.env[%d];",
		    param);


		data.addDataOp (
		    "ADD t0, fCoord, program.env[%d];"
		    "TEX s0, t0, texture[%d], %s;"

		    "SUB t1, fCoord, program.env[%d];"
		    "TEX s1, t1, texture[%d], %s;"

		    "MAD t2, program.env[%d], { -1.0, 1.0, 0.0, 0.0 }, fCoord;"
		    "TEX s2, t2, texture[%d], %s;"

		    "MAD t3, program.env[%d], { 1.0, -1.0, 0.0, 0.0 }, fCoord;"
		    "TEX s3, t3, texture[%d], %s;"

		    "MUL_SAT mask, output.a, program.env[%d];"

		    "MUL sum, s0, 0.25;"
		    "MAD sum, s1, 0.25, sum;"
		    "MAD sum, s2, 0.25, sum;"
		    "MAD sum, s3, 0.25, sum;",

		    param + 2, unit, targetString,
		    param + 2, unit, targetString,
		    param + 2, unit, targetString,
		    param + 2, unit, targetString,
		    param + 1);

	    } break;
	    case BlurOptions::FilterGaussian: {

		/* try to use only half of the available temporaries to keep
		   other plugins working */
		if ((maxTemp / 2) - 4 >
		     (numTexop + (numTexop - numITC)) * 2)
		{
		    numIndirect   = 1;
		    numIndirectOp = numTexop;
		}
		else
		{
		    i = MAX (((maxTemp / 2) - 4) / 4, 1);
		    numIndirect = ceil ((float)numTexop / (float)i);
		    numIndirectOp = ceil ((float)numTexop / (float)numIndirect);
		}

		/* we need to define all coordinate temporaries if we have
		   multiple indirection steps */
		j = (numIndirect > 1) ? 0 : numITC;

		for (i = 0; i < numIndirectOp * 2; i++)
		{
		    snprintf (str, 1024, "pix_%d", i);
		    data.addTempHeaderOp (str);
		}

		for (i = j * 2; i < numIndirectOp * 2; i++)
		{
		    snprintf (str, 1024, "coord_%d", i);
		    data.addTempHeaderOp (str);
		}

		data.addFetchOp ("output", NULL, target);
		data.addColorOp ("output", "output");

		data.addDataOp (
		    "MUL fCoord, fragment.position, program.env[%d];",
		    param);


		data.addDataOp ("TEX sum, fCoord, texture[%d], %s;",
			        unit + 1, targetString);


		data.addDataOp ("MUL_SAT mask, output.a, program.env[%d];"
				"MUL sum, sum, %f;",
				param + 1, amp[numTexop]);

		for (j = 0; j < numIndirect; j++)
		{
		    base = j * numIndirectOp;
		    end  = MIN ((j + 1) * numIndirectOp, numTexop) - base;

		    ITCbase = MAX (numITC - base, 0);

		    for (i = ITCbase; i < end; i++)
		    {
			data.addDataOp (
			    "ADD coord_%d, fCoord, {0.0, %g, 0.0, 0.0};"
			    "SUB coord_%d, fCoord, {0.0, %g, 0.0, 0.0};",
			    i * 2, pos[base + i] * ty,
			    (i * 2) + 1, pos[base + i] * ty);
		    }

		    for (i = 0; i < ITCbase; i++)
		    {
			data.addDataOp (
			    "TXP pix_%d, fragment.texcoord[%d], texture[%d], %s;"
			    "TXP pix_%d, fragment.texcoord[%d], texture[%d], %s;",
			    i * 2, startTC + ((i + base) * 2),
			    unit + 1, targetString,
			    (i * 2) + 1, startTC + 1 + ((i + base) * 2),
			    unit + 1, targetString);
		    }

		    for (i = ITCbase; i < end; i++)
		    {
			data.addDataOp (
			    "TEX pix_%d, coord_%d, texture[%d], %s;"
			    "TEX pix_%d, coord_%d, texture[%d], %s;",
			    i * 2, i * 2,
			    unit + 1, targetString,
			    (i * 2) + 1, (i * 2) + 1,
			    unit + 1, targetString);
		    }

		    for (i = 0; i < end * 2; i++)
		    {
			data.addDataOp (
			    "MAD sum, pix_%d, %f, sum;",
			    i, amp[base + (i / 2)]);
		    }
		}

	    } break;
	    case BlurOptions::FilterMipmap:
		data.addFetchOp ("output", NULL, target);
		data.addColorOp ("output", "output");

		data.addDataOp (
		    "MUL fCoord, fragment.position, program.env[%d].xyzz;"
		    "MOV fCoord.w, program.env[%d].w;"
		    "TXB sum, fCoord, texture[%d], %s;"
		    "MUL_SAT mask, output.a, program.env[%d];",
		    param, param, unit, targetString,
		    param + 1);

		break;
	}

	if (saturation < 100)
	{
	    data.addDataOp (
		"MUL sat, sum, { 1.0, 1.0, 1.0, 0.0 };"
		"DP3 sat, sat, { %f, %f, %f, %f };"
		"LRP sum.xyz, %f, sum, sat;",
		RED_SATURATION_WEIGHT, GREEN_SATURATION_WEIGHT,
		BLUE_SATURATION_WEIGHT, 0.0f, saturation / 100.0f);
	}

	data.addDataOp (
	    "MAD dst, mask, -output.a, mask;"
	    "MAD output.rgb, sum, dst.a, output;"
	    "ADD output.a, output.a, dst.a;");

	if (!data.status ())
	{
	    return 0;
	}



	function.id      = data.createFragmentFunction ("blur");
	function.target  = target;
	function.param   = param;
	function.unit    = unit;
	function.numITC  = numITC;
	function.startTC = startTC;

	dstBlurFunctions.push_back (function);

	return function.id;
    }

    return 0;
}

bool
BlurScreen::projectVertices (CompOutput     *output,
			     const GLMatrix &transform,
			     const float    *object,
			     float          *scr,
			     int            n)
{
    GLdouble dProjection[16];
    GLdouble dModel[16];
    GLint    viewport[4];
    double   x, y, z;
    int	     i;

    viewport[0] = output->x1 ();
    viewport[1] = screen->height () - output->y2 ();
    viewport[2] = output->width ();
    viewport[3] = output->height ();

    for (i = 0; i < 16; i++)
    {
	dModel[i]      = transform.getMatrix ()[i];
	dProjection[i] = gScreen->projectionMatrix ()[i];
    }

    while (n--)
    {
	if (!gluProject (object[0], object[1], object[2],
			 dModel, dProjection, viewport,
			 &x, &y, &z))
	    return false;

	scr[0] = x;
	scr[1] = y;

	object += 3;
	scr += 2;
    }

    return true;
}

bool
BlurScreen::loadFragmentProgram (GLuint	*program,
				 const char *string)
{
    GLint errorPos;

    /* clear errors */
    glGetError ();

    if (!*program)
	(*GL::genPrograms) (1, program);

    (*GL::bindProgram) (GL_FRAGMENT_PROGRAM_ARB, *program);
    (*GL::programString) (GL_FRAGMENT_PROGRAM_ARB,
			  GL_PROGRAM_FORMAT_ASCII_ARB,
			  strlen (string), string);

    glGetIntegerv (GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
    if (glGetError () != GL_NO_ERROR || errorPos != -1)
    {
	compLogMessage ("blur", CompLogLevelError,
			"Failed to load blur program %s", string);

	(*GL::deletePrograms) (1, program);
	*program = 0;

	return false;
    }

    return true;
}

bool
BlurScreen::loadFilterProgram (int numITC)
{
    char  buffer[4096];
    char  *targetString;
    char  *str = buffer;
    int   i, j;
    int   numIndirect;
    int   numIndirectOp;
    int   base, end, ITCbase;

    if (target == GL_TEXTURE_2D)
	targetString = (char *) "2D";
    else
	targetString = (char *) "RECT";

    str += sprintf (str,
		    "!!ARBfp1.0"
		    "ATTRIB texcoord = fragment.texcoord[0];"
		    "TEMP sum;");

    if (maxTemp - 1 > (numTexop + (numTexop - numITC)) * 2)
    {
	numIndirect   = 1;
	numIndirectOp = numTexop;
    }
    else
    {
	i = (maxTemp - 1) / 4;
	numIndirect = ceil ((float)numTexop / (float)i);
	numIndirectOp = ceil ((float)numTexop / (float)numIndirect);
    }

    /* we need to define all coordinate temporaries if we have
       multiple indirection steps */
    j = (numIndirect > 1) ? 0 : numITC;

    for (i = 0; i < numIndirectOp; i++)
	str += sprintf (str,"TEMP pix_%d, pix_%d;", i * 2, (i * 2) + 1);

    for (i = j; i < numIndirectOp; i++)
	str += sprintf (str,"TEMP coord_%d, coord_%d;", i * 2, (i * 2) + 1);

    str += sprintf (str,
		    "TEX sum, texcoord, texture[0], %s;",
		    targetString);

    str += sprintf (str,
		    "MUL sum, sum, %f;",
		    amp[numTexop]);

    for (j = 0; j < numIndirect; j++)
    {
	base = j * numIndirectOp;
	end  = MIN ((j + 1) * numIndirectOp, numTexop) - base;

	ITCbase = MAX (numITC - base, 0);

	for (i = ITCbase; i < end; i++)
	    str += sprintf (str,
			    "ADD coord_%d, texcoord, {%g, 0.0, 0.0, 0.0};"
			    "SUB coord_%d, texcoord, {%g, 0.0, 0.0, 0.0};",
			    i * 2, pos[base + i] * tx,
			    (i * 2) + 1, pos[base + i] * tx);

	for (i = 0; i < ITCbase; i++)
	    str += sprintf (str,
		"TEX pix_%d, fragment.texcoord[%d], texture[0], %s;"
		"TEX pix_%d, fragment.texcoord[%d], texture[0], %s;",
		i * 2, ((i + base) * 2) + 1, targetString,
		(i * 2) + 1, ((i + base) * 2) + 2, targetString);

	for (i = ITCbase; i < end; i++)
	    str += sprintf (str,
			    "TEX pix_%d, coord_%d, texture[0], %s;"
			    "TEX pix_%d, coord_%d, texture[0], %s;",
			    i * 2, i * 2, targetString,
			    (i * 2) + 1, (i * 2) + 1, targetString);

	for (i = 0; i < end * 2; i++)
	    str += sprintf (str,
			    "MAD sum, pix_%d, %f, sum;",
			    i, amp[base + (i / 2)]);
    }

    str += sprintf (str,
		    "MOV result.color, sum;"
		    "END");

    return loadFragmentProgram (&program, buffer);
}

bool
BlurScreen::fboPrologue ()
{
    if (!fbo)
	return false;

    (*GL::bindFramebuffer) (GL_FRAMEBUFFER_EXT, fbo);

    /* bind texture and check status the first time */
    if (!fboStatus)
    {
	(*GL::framebufferTexture2D) (GL_FRAMEBUFFER_EXT,
				     GL_COLOR_ATTACHMENT0_EXT,
				     target, texture[1],
				     0);

	int currStatus = (*GL::checkFramebufferStatus) (GL_FRAMEBUFFER_EXT);
	if (currStatus != GL_FRAMEBUFFER_COMPLETE_EXT)
	{
	    compLogMessage ("blur", CompLogLevelError,
			    "Framebuffer incomplete");

	    (*GL::bindFramebuffer) (GL_FRAMEBUFFER_EXT, 0);
	    (*GL::deleteFramebuffers) (1, &fbo);

	    fbo = 0;

	    return false;
	}
	else
	    fboStatus = true;
    }

    glPushAttrib (GL_VIEWPORT_BIT | GL_ENABLE_BIT);

    glDrawBuffer (GL_COLOR_ATTACHMENT0_EXT);
    glReadBuffer (GL_COLOR_ATTACHMENT0_EXT);

    glDisable (GL_CLIP_PLANE0);
    glDisable (GL_CLIP_PLANE1);
    glDisable (GL_CLIP_PLANE2);
    glDisable (GL_CLIP_PLANE3);

    glViewport (0, 0, width, height);
    glMatrixMode (GL_PROJECTION);
    glPushMatrix ();
    glLoadIdentity ();
    glOrtho (0.0, width, 0.0, height, -1.0, 1.0);
    glMatrixMode (GL_MODELVIEW);
    glPushMatrix ();
    glLoadIdentity ();

    return true;
}

void
BlurScreen::fboEpilogue ()
{
    (*GL::bindFramebuffer) (GL_FRAMEBUFFER_EXT, 0);

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
    glDepthRange (0, 1);
    glViewport (-1, -1, 2, 2);
    glRasterPos2f (0, 0);

    gScreen->resetRasterPos ();

    glMatrixMode (GL_PROJECTION);
    glPopMatrix ();
    glMatrixMode (GL_MODELVIEW);
    glPopMatrix ();

    glDrawBuffer (GL_BACK);
    glReadBuffer (GL_BACK);

    glPopAttrib ();
}

bool
BlurScreen::fboUpdate (BoxPtr pBox,
		       int    nBox)
{
    int  i, y, iTC = 0;
    bool wasCulled = glIsEnabled (GL_CULL_FACE);

    if (GL::maxTextureUnits && optionGetIndependentTex ())
	iTC = MIN ((GL::maxTextureUnits - 1) / 2, numTexop);

    if (!program)
	if (!loadFilterProgram (iTC))
	    return false;

    if (!fboPrologue ())
	return false;

    glDisable (GL_CULL_FACE);

    glDisableClientState (GL_TEXTURE_COORD_ARRAY);

    glBindTexture (target, texture[0]);

    glEnable (GL_FRAGMENT_PROGRAM_ARB);
    (*GL::bindProgram) (GL_FRAGMENT_PROGRAM_ARB, program);

    glBegin (GL_QUADS);

    while (nBox--)
    {
	y = screen->height () - pBox->y2;

	for (i = 0; i < iTC; i++)
	{
	    (*GL::multiTexCoord2f) (GL_TEXTURE1_ARB + (i * 2),
				    tx * (pBox->x1 + pos[i]),
				    ty * y);
	    (*GL::multiTexCoord2f) (GL_TEXTURE1_ARB + (i * 2) + 1,
				    tx * (pBox->x1 - pos[i]),
				    ty * y);
	}

	glTexCoord2f (tx * pBox->x1, ty * y);
	glVertex2i   (pBox->x1, y);

	for (i = 0; i < iTC; i++)
	{
	    (*GL::multiTexCoord2f) (GL_TEXTURE1_ARB + (i * 2),
				    tx * (pBox->x2 + pos[i]),
				    ty * y);
	    (*GL::multiTexCoord2f) (GL_TEXTURE1_ARB + (i * 2) + 1,
				    tx * (pBox->x2 - pos[i]),
				    ty * y);
	}

	glTexCoord2f (tx * pBox->x2, ty * y);
	glVertex2i   (pBox->x2, y);

	y = screen->height () - pBox->y1;

	for (i = 0; i < iTC; i++)
	{
	    (*GL::multiTexCoord2f) (GL_TEXTURE1_ARB + (i * 2),
				    tx * (pBox->x2 + pos[i]),
				    ty * y);
	    (*GL::multiTexCoord2f) (GL_TEXTURE1_ARB + (i * 2) + 1,
				    tx * (pBox->x2 - pos[i]),
				    ty * y);
	}

	glTexCoord2f (tx * pBox->x2, ty * y);
	glVertex2i   (pBox->x2, y);

	for (i = 0; i < iTC; i++)
	{
	    (*GL::multiTexCoord2f) (GL_TEXTURE1_ARB + (i * 2),
				    tx * (pBox->x1 + pos[i]),
				    ty * y);
	    (*GL::multiTexCoord2f) (GL_TEXTURE1_ARB + (i * 2) + 1,
				    tx * (pBox->x1 - pos[i]),
				    ty * y);
	}

	glTexCoord2f (tx * pBox->x1, ty * y);
	glVertex2i   (pBox->x1, y);

	pBox++;
    }

    glEnd ();

    glDisable (GL_FRAGMENT_PROGRAM_ARB);

    glEnableClientState (GL_TEXTURE_COORD_ARRAY);

    if (wasCulled)
	glEnable (GL_CULL_FACE);

    fboEpilogue ();

    return true;
}

static const unsigned short MAX_VERTEX_PROJECT_COUNT = 20;

void
BlurWindow::projectRegion (CompOutput     *output,
			   const GLMatrix &transform)
{
    float      scrv[MAX_VERTEX_PROJECT_COUNT * 2];
    float      vertices[MAX_VERTEX_PROJECT_COUNT * 3];
    int	       nVertices, nQuadCombine;
    int        i, j, stride;
    float      *v, *vert;
    float      minX, maxX, minY, maxY, minZ, maxZ;
    float      *scr;

    GLTexture::MatrixList ml;
    GLWindow::Geometry    *gm;

    gWindow->geometry ().reset ();
    gWindow->glAddGeometry (ml, bScreen->tmpRegion2, infiniteRegion);

    if (!gWindow->geometry ().vCount)
	return;

    gm = &gWindow->geometry ();

    nVertices    = (gm->indexCount) ? gm->indexCount: gm->vCount;
    nQuadCombine = 1;

    stride = gm->vertexStride;
    vert = gm->vertices + (stride - 3);

    /* we need to find the best value here */
    if (nVertices <= MAX_VERTEX_PROJECT_COUNT)
    {
	for (i = 0; i < nVertices; i++)
	{
	    if (gm->indexCount)
	    {
		v = vert + (stride * gm->indices[i]);
	    }
	    else
	    {
		v = vert + (stride * i);
	    }

	    vertices[i * 3] = v[0];
	    vertices[(i * 3) + 1] = v[1];
	    vertices[(i * 3) + 2] = v[2];
	}
    }
    else
    {
	minX = screen->width ();
	maxX = 0;
	minY = screen->height ();
	maxY = 0;
	minZ = 1000000;
	maxZ = -1000000;

	for (i = 0; i < gm->vCount; i++)
	{
	    v = vert + (stride * i);

	    if (v[0] < minX)
		minX = v[0];

	    if (v[0] > maxX)
		maxX = v[0];

	    if (v[1] < minY)
		minY = v[1];

	    if (v[1] > maxY)
		maxY = v[1];

	    if (v[2] < minZ)
		minZ = v[2];

	    if (v[2] > maxZ)
		maxZ = v[2];
	}

	vertices[0] = vertices[9]  = minX;
	vertices[1] = vertices[4]  = minY;
	vertices[3] = vertices[6]  = maxX;
	vertices[7] = vertices[10] = maxY;
	vertices[2] = vertices[5]  = maxZ;
	vertices[8] = vertices[11] = maxZ;

	nVertices = 4;

	if (maxZ != minZ)
	{
	    vertices[12] = vertices[21] = minX;
	    vertices[13] = vertices[16] = minY;
	    vertices[15] = vertices[18] = maxX;
	    vertices[19] = vertices[22] = maxY;
	    vertices[14] = vertices[17] = minZ;
	    vertices[20] = vertices[23] = minZ;
	    nQuadCombine = 2;
	}
    }

    if (!bScreen->projectVertices (output, transform, vertices, scrv,
				   nVertices * nQuadCombine))
	return;

    for (i = 0; i < nVertices / 4; i++)
    {
	scr = scrv + (i * 4 * 2);

	minX = screen->width ();
	maxX = 0;
	minY = screen->height ();
	maxY = 0;

	for (j = 0; j < 8 * nQuadCombine; j += 2)
	{
	    if (scr[j] < minX)
		minX = scr[j];

	    if (scr[j] > maxX)
		maxX = scr[j];

	    if (scr[j + 1] < minY)
		minY = scr[j + 1];

	    if (scr[j + 1] > maxY)
		maxY = scr[j + 1];
	}

	int x1, y1, x2, y2;

        x1 = minX - bScreen->filterRadius;
        y1 = screen->height () - maxY - bScreen->filterRadius;
        x2 = maxX + bScreen->filterRadius + 0.5f;
        y2 = screen->height () - minY + bScreen->filterRadius + 0.5f;


        bScreen->tmpRegion3 += CompRect (x1, y1, x2 - x1, y2 - y1);

    }
}

bool
BlurWindow::updateDstTexture (const GLMatrix &transform,
			      CompRect       *pExtents,
			      int            clientThreshold)
{
    int        y;
    int        filter;

    filter = bScreen->optionGetFilter ();

    bScreen->tmpRegion3 = CompRegion ();

    if (filter == BlurOptions::FilterGaussian)
    {

	if (state[BLUR_STATE_DECOR].threshold)
	{
	    int  xx, yy, ww, hh;
	    // top
	    xx = window->x () - window->output ().left;
	    yy = window->y () - window->output ().top;
	    ww = window->width () + window->output ().left +
		 window->output ().right;
	    hh = window->output ().top;

	    bScreen->tmpRegion2 = bScreen->tmpRegion.intersected (
		CompRect (xx, yy, ww, hh));

	    if (!bScreen->tmpRegion2.isEmpty ())
		projectRegion (bScreen->output, transform);

	    // bottom
	    xx = window->x () - window->output ().left;
	    yy = window->y () + window->height ();
            ww = window->width () + window->output ().left +
		 window->output ().right;
	    hh = window->output ().bottom;

	    bScreen->tmpRegion2 = bScreen->tmpRegion.intersected (
		CompRect (xx, yy, ww, hh));

	    if (!bScreen->tmpRegion2.isEmpty ())
		projectRegion (bScreen->output, transform);

	    // left
	    xx = window->x () - window->output ().left;
	    yy = window->y ();
	    ww = window->output ().left;
	    hh = window->height ();

            bScreen->tmpRegion2 = bScreen->tmpRegion.intersected (
		CompRect (xx, yy, ww, hh));

	    if (!bScreen->tmpRegion2.isEmpty ())
		projectRegion (bScreen->output, transform);

	    // right
	    xx = window->x () + window->width ();
	    yy = window->y ();
	    ww = window->output ().right;
	    hh = window->height ();

            bScreen->tmpRegion2 = bScreen->tmpRegion.intersected (
		CompRect (xx, yy, ww, hh));

	    if (!bScreen->tmpRegion2.isEmpty ())
		projectRegion (bScreen->output, transform);
	}

	if (clientThreshold)
	{
	    // center
	    bScreen->tmpRegion2 = bScreen->tmpRegion.intersected (
		CompRect (window->x (),
			  window->y (),
			  window->width (),
			  window->height ()));

	    if (!bScreen->tmpRegion2.isEmpty ())
		projectRegion (bScreen->output, transform);
	}
    }
    else
    {
	// center
	bScreen->tmpRegion2 = bScreen->tmpRegion;

	if (!bScreen->tmpRegion2.isEmpty ())
	    projectRegion (bScreen->output, transform);
    }

    bScreen->tmpRegion = bScreen->region.intersected (bScreen->tmpRegion3);

    if (bScreen->tmpRegion.isEmpty ())
	return false;

    *pExtents = bScreen->tmpRegion.boundingRect ();

    if (!bScreen->texture[0] || bScreen->width != screen->width () ||
	bScreen->height != screen->height ())
    {
	int i, textures = 1;

	bScreen->width  = screen->width ();
	bScreen->height = screen->height ();

	if (GL::textureNonPowerOfTwo ||
	    (POWER_OF_TWO (bScreen->width) && POWER_OF_TWO (bScreen->height)))
	{
	    bScreen->target = GL_TEXTURE_2D;
	    bScreen->tx = 1.0f / bScreen->width;
	    bScreen->ty = 1.0f / bScreen->height;
	}
	else
	{
	    bScreen->target = GL_TEXTURE_RECTANGLE_NV;
	    bScreen->tx = 1;
	    bScreen->ty = 1;
	}

	if (filter == BlurOptions::FilterGaussian)
	{
	    if (GL::fbo && !bScreen->fbo)
		(*GL::genFramebuffers) (1, &bScreen->fbo);

	    if (!bScreen->fbo)
		compLogMessage ("blur", CompLogLevelError,
				"Failed to create framebuffer object");

	    textures = 2;
	}

	bScreen->fboStatus = false;

	for (i = 0; i < textures; i++)
	{
	    if (!bScreen->texture[i])
		glGenTextures (1, &bScreen->texture[i]);

	    glBindTexture (bScreen->target, bScreen->texture[i]);

	    glTexImage2D (bScreen->target, 0, GL_RGB,
			  bScreen->width,
			  bScreen->height,
			  0, GL_BGRA,

#if IMAGE_BYTE_ORDER == MSBFirst
			  GL_UNSIGNED_INT_8_8_8_8_REV,
#else
			  GL_UNSIGNED_BYTE,
#endif

			  NULL);

	    glTexParameteri (bScreen->target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    glTexParameteri (bScreen->target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	    if (filter == BlurOptions::FilterMipmap)
	    {
		if (!GL::fbo)
		{
		    compLogMessage ("blur", CompLogLevelWarn,
			     "GL_EXT_framebuffer_object extension "
			     "is required for mipmap filter");
		}
		else if (bScreen->target != GL_TEXTURE_2D)
		{
		    compLogMessage ("blur", CompLogLevelWarn,
			     "GL_ARB_texture_non_power_of_two "
			     "extension is required for mipmap filter");
		}
		else
		{
		    glTexParameteri (bScreen->target, GL_TEXTURE_MIN_FILTER,
				     GL_LINEAR_MIPMAP_LINEAR);
		    glTexParameteri (bScreen->target, GL_TEXTURE_MAG_FILTER,
				     GL_LINEAR_MIPMAP_LINEAR);
		}
	    }

	    glTexParameteri (bScreen->target, GL_TEXTURE_WRAP_S,
			     GL_CLAMP_TO_EDGE);
	    glTexParameteri (bScreen->target, GL_TEXTURE_WRAP_T,
			     GL_CLAMP_TO_EDGE);

	    glCopyTexSubImage2D (bScreen->target, 0, 0, 0, 0, 0,
				 bScreen->width, bScreen->height);
	}
    }
    else
    {
	glBindTexture (bScreen->target, bScreen->texture[0]);

	CompRect br = bScreen->tmpRegion.boundingRect ();

	y = screen->height () - br.y2 ();

	glCopyTexSubImage2D (bScreen->target, 0,
			     br.x1 (), y,
			     br.x1 (), y,
			     br.width (),
			     br.height ());
    }

    switch (filter) {
	case BlurOptions::FilterGaussian:
	    return bScreen->fboUpdate (bScreen->tmpRegion.handle ()->rects,
				       bScreen->tmpRegion.numRects ());
	case BlurOptions::FilterMipmap:
	    if (GL::generateMipmap)
		(*GL::generateMipmap) (bScreen->target);
	    break;
	case BlurOptions::Filter4xbilinear:
	    break;
    }

    glBindTexture (bScreen->target, 0);

    return true;
}

bool
BlurWindow::glDraw (const GLMatrix     &transform,
		    GLFragment::Attrib &attrib,
		    const CompRegion   &region,
		    unsigned int       mask)
{
    bool       status;

    if (bScreen->alphaBlur && !region.isEmpty ())
    {
	int clientThreshold;

	/* only care about client window blurring when it's translucent */
	if (mask & PAINT_WINDOW_TRANSLUCENT_MASK)
	    clientThreshold = state[BLUR_STATE_CLIENT].threshold;
	else
	    clientThreshold = 0;

	if (state[BLUR_STATE_DECOR].threshold || clientThreshold)
	{
	    bool       clipped = false;
	    CompRect   box (0, 0, 0, 0);
	    CompRegion reg;

	    bScreen->mvp = GLMatrix (bScreen->gScreen->projectionMatrix ());
	    bScreen->mvp *= transform;

	    if (mask & PAINT_WINDOW_TRANSFORMED_MASK)
		reg = infiniteRegion;
	    else
		reg = region;

	    bScreen->tmpRegion = this->region.intersected (reg);
	    if (!bScreen->blurOcclusion &&
		!(mask & PAINT_WINDOW_TRANSFORMED_MASK))
		bScreen->tmpRegion -= clip;

	    if (updateDstTexture (transform, &box, clientThreshold))
	    {
		if (clientThreshold)
		{
		    if (state[BLUR_STATE_CLIENT].clipped)
		    {
			if (bScreen->stencilBits)
			{
			    state[BLUR_STATE_CLIENT].active = true;
			    clipped = true;
			}
		    }
		    else
		    {
			state[BLUR_STATE_CLIENT].active = true;
		    }
		}

		if (state[BLUR_STATE_DECOR].threshold)
		{
		    if (state[BLUR_STATE_DECOR].clipped)
		    {
			if (bScreen->stencilBits)
			{
			    state[BLUR_STATE_DECOR].active = true;
			    clipped = true;
			}
		    }
		    else
		    {
			state[BLUR_STATE_DECOR].active = true;
		    }
		}

		if (!bScreen->blurOcclusion && !clip.isEmpty ())
		    clipped = true;
	    }

	    if (!bScreen->blurOcclusion)
		bScreen->tmpRegion = this->region - clip;
	    else
		bScreen->tmpRegion = this->region;

	    if (!clientThreshold)
	    {
		bScreen->tmpRegion -= CompRect (window->x (),
						window->x () + window->width (),
						window->y (),
						window->y () + window->height ());
	    }

	    if (clipped)
	    {
		GLTexture::MatrixList ml;

		gWindow->geometry ().reset ();

		gWindow->glAddGeometry (ml, bScreen->tmpRegion, reg);
		if (gWindow->geometry ().vCount)
		{
		    CompRect clearBox = bScreen->stencilBox;

		    bScreen->stencilBox = box;

		    glEnable (GL_STENCIL_TEST);
		    glColorMask (GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

		    if (clearBox.x2 () > clearBox.x1 () &&
			clearBox.y2 () > clearBox.y1 ())
		    {
			glPushAttrib (GL_SCISSOR_BIT);
			glEnable (GL_SCISSOR_TEST);
			glScissor (clearBox.x1 (),
				   screen->height () - clearBox.y2 (),
				   clearBox.width (),
				   clearBox.height ());
			glClear (GL_STENCIL_BUFFER_BIT);
			glPopAttrib ();
		    }

		    glStencilFunc (GL_ALWAYS, 0x1, ~0);
		    glStencilOp (GL_KEEP, GL_KEEP, GL_REPLACE);

		    glDisableClientState (GL_TEXTURE_COORD_ARRAY);
		    gWindow->glDrawGeometry ();
		    glEnableClientState (GL_TEXTURE_COORD_ARRAY);

		    glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		    glDisable (GL_STENCIL_TEST);
		}
	    }
	}
    }

    status = gWindow->glDraw (transform, attrib, region, mask);

    state[BLUR_STATE_CLIENT].active = false;
    state[BLUR_STATE_DECOR].active  = false;

    return status;
}

void
BlurWindow::glDrawTexture (GLTexture          *texture,
			   GLFragment::Attrib &attrib,
			   unsigned int       mask)
{
    int state = BLUR_STATE_DECOR;

    foreach (GLTexture *tex, gWindow->textures ())
	if (texture == tex)
	    state = BLUR_STATE_CLIENT;

    if (blur || this->state[state].active)
    {
	GLFragment::Attrib fa (attrib);
	int	           param, function;
	int	           unit = 0;
	GLfloat	           dx, dy;
	int                iTC = 0;

	if (blur)
	{
	    param = fa.allocParameters (1);

	    function = bScreen->getSrcBlurFragmentFunction (texture, param);
	    if (function)
	    {
		fa.addFunction (function);

		dx = ((texture->matrix ().xx / 2.1f) * blur) / 65535.0f;
		dy = ((texture->matrix ().yy / 2.1f) * blur) / 65535.0f;

		(*GL::programEnvParameter4f) (GL_FRAGMENT_PROGRAM_ARB,
					      param, dx, dy, dx, -dy);

		/* bi-linear filtering is required */
		mask |= PAINT_WINDOW_ON_TRANSFORMED_SCREEN_MASK;
	    }
	}

	if (this->state[state].active)
	{
	    GLFragment::Attrib dstFa (fa);
	    float	       threshold = (float) this->state[state].threshold;

	    switch (bScreen->optionGetFilter ()) {
		case BlurOptions::Filter4xbilinear:
		    dx = bScreen->tx / 2.1f;
		    dy = bScreen->ty / 2.1f;

		    param = dstFa.allocParameters (3);
		    unit  = dstFa.allocTextureUnits (1);

		    function = bScreen->getDstBlurFragmentFunction (
			texture, param, unit, 0, 0);
		    if (function)
		    {
			dstFa.addFunction (function);

			(*GL::activeTexture) (GL_TEXTURE0_ARB + unit);
			glBindTexture (bScreen->target, bScreen->texture[0]);
			(*GL::activeTexture) (GL_TEXTURE0_ARB);

			(*GL::programEnvParameter4f) (GL_FRAGMENT_PROGRAM_ARB,
						      param,
						      bScreen->tx, bScreen->ty,
						      0.0f, 0.0f);

			(*GL::programEnvParameter4f) (GL_FRAGMENT_PROGRAM_ARB,
						      param + 1,
						      threshold, threshold,
						      threshold, threshold);

			(*GL::programEnvParameter4f) (GL_FRAGMENT_PROGRAM_ARB,
						      param + 2,
						      dx, dy, 0.0f, 0.0f);
		    }
		    break;
		case BlurOptions::FilterGaussian:
		    if (bScreen->optionGetIndependentTex ())
		    {
			/* leave one free texture unit for fragment position */
			iTC = MAX (0, GL::maxTextureUnits -
				   (gWindow->geometry ().texUnits + 1));
			if (iTC)
			    iTC = MIN (iTC / 2, bScreen->numTexop);
		    }

		    param = dstFa.allocParameters (2);
		    unit  = dstFa.allocTextureUnits (2);

		    function = bScreen->getDstBlurFragmentFunction (
			texture, param, unit, iTC,
			gWindow->geometry ().texUnits);

		    if (function)
		    {
			dstFa.addFunction (function);

			(*GL::activeTexture) (GL_TEXTURE0_ARB + unit);
			glBindTexture (bScreen->target, bScreen->texture[0]);
			(*GL::activeTexture) (GL_TEXTURE0_ARB + unit + 1);
			glBindTexture (bScreen->target, bScreen->texture[1]);
			(*GL::activeTexture) (GL_TEXTURE0_ARB);

			(*GL::programEnvParameter4f) (GL_FRAGMENT_PROGRAM_ARB,
						      param, bScreen->tx,
						      bScreen->ty, 0.0f, 0.0f);

			(*GL::programEnvParameter4f) (GL_FRAGMENT_PROGRAM_ARB,
						      param + 1,
						      threshold, threshold,
						      threshold, threshold);

			if (iTC)
			{
			    GLMatrix tm, rm;
			    float s_gen[4], t_gen[4], q_gen[4];

			    for (unsigned int i = 0; i < 16; i++)
				tm[i] = 0;
			    tm[0] = (bScreen->output->width () / 2.0) *
				    bScreen->tx;
			    tm[5] = (bScreen->output->height () / 2.0) *
				    bScreen->ty;
			    tm[10] = 1;

			    tm[12] = (bScreen->output->width () / 2.0 +
				     bScreen->output->x1 ()) * bScreen->tx;
			    tm[13] = (bScreen->output->height () / 2.0 +
				     screen->height () -
				     bScreen->output->y2 ()) * bScreen->ty;
			    tm[14] = 1;
			    tm[15] = 1;

			    tm *= bScreen->mvp;

			    for (int i = 0; i < iTC; i++)
			    {
				(*GL::activeTexture) (GL_TEXTURE0_ARB +
				    gWindow->geometry ().texUnits + (i * 2));

				rm.reset ();
				rm[13] = bScreen->ty * bScreen->pos[i];
				rm *= tm;

				s_gen[0] = rm[0];
				s_gen[1] = rm[4];
				s_gen[2] = rm[8];
				s_gen[3] = rm[12];
				t_gen[0] = rm[1];
				t_gen[1] = rm[5];
				t_gen[2] = rm[9];
				t_gen[3] = rm[13];
				q_gen[0] = rm[3];
				q_gen[1] = rm[7];
				q_gen[2] = rm[11];
				q_gen[3] = rm[15];

				glTexGenfv (GL_T, GL_OBJECT_PLANE, t_gen);
				glTexGenfv (GL_S, GL_OBJECT_PLANE, s_gen);
				glTexGenfv (GL_Q, GL_OBJECT_PLANE, q_gen);

				glTexGeni (GL_S, GL_TEXTURE_GEN_MODE,
					   GL_OBJECT_LINEAR);
				glTexGeni (GL_T, GL_TEXTURE_GEN_MODE,
					   GL_OBJECT_LINEAR);
				glTexGeni (GL_Q, GL_TEXTURE_GEN_MODE,
					   GL_OBJECT_LINEAR);

				glEnable (GL_TEXTURE_GEN_S);
				glEnable (GL_TEXTURE_GEN_T);
				glEnable (GL_TEXTURE_GEN_Q);

				(*GL::activeTexture) (GL_TEXTURE0_ARB +
				    gWindow->geometry ().texUnits +
				    1 + (i * 2));

				rm.reset ();

				rm[13] = -bScreen->ty * bScreen->pos[i];
				rm *= tm;

				s_gen[0] = rm[0];
				s_gen[1] = rm[4];
				s_gen[2] = rm[8];
				s_gen[3] = rm[12];
				t_gen[0] = rm[1];
				t_gen[1] = rm[5];
				t_gen[2] = rm[9];
				t_gen[3] = rm[13];
				q_gen[0] = rm[3];
				q_gen[1] = rm[7];
				q_gen[2] = rm[11];
				q_gen[3] = rm[15];

				glTexGenfv (GL_T, GL_OBJECT_PLANE, t_gen);
				glTexGenfv (GL_S, GL_OBJECT_PLANE, s_gen);
				glTexGenfv (GL_Q, GL_OBJECT_PLANE, q_gen);

				glTexGeni (GL_S, GL_TEXTURE_GEN_MODE,
					   GL_OBJECT_LINEAR);
				glTexGeni (GL_T, GL_TEXTURE_GEN_MODE,
					   GL_OBJECT_LINEAR);
				glTexGeni (GL_Q, GL_TEXTURE_GEN_MODE,
					   GL_OBJECT_LINEAR);

				glEnable (GL_TEXTURE_GEN_S);
				glEnable (GL_TEXTURE_GEN_T);
				glEnable (GL_TEXTURE_GEN_Q);
			    }

			    (*GL::activeTexture) (GL_TEXTURE0_ARB);
			}

		    }
		    break;
		case BlurOptions::FilterMipmap:
		    param = dstFa.allocParameters (2);
		    unit  = dstFa.allocTextureUnits (1);

		    function =
			bScreen->getDstBlurFragmentFunction (texture, param,
							     unit, 0, 0);
		    if (function)
		    {
			float lod =
			    bScreen->optionGetMipmapLod ();

			dstFa.addFunction (function);

			(*GL::activeTexture) (GL_TEXTURE0_ARB + unit);
			glBindTexture (bScreen->target, bScreen->texture[0]);
			(*GL::activeTexture) (GL_TEXTURE0_ARB);

			(*GL::programEnvParameter4f) (GL_FRAGMENT_PROGRAM_ARB,
						      param,
						      bScreen->tx, bScreen->ty,
						      0.0f, lod);

			(*GL::programEnvParameter4f) (GL_FRAGMENT_PROGRAM_ARB,
						      param + 1,
						      threshold, threshold,
						      threshold, threshold);
		    }
		    break;
	    }

	    if (this->state[state].clipped ||
		(!bScreen->blurOcclusion && !clip.isEmpty ()))
	    {
		glEnable (GL_STENCIL_TEST);

		glStencilOp (GL_KEEP, GL_KEEP, GL_KEEP);
		glStencilFunc (GL_EQUAL, 0x1, ~0);

		/* draw region with destination blur */
		gWindow->glDrawTexture (texture, dstFa, mask);

		glStencilFunc (GL_EQUAL, 0, ~0);

		/* draw region without destination blur */
		gWindow->glDrawTexture (texture, fa, mask);

		glDisable (GL_STENCIL_TEST);
	    }
	    else
	    {
		/* draw with destination blur */
		gWindow->glDrawTexture (texture, dstFa, mask);
	    }
	}
	else
	{
	    gWindow->glDrawTexture (texture, fa, mask);
	}

	if (unit)
	{
	    (*GL::activeTexture) (GL_TEXTURE0_ARB + unit);
	    glBindTexture (bScreen->target, 0);
	    (*GL::activeTexture) (GL_TEXTURE0_ARB + unit + 1);
	    glBindTexture (bScreen->target, 0);
	    (*GL::activeTexture) (GL_TEXTURE0_ARB);
	}

	if (iTC)
	{
	    int i;
	    for (i = gWindow->geometry ().texUnits;
		 i < gWindow->geometry ().texUnits + (2 * iTC); i++)
	    {
		(*GL::activeTexture) (GL_TEXTURE0_ARB + i);
		glDisable (GL_TEXTURE_GEN_S);
		glDisable (GL_TEXTURE_GEN_T);
		glDisable (GL_TEXTURE_GEN_Q);
	    }
	    (*GL::activeTexture) (GL_TEXTURE0_ARB);
	}
    }
    else
    {
	gWindow->glDrawTexture (texture, attrib, mask);
    }
}

void
BlurScreen::handleEvent (XEvent *event)
{
    Window activeWindow = screen->activeWindow ();

    screen->handleEvent (event);

    if (screen->activeWindow () != activeWindow)
    {
	CompWindow *w;

	w = screen->findWindow (activeWindow);
	if (w)
	{
	    if (optionGetFocusBlur ())
	    {
		CompositeWindow::get (w)->addDamage ();
		moreBlur = true;
	    }
	}

	w = screen->findWindow (screen->activeWindow ());
	if (w)
	{
	    if (optionGetFocusBlur ())
	    {
		CompositeWindow::get (w)->addDamage ();
		moreBlur = true;
	    }
	}
    }

    if (event->type == PropertyNotify)
    {
	int i;

	for (i = 0; i < BLUR_STATE_NUM; i++)
	{
	    if (event->xproperty.atom == blurAtom[i])
	    {
		CompWindow *w;

		w = screen->findWindow (event->xproperty.window);
		if (w)
		    BlurWindow::get (w)->update (i);
	    }
	}
    }
}

void
BlurWindow::resizeNotify (int dx,
			  int dy,
			  int dwidth,
			  int dheight)
{
    if (bScreen->alphaBlur)
    {
	if (state[BLUR_STATE_CLIENT].threshold ||
	    state[BLUR_STATE_DECOR].threshold)
	    updateRegion ();
    }

    window->resizeNotify (dx, dy, dwidth, dheight);

}

void
BlurWindow::moveNotify (int  dx,
		        int  dy,
		        bool immediate)
{
    if (!region.isEmpty ())
	region.translate (dx, dy);

    window->moveNotify (dx, dy, immediate);
}

static bool
blurPulse (CompAction         *action,
	   CompAction::State  state,
	   CompOption::Vector &options)
{
    CompWindow *w;
    int	       xid;

    xid = CompOption::getIntOptionNamed (options, "window",
					 screen->activeWindow ());

    w = screen->findWindow (xid);
    if (w && GL::fragmentProgram)
    {
	BLUR_SCREEN (screen);
	BLUR_WINDOW (w);

	bw->pulse    = true;
	bs->moreBlur = true;

	bw->cWindow->addDamage ();
    }

    return false;
}

void
BlurScreen::matchExpHandlerChanged ()
{
    screen->matchExpHandlerChanged ();

    /* match options are up to date after the call to matchExpHandlerChanged */
    foreach (CompWindow *w, screen->windows ())
	BlurWindow::get (w)->updateMatch ();

}

void
BlurScreen::matchPropertyChanged (CompWindow *w)
{
    BlurWindow::get (w)->updateMatch ();

    screen->matchPropertyChanged (w);
}

bool
BlurScreen::setOption (const CompString &name, CompOption::Value &value)
{
    unsigned int index;

    bool rv = BlurOptions::setOption (name, value);

    if (!rv || !CompOption::findOption (getOptions (), name, &index))
	return false;

    switch (index) {
	case BlurOptions::BlurSpeed:
	    blurTime = 1000.0f / optionGetBlurSpeed ();
	    break;
	case BlurOptions::FocusBlurMatch:
	case BlurOptions::AlphaBlurMatch:
	    foreach (CompWindow *w, screen->windows ())
		BlurWindow::get (w)->updateMatch ();

	    moreBlur = true;
	    cScreen->damageScreen ();
	    break;
	case BlurOptions::FocusBlur:
	    moreBlur = true;
	    cScreen->damageScreen ();
	    break;
	case BlurOptions::AlphaBlur:
	    if (GL::fragmentProgram && optionGetAlphaBlur ())
		alphaBlur = true;
	    else
		alphaBlur = false;

	    cScreen->damageScreen ();
	    break;
	case BlurOptions::Filter:
	    blurReset ();
	    cScreen->damageScreen ();
	    break;
	case BlurOptions::GaussianRadius:
	case BlurOptions::GaussianStrength:
	case BlurOptions::IndependentTex:
	    if (optionGetFilter () == BlurOptions::FilterGaussian)
	    {
		blurReset ();
		cScreen->damageScreen ();
	    }
	    break;
	case BlurOptions::MipmapLod:
	    if (optionGetFilter () == BlurOptions::FilterMipmap)
	    {
		blurReset ();
		cScreen->damageScreen ();
	    }
	    break;
	case BlurOptions::Saturation:
	    blurReset ();
	    cScreen->damageScreen ();
	    break;
	case BlurOptions::Occlusion:
	    blurOcclusion = optionGetOcclusion ();
	    blurReset ();
	    cScreen->damageScreen ();
	    break;
	default:
	    break;
    }

    return rv;
}

BlurScreen::BlurScreen (CompScreen *screen) :
    PluginClassHandler<BlurScreen,CompScreen> (screen),
    gScreen (GLScreen::get (screen)),
    cScreen (CompositeScreen::get (screen)),
    moreBlur (false),
    filterRadius (0),
    srcBlurFunctions (0),
    dstBlurFunctions (0),
    output (NULL),
    count (0),
    program (0),
    maxTemp (32),
    fbo (0),
    fboStatus (0)
{

    blurAtom[BLUR_STATE_CLIENT] =
	XInternAtom (screen->dpy (), "_COMPIZ_WM_WINDOW_BLUR", 0);
    blurAtom[BLUR_STATE_DECOR] =
	XInternAtom (screen->dpy (), DECOR_BLUR_ATOM_NAME, 0);

    blurTime = 1000.0f / optionGetBlurSpeed ();
    blurOcclusion = optionGetOcclusion ();

    for (int i = 0; i < 2; i++)
	texture[i] = 0;

    glGetIntegerv (GL_STENCIL_BITS, &stencilBits);
    if (!stencilBits)
	compLogMessage ("blur", CompLogLevelWarn,
			"No stencil buffer. Region based blur disabled");

    /* We need GL_ARB_fragment_program for blur */
    if (GL::fragmentProgram)
	alphaBlur = optionGetAlphaBlur ();
    else
	alphaBlur = false;

    if (GL::fragmentProgram)
    {
	int tmp[4];
	GL::getProgramiv (GL_FRAGMENT_PROGRAM_ARB,
			  GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB,
			  tmp);
	maxTemp = tmp[0];
    }

    updateFilterRadius ();

    optionSetPulseInitiate (blurPulse);

    ScreenInterface::setHandler (screen, true);
    CompositeScreenInterface::setHandler (cScreen, true);
    GLScreenInterface::setHandler (gScreen, true);
}


BlurScreen::~BlurScreen ()
{
    foreach (BlurFunction &bf, srcBlurFunctions)
	GLFragment::destroyFragmentFunction (bf.id);
    foreach (BlurFunction &bf, dstBlurFunctions)
	GLFragment::destroyFragmentFunction (bf.id);

    cScreen->damageScreen ();

    if (fbo)
	(*GL::deleteFramebuffers) (1, &fbo);

    for (int i = 0; i < 2; i++)
	if (texture[i])
	    glDeleteTextures (1, &texture[i]);

}

BlurWindow::BlurWindow (CompWindow *w) :
    PluginClassHandler<BlurWindow,CompWindow> (w),
    window (w),
    cWindow (CompositeWindow::get (w)),
    gWindow (GLWindow::get (w)),
    bScreen (BlurScreen::get (screen)),
    blur (0),
    pulse (false),
    focusBlur (false)
{
    for (int i = 0; i < BLUR_STATE_NUM; i++)
    {
	state[i].threshold = 0;
	state[i].clipped   = false;
	state[i].active    = false;

	propSet[i] = false;
    }

    update (BLUR_STATE_CLIENT);
    update (BLUR_STATE_DECOR);

    updateMatch ();


    WindowInterface::setHandler (window, true);
    GLWindowInterface::setHandler (gWindow, true);
}

BlurWindow::~BlurWindow ()
{
}

bool
BlurPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) |
        !CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) |
        !CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
	 return false;

    return true;
}

