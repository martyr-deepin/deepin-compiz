/*
 * text.h - adds text image support to compiz.
 * Copyright: (C) 2006 Patrick Niklaus
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 */

#ifndef _COMPIZ_TEXT_H
#define _COMPIZ_TEXT_H

#define COMPIZ_TEXT_ABI 20090905

class CompText
{
    public:
	/**
	 * Flags to be passed into the flags field of CompTextAttrib
	 */
	typedef enum {
	    StyleBold      = (1 << 0), /**< render the text in bold */
	    StyleItalic    = (1 << 1), /**< render the text italic */
	    Ellipsized     = (1 << 2), /**< ellipsize the text if the
					    specified maximum size is
					    too small */
	    WithBackground = (1 << 3), /**< render a rounded rectangle as
					    background behind the text */
	    NoAutoBinding  = (1 << 4) /**< do not automatically bind the
					   rendered text pixmap to a texture */
	} Flags;

	typedef struct {
	    const char     *family;    /**< font family */
	    int            size;       /**< font size in points */
	    unsigned short color[4];   /**< font color (RGBA) */

	    unsigned int   flags;      /**< rendering flags, see above */

	    int            maxWidth;   /**< maximum width of the
					    generated pixmap */
	    int            maxHeight;  /**< maximum height of the
					    generated pixmap */

	    int            bgHMargin;  /**< horizontal margin in pixels
		     			    (offset of text into background) */
	    int            bgVMargin;  /**< vertical margin */
	    unsigned short bgColor[4]; /**< background color (RGBA) */
	} Attrib;

	CompText ();
	~CompText();

	bool renderText (CompString   text,
			 const Attrib &attrib);


	bool renderWindowTitle (Window       window,
				bool         renderViewportNumber,
				const Attrib &attrib);

	void clear ();

	Pixmap getPixmap ();
	int getWidth () const;
	int getHeight () const;

	void draw (const GLMatrix &transform,
	           float x,
		   float y,
		   float alpha) const;

    private:
	int width;
	int height;

	Pixmap          pixmap;
	GLTexture::List texture;
};

#endif
