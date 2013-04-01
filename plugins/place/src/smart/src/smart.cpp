#include "smart.h"
#include <boost/foreach.hpp>

#ifndef foreach
#define foreach BOOST_FOREACH
#endif

/* overlap types */
static const unsigned short NONE = 0;
static const short H_WRONG = -1;
static const short W_WRONG = -2;

namespace compiz
{
    namespace place
    {
	const unsigned int WindowAbove = 1 << 0;
	const unsigned int WindowBelow = 1 << 1;

	Placeable::Placeable ()
	{
	}

	Placeable::~Placeable ()
	{
	}

	void smart (Placeable                      *placeable,
		    CompPoint			   &pos,
		    const compiz::place::Placeable::Vector &placeables)
	{
	    /*
	     * SmartPlacement by Cristian Tibirna (tibirna@kde.org)
	     * adapted for kwm (16-19jan98) and for kwin (16Nov1999) using (with
	     * permission) ideas from fvwm, authored by
	     * Anthony Martin (amartin@engr.csulb.edu).
	     * Xinerama supported added by Balaji Ramani (balaji@yablibli.com)
	     * with ideas from xfce.
	     * adapted for Compiz by Bellegarde Cedric (gnumdk(at)gmail.com)
	     */
	    int overlap = 0, minOverlap = 0;

	    /* temp holder */
	    int basket = 0;
	    /* CT lame flag. Don't like it. What else would do? */
	    bool firstPass = true;

	    /* get the maximum allowed windows space */
	    int xTmp = placeable->workArea ().x ();
	    int yTmp = placeable->workArea ().y ();

	    /* client gabarit */
	    int cw = placeable->geometry ().width () - 1;
	    int ch = placeable->geometry ().height () - 1;

	    int xOptimal = xTmp;
	    int yOptimal = yTmp;

	    /* loop over possible positions */
	    do
	    {
		/* test if enough room in x and y directions */
		if (yTmp + ch > placeable->workArea ().bottom () && ch < placeable->workArea ().height ())
		    overlap = H_WRONG; /* this throws the algorithm to an exit */
		else if (xTmp + cw > placeable->workArea ().right ())
		    overlap = W_WRONG;
		else
		{
		    overlap = NONE; /* initialize */

		    int cxl = xTmp;
		    int cxr = xTmp + cw;
		    int cyt = yTmp;
		    int cyb = yTmp + ch;

		    foreach (Placeable *p, placeables)
		    {
			const compiz::window::Geometry &otherGeometry = p->geometry ();
			const compiz::window::extents::Extents &otherExtents = p->extents ();

			int xl = otherGeometry.x () - otherExtents.left;
			int yt = otherGeometry.y () - otherExtents.top;
			int xr = otherGeometry.x2 () + otherExtents.right + otherGeometry.border () * 2;
			int yb = otherGeometry.y2 () + otherExtents.bottom + otherGeometry.border () * 2;

			/* if windows overlap, calc the overall overlapping */
			if (cxl < xr && cxr > xl && cyt < yb && cyb > yt)
			{
			    xl = MAX (cxl, xl);
			    xr = MIN (cxr, xr);
			    yt = MAX (cyt, yt);
			    yb = MIN (cyb, yb);

			    if (p->state () & compiz::place::WindowAbove)
				overlap += 16 * (xr - xl) * (yb - yt);
			    else if (p->state () & compiz::place::WindowBelow)
				overlap += 0;
			    else
				overlap += (xr - xl) * (yb - yt);
			}
		    }
		}

		/* CT first time we get no overlap we stop */
		if (overlap == NONE)
		{
		    xOptimal = xTmp;
		    yOptimal = yTmp;
		    break;
		}

		if (firstPass)
		{
		    firstPass  = false;
		    minOverlap = overlap;
		}
		/* CT save the best position and the minimum overlap up to now */
		else if (overlap >= NONE && overlap < minOverlap)
		{
		    minOverlap = overlap;
		    xOptimal = xTmp;
		    yOptimal = yTmp;
		}

		/* really need to loop? test if there's any overlap */
		if (overlap > NONE)
		{
		    int possible = placeable->workArea ().right ();

		    if (possible - cw > xTmp)
			possible -= cw;

		    /* compare to the position of each client on the same desk */
		    foreach (Placeable *p, placeables)
		    {
			const compiz::window::Geometry &otherGeometry = p->geometry ();
			const compiz::window::extents::Extents &otherExtents = p->extents ();

			int xl = otherGeometry.x () - otherExtents.left;
			int yt = otherGeometry.y () - otherExtents.top;
			int xr = otherGeometry.x2 () + otherExtents.right + otherGeometry.border () * 2;
			int yb = otherGeometry.y2 () + otherExtents.bottom + otherGeometry.border () * 2;

			/* if not enough room above or under the current
			 * client determine the first non-overlapped x position
			 */
			if (yTmp < yb && yt < ch + yTmp)
			{
			    if (xr > xTmp && possible > xr)
				possible = xr;

			    basket = xl - cw;
			    if (basket > xTmp && possible > basket)
				possible = basket;
			}
		    }
		    xTmp = possible;
		}
		/* else ==> not enough x dimension (overlap was wrong on horizontal) */
		else if (overlap == W_WRONG)
		{
		    xTmp     = placeable->workArea ().x ();
		    int possible = placeable->workArea ().bottom ();

		    if (possible - ch > yTmp)
			possible -= ch;

		    /* test the position of each window on the desk */
		    foreach (Placeable *p, placeables)
		    {
			const compiz::window::Geometry &otherGeometry = p->geometry ();
			const compiz::window::extents::Extents &otherExtents = p->extents ();

			int yt = otherGeometry.y () - otherExtents.top;
			int yb = otherGeometry.y2 () + otherExtents.bottom + otherGeometry.border () * 2;

			/* if not enough room to the left or right of the current
			 * client determine the first non-overlapped y position
			 */
			if (yb > yTmp && possible > yb)
			    possible = yb;

			basket = yt - ch;
			if (basket > yTmp && possible > basket)
			    possible = basket;
		    }
		    yTmp = possible;
		}
	    }
	    while (overlap != NONE && overlap != H_WRONG && yTmp < placeable->workArea ().bottom ());

	    if (ch >= placeable->workArea ().height ())
		yOptimal = placeable->workArea ().y ();

	    pos.setX (xOptimal + placeable->extents ().left);
	    pos.setY (yOptimal + placeable->extents ().top);
	}
    }
}
