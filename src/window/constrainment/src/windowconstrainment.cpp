/*
 * Copyright © 2005 Novell, Inc.
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

#include <core/windowconstrainment.h>
#include <stdio.h>

static inline int constrainmentFloor (int value, int base)
{
    return (value / base) * base;
}

static inline uint64_t constrainment64Floor (uint64_t value, uint64_t base)
{
    return (value / base) * base;
}

static inline float constrainmentClamp(float x, float a, float b)
{
    return x < a ? a : (x > b ? b : x);
}

CompSize
compiz::window::constrainment::constrainToHints (const XSizeHints &hints,
						 const CompSize   &size,
						 long     ignoreHints,
						 long     resizeIgnoreHints)
{
    int              width = size.width ();
    int              height = size.height ();
    int		     min_width = 1;
    int		     min_height = 1;
    int		     base_width = 1;
    int		     base_height = 1;
    int		     xinc = 1;
    int		     yinc = 1;
    int		     max_width = std::numeric_limits <short>::max ();
    int		     max_height = std::numeric_limits <short>::max ();
    long flags =     hints.flags & ~ignoreHints;
    long resizeIncFlags = (flags & PResizeInc) ? ((PHorzResizeInc | PVertResizeInc) & ~resizeIgnoreHints) : 0;

    /* Ater gdk_window_constrain_size(), which is partially borrowed from fvwm.
     *
     * Copyright 1993, Robert Nation
     *     You may use this code for any purpose, as long as the original
     *     copyright remains in the source code and all documentation
     *
     * which in turn borrows parts of the algorithm from uwm
     */

    if ((flags & PBaseSize) && (flags & PMinSize))
    {
	base_width = std::max (1, hints.base_width);
	base_height = std::max (1, hints.base_height);
	min_width = std::max (1, hints.min_width);
	min_height = std::max (1, hints.min_height);
    }
    else if (flags & PBaseSize)
    {
	base_width = std::max (1, hints.base_width);
	base_height = std::max (1, hints.base_height);
	min_width = std::max (1, hints.base_width);
	min_height = std::max (1, hints.base_height);
    }
    else if (flags & PMinSize)
    {
	base_width = std::max (1, hints.min_width);
	base_height = std::max (1, hints.min_height);
	min_width = std::max (1, hints.min_width);
	min_height = std::max (1, hints.min_height);
    }

    if (flags & PMaxSize)
    {
	max_width = std::max (1, hints.max_width);
	max_height = std::max (1, hints.max_height);
    }

    if (resizeIncFlags & PHorzResizeInc)
	xinc = std::max (xinc, hints.width_inc);

    if (resizeIncFlags & PVertResizeInc)
	yinc = std::max (yinc, hints.height_inc);

    /* clamp width and height to min and max values */
    width  = constrainmentClamp (width, min_width, max_width);
    height = constrainmentClamp (height, min_height, max_height);

    /* shrink to base + N * inc */
    width  = base_width + constrainmentFloor (width - base_width, xinc);
    height = base_height + constrainmentFloor (height - base_height, yinc);

    /* constrain aspect ratio, according to:
     *
     * min_aspect.x       width      max_aspect.x
     * ------------  <= -------- <=  -----------
     * min_aspect.y       height     max_aspect.y
     */
    if ((flags & PAspect) && hints.min_aspect.y > 0 && hints.max_aspect.x > 0)
    {
	/* Use 64 bit arithmetic to prevent overflow */

	uint64_t min_aspect_x = hints.min_aspect.x;
	uint64_t min_aspect_y = hints.min_aspect.y;
	uint64_t max_aspect_x = hints.max_aspect.x;
	uint64_t max_aspect_y = hints.max_aspect.y;
	uint64_t delta;

	if (min_aspect_x * height > width * min_aspect_y)
	{
	    delta = constrainment64Floor (height - width * min_aspect_y / min_aspect_x,
			     yinc);
	    if (height - (int) delta >= min_height)
		height -= delta;
	    else
	    {
		delta = constrainment64Floor (height * min_aspect_x / min_aspect_y - width,
				 xinc);
		if (width + (int) delta <= max_width)
		    width += delta;
	    }
	}

	if (width * max_aspect_y > max_aspect_x * height)
	{
	    delta = constrainment64Floor (width - height * max_aspect_x / max_aspect_y,
			     xinc);
	    if (width - (int) delta >= min_width)
		width -= delta;
	    else
	    {
		delta = constrainment64Floor (width * min_aspect_y / min_aspect_x - height,
				 yinc);
		if (height + (int) delta <= max_height)
		    height += delta;
	    }
	}
    }

    return CompSize (width, height);
}
