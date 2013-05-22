/*
 * Compiz Core: OutputDevices class (functions that link to core and/or X11)
 *
 * Copyright (c) 2012 Canonical Ltd.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "outputdevices.h"
#include "core_options.h"

namespace compiz { namespace core {

CompRect
OutputDevices::computeWorkareaForBox (const CompRect& box, CompWindowList const& windows)
{
    CompRegion region;
    int        x1, y1, x2, y2;

    region += box;

    foreach (CompWindow *w, windows)
    {
	if (!w->isMapped ())
	    continue;

	if (w->struts ())
	{
	    x1 = w->struts ()->left.x;
	    y1 = w->struts ()->left.y;
	    x2 = x1 + w->struts ()->left.width;
	    y2 = y1 + w->struts ()->left.height;

	    if (y1 < box.y2 () && y2 > box.y1 ())
		region -= CompRect (x1, box.y1 (), x2 - x1, box.height ());

	    x1 = w->struts ()->right.x;
	    y1 = w->struts ()->right.y;
	    x2 = x1 + w->struts ()->right.width;
	    y2 = y1 + w->struts ()->right.height;

	    if (y1 < box.y2 () && y2 > box.y1 ())
		region -= CompRect (x1, box.y1 (), x2 - x1, box.height ());

	    x1 = w->struts ()->top.x;
	    y1 = w->struts ()->top.y;
	    x2 = x1 + w->struts ()->top.width;
	    y2 = y1 + w->struts ()->top.height;

	    if (x1 < box.x2 () && x2 > box.x1 ())
		region -= CompRect (box.x1 (), y1, box.width (), y2 - y1);

	    x1 = w->struts ()->bottom.x;
	    y1 = w->struts ()->bottom.y;
	    x2 = x1 + w->struts ()->bottom.width;
	    y2 = y1 + w->struts ()->bottom.height;

	    if (x1 < box.x2 () && x2 > box.x1 ())
		region -= CompRect (box.x1 (), y1, box.width (), y2 - y1);
	}
    }

    if (region.isEmpty ())
    {
	compLogMessage ("core", CompLogLevelWarn,
			"Empty box after applying struts, ignoring struts");
	return box;
    }

    return region.boundingRect ();
}

void
OutputDevices::computeWorkAreas(CompRect& workArea, bool& workAreaChanged,
	CompRegion& allWorkArea, CompWindowList const& windows)
{
    for (unsigned int i = 0; i < outputDevs.size(); i++)
    {
	CompRect oldWorkArea = outputDevs[i].workArea();
	workArea = computeWorkareaForBox(outputDevs[i], windows);
	if (workArea != oldWorkArea)
	{
	    workAreaChanged = true;
	    outputDevs[i].setWorkArea(workArea);
	}
	allWorkArea += workArea;
    }
}

void
OutputDevices::updateOutputDevices(CoreOptions& coreOptions, CompSize* screen)
{
    CompOption::Value::Vector& list = coreOptions.optionGetOutputs();
    unsigned int nOutput = 0;
    int x, y, bits;
    unsigned int uWidth, uHeight;
    int width, height;
    int x1, y1, x2, y2;
    foreach(CompOption::Value & value, list)
    {
	x = 0;
	y = 0;
	uWidth = (unsigned) screen->width();
	uHeight = (unsigned) screen->height();

	bits = XParseGeometry(value.s().c_str(), &x, &y, &uWidth, &uHeight);
	width = (int) uWidth;
	height = (int) uHeight;

	if (bits & XNegative)
	    x = screen->width() + x - width;

	if (bits & YNegative)
	    y = screen->height() + y - height;

	x1 = x;
	y1 = y;
	x2 = x + width;
	y2 = y + height;

	if (x1 < 0)
	    x1 = 0;
	if (y1 < 0)
	    y1 = 0;
	if (x2 > screen->width())
	    x2 = screen->width();
	if (y2 > screen->height())
	    y2 = screen->height();

	if (x1 < x2 && y1 < y2)
	{
	    setGeometryOnDevice(nOutput, x1, y1, x2 - x1, y2 - y1);
	    nOutput++;
	}
    }
    adoptDevices(nOutput, screen);
}

}} // namespace compiz::core

