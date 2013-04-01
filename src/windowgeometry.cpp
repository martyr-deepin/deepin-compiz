/*
 * Copyright © 2008 Dennis Kasprzyk
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Dennis Kasprzyk not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Dennis Kasprzyk makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * DENNIS KASPRZYK DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL DENNIS KASPRZYK BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors: Dennis Kasprzyk <onestone@compiz-fusion.org>
 */

#include "privatewindow.h"
#include "core/window.h"


CompWindow::Geometry &
CompWindow::serverGeometry () const
{
    return priv->attrib.override_redirect ? priv->geometry : priv->serverGeometry;
}

CompWindow::Geometry &
CompWindow::geometry () const
{
    return priv->attrib.override_redirect ? priv->geometry : priv->serverGeometry;
}

int
CompWindow::x () const
{
    return geometry ().x ();
}

int
CompWindow::y () const
{
    return geometry ().y ();
}

CompPoint
CompWindow::pos () const
{
    return CompPoint (geometry ().x (), geometry ().y ());
}

/* With border */
int
CompWindow::width () const
{
    return geometry ().widthIncBorders ();
}

int
CompWindow::height () const
{
    return geometry ().heightIncBorders ();
}

CompSize
CompWindow::size () const
{
    return CompSize (width (), height ());
}

int
CompWindow::serverX () const
{
    return serverGeometry ().x ();
}

int
CompWindow::serverY () const
{
    return serverGeometry ().y ();
}

CompPoint
CompWindow::serverPos () const
{
    return CompPoint (serverGeometry ().x (),
		      serverGeometry ().y ());
}

/* With border */
int
CompWindow::serverWidth () const
{
    return serverGeometry ().widthIncBorders ();
}

int
CompWindow::serverHeight () const
{
    return serverGeometry ().heightIncBorders ();
}

const CompSize
CompWindow::serverSize () const
{
    return CompSize (serverGeometry ().widthIncBorders (),
		     serverGeometry ().heightIncBorders ());
}

CompRect
CompWindow::borderRect () const
{
    return CompRect (geometry ().xMinusBorder () - priv->border.left,
		     geometry ().yMinusBorder () - priv->border.top,
		     geometry ().widthIncBorders () +
		     priv->border.left + priv->border.right,
		     geometry ().heightIncBorders () +
		     priv->border.top + priv->border.bottom);
}

CompRect
CompWindow::serverBorderRect () const
{
    return CompRect (serverGeometry ().xMinusBorder () - priv->border.left,
		     serverGeometry ().yMinusBorder () - priv->border.top,
		     serverGeometry ().widthIncBorders () +
		     priv->border.left + priv->border.right,
		     serverGeometry ().heightIncBorders() +
		     priv->border.top + priv->border.bottom);
}

CompRect
CompWindow::inputRect () const
{
    return CompRect (geometry ().xMinusBorder () - priv->serverInput.left,
		     geometry ().yMinusBorder () - priv->serverInput.top,
		     geometry ().widthIncBorders () +
		     priv->serverInput.left + priv->serverInput.right,
		     geometry ().heightIncBorders () +
		     priv->serverInput.top + priv->serverInput.bottom);
}

CompRect
CompWindow::serverInputRect () const
{
    return CompRect (serverGeometry ().xMinusBorder () - priv->serverInput.left,
		     serverGeometry ().yMinusBorder () - priv->serverInput.top,
		     serverGeometry ().widthIncBorders () +
		     priv->serverInput.left + priv->serverInput.right,
		     serverGeometry ().heightIncBorders () +
		     priv->serverInput.top + priv->serverInput.bottom);
}

CompRect
CompWindow::outputRect () const
{
    return CompRect (geometry ().xMinusBorder ()- priv->output.left,
		     geometry ().yMinusBorder () - priv->output.top,
		     geometry ().widthIncBorders () +
		     priv->output.left + priv->output.right,
		     geometry ().heightIncBorders () +
		     priv->output.top + priv->output.bottom);
}

CompRect
CompWindow::serverOutputRect () const
{
    return CompRect (serverGeometry ().xMinusBorder () -  priv->output.left,
		     serverGeometry ().yMinusBorder () - priv->output.top,
		     serverGeometry ().widthIncBorders () +
		     priv->output.left + priv->output.right,
		     serverGeometry ().heightIncBorders () +
		     priv->output.top + priv->output.bottom);
}
