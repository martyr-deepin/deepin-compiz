/*
 * Compiz Core: OutputDevices class
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

#include <core/option.h>
#include "outputdevices.h"
#include "core_options.h"

namespace compiz
{
namespace core
{

OutputDevices::OutputDevices() :
    outputDevs (),
    overlappingOutputs (false),
    currentOutputDev (0)
{
}

void
OutputDevices::setGeometryOnDevice(unsigned int const nOutput,
    int x, int y,
    const int width, const int height)
{
    if (outputDevs.size() < nOutput + 1)
	outputDevs.resize(nOutput + 1);

    outputDevs[nOutput].setGeometry(x, y, width, height);
}

void
OutputDevices::adoptDevices(unsigned int nOutput, CompSize* screen)
{
    /* make sure we have at least one output */
    if (!nOutput)
    {
	setGeometryOnDevice(nOutput, 0, 0, screen->width(), screen->height());
	nOutput++;
    }
    if (outputDevs.size() > nOutput)
	outputDevs.resize(nOutput);

    char str[10];
    /* set name, width, height and update rect pointers in all regions */
    for (unsigned int i = 0; i < nOutput; i++)
    {
	snprintf(str, 10, "Output %d", i);
	outputDevs[i].setId(str, i);
    }
    overlappingOutputs = false;
    setCurrentOutput (currentOutputDev);
    for (unsigned int i = 0; i < nOutput - 1; i++)
	for (unsigned int j = i + 1; j < nOutput; j++)
	    if (outputDevs[i].intersects(outputDevs[j]))
		overlappingOutputs = true;
}

void
OutputDevices::setCurrentOutput (unsigned int outputNum)
{
    if (outputNum >= outputDevs.size ())
	outputNum = 0;

    currentOutputDev = outputNum;
}

int
OutputDevices::outputDeviceForGeometry (
	const CompWindow::Geometry& gm,
	int strategy,
	CompSize* screen) const
{
    int          overlapAreas[outputDevs.size ()];
    int          highest, seen, highestScore;
    int          x, y;
    unsigned int i;
    CompRect     geomRect;

    if (outputDevs.size () == 1)
	return 0;


    if (strategy == CoreOptions::OverlappingOutputsSmartMode)
    {
	/* We're only going to use geomRect for overlapping area calculations,
	   so the window rectangle is enough. We don't need to consider
	   anything more like the border because it will never be significant
	   to the result */
	geomRect = gm;
    }
    else
    {
	/* for biggest/smallest modes, only use the window center to determine
	   the correct output device */
	x = (gm.x () + (gm.width () / 2) + gm.border ()) % screen->width ();
	if (x < 0)
	    x += screen->width ();
	y = (gm.y () + (gm.height () / 2) + gm.border ()) % screen->height ();
	if (y < 0)
	    y += screen->height ();

	geomRect.setGeometry (x, y, 1, 1);
    }

    /* get amount of overlap on all output devices */
    for (i = 0; i < outputDevs.size (); i++)
    {
	CompRect overlap = outputDevs[i] & geomRect;
	overlapAreas[i] = overlap.area ();
    }

    /* find output with largest overlap */
    for (i = 0, highest = 0, highestScore = 0;
	 i < outputDevs.size (); i++)
    {
	if (overlapAreas[i] > highestScore)
	{
	    highest = i;
	    highestScore = overlapAreas[i];
	}
    }

    /* look if the highest score is unique */
    for (i = 0, seen = 0; i < outputDevs.size (); i++)
	if (overlapAreas[i] == highestScore)
	    seen++;

    if (seen > 1)
    {
	/* it's not unique, select one output of the matching ones and use the
	   user preferred strategy for that */
	unsigned int currentSize, bestOutputSize;
	bool         searchLargest;

	searchLargest =
	    (strategy != CoreOptions::OverlappingOutputsPreferSmallerOutput);

	if (searchLargest)
	    bestOutputSize = 0;
	else
	    bestOutputSize = UINT_MAX;

	for (i = 0, highest = 0; i < outputDevs.size (); i++)
	    if (overlapAreas[i] == highestScore)
	    {
		bool bestFit;

		currentSize = outputDevs[i].area ();

		if (searchLargest)
		    bestFit = (currentSize > bestOutputSize);
		else
		    bestFit = (currentSize < bestOutputSize);

		if (bestFit)
		{
		    highest = i;
		    bestOutputSize = currentSize;
		}
	    }
    }

    return highest;
}

} // namespace core
} // namespace compiz

