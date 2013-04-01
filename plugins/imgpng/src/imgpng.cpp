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

#include "imgpng.h"

#include "core/abiversion.h"

#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>


COMPIZ_PLUGIN_20090315 (imgpng, PngPluginVTable)

const unsigned short PNG_SIG_SIZE = 8;

PngScreen::PngScreen (CompScreen *screen) :
    PluginClassHandler<PngScreen, CompScreen> (screen)
{
    ScreenInterface::setHandler (screen, true);

    screen->updateDefaultIcon ();
}

PngScreen::~PngScreen ()
{
    screen->updateDefaultIcon ();
}

static void
premultiplyData (png_structp   png,
		 png_row_infop row_info,
		 png_bytep     data)
{
    unsigned int i;

    for (i = 0; i < row_info->rowbytes; i += 4)
    {
	unsigned char *base = &data[i];
	unsigned char blue  = base[0];
	unsigned char green = base[1];
	unsigned char red   = base[2];
	unsigned char alpha = base[3];
	int	      p;

	red   = (unsigned) red   * (unsigned) alpha / 255;
	green = (unsigned) green * (unsigned) alpha / 255;
	blue  = (unsigned) blue  * (unsigned) alpha / 255;

	p = (alpha << 24) | (red << 16) | (green << 8) | (blue << 0);
	memcpy (base, &p, sizeof (int));
    }
}

bool
PngScreen::readPngData (png_struct *png,
			png_info   *info,
			void	   *&data,
			CompSize   &size)
{
    png_uint_32	 pngWidth, pngHeight;
    int		 depth, colorType, interlace;
    unsigned int pixelSize, i;
    png_byte	 **rowPointers;
    char	 *d;

    png_read_info (png, info);

    png_get_IHDR (png, info,
		  &pngWidth, &pngHeight, &depth,
		  &colorType, &interlace, NULL, NULL);

    size.setWidth (pngWidth);
    size.setHeight (pngHeight);

    /* convert palette/gray image to rgb */
    if (colorType == PNG_COLOR_TYPE_PALETTE)
	png_set_palette_to_rgb (png);

    /* expand gray bit depth if needed */
    if (colorType == PNG_COLOR_TYPE_GRAY && depth < 8)
	png_set_expand_gray_1_2_4_to_8 (png);

    /* transform transparency to alpha */
    if (png_get_valid (png, info, PNG_INFO_tRNS))
	png_set_tRNS_to_alpha (png);

    if (depth == 16)
	png_set_strip_16 (png);

    if (depth < 8)
	png_set_packing (png);

    /* convert grayscale to RGB */
    if (colorType == PNG_COLOR_TYPE_GRAY ||
	colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
	png_set_gray_to_rgb (png);

    if (interlace != PNG_INTERLACE_NONE)
	png_set_interlace_handling (png);

    png_set_bgr (png);
    png_set_filler (png, 0xff, PNG_FILLER_AFTER);

    png_set_read_user_transform_fn (png, premultiplyData);

    png_read_update_info (png, info);

    pixelSize = 4;
    d = (char *) malloc (pngWidth * pngHeight * pixelSize);
    if (!d)
	return false;

    data = d;

    rowPointers = new png_byte *[pngHeight];
    if (!rowPointers)
    {
	free (d);
	return false;
    }

    for (i = 0; i < pngHeight; i++)
	rowPointers[i] = (png_byte *) (d + i * pngWidth * pixelSize);

    png_read_image (png, rowPointers);
    png_read_end (png, info);

    delete [] rowPointers;

    return true;
}

static void
stdioReadFunc (png_structp png,
	       png_bytep   data,
	       png_size_t  size)
{
    std::ifstream *file = (std::ifstream *) png_get_io_ptr (png);

    file->read ((char *) data, size);
    if (file->fail ())
	png_error (png, "Read Error");
}

bool
PngScreen::readPng (std::ifstream &file,
		    CompSize      &size,
		    void          *&data)
{
    unsigned char png_sig[PNG_SIG_SIZE];
    png_struct	  *png;
    png_info	  *info;
    bool	  status;

    file.read ((char *) png_sig, PNG_SIG_SIZE);
    if (file.fail ())
	return false;
    if (png_sig_cmp (png_sig, 0, PNG_SIG_SIZE) != 0)
	return false;

    png = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png)
	return false;

    info = png_create_info_struct (png);
    if (!info)
    {
	png_destroy_read_struct (&png, NULL, NULL);
	return false;
    }

    png_set_read_fn (png, &file, stdioReadFunc);
    png_set_sig_bytes (png, PNG_SIG_SIZE);

    status = readPngData (png, info, data, size);

    png_destroy_read_struct (&png, &info, NULL);

    return status;
}

static void
stdioWriteFunc (png_structp png,
		png_bytep   data,
		png_size_t  size)
{
    std::ofstream *file = (std::ofstream *) png_get_io_ptr (png);

    file->write ((char *) data, size);
    if (file->bad ())
	png_error (png, "Write Error");
}

bool
PngScreen::writePng (unsigned char *buffer,
		     std::ostream  &file,
		     CompSize      &size,
		     int           stride)
{
    png_struct	 *png;
    png_info	 *info;
    png_byte	 **rows;
    png_color_16 white;
    int		 i, height = size.height ();

    rows = new png_byte *[height];
    if (!rows)
	return false;

    for (i = 0; i < height; i++)
	rows[height - i - 1] = buffer + i * stride;

    png = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png)
    {
	delete [] rows;
	return false;
    }

    info = png_create_info_struct (png);
    if (!info)
    {
	png_destroy_write_struct (&png, NULL);
	delete [] rows;
	return false;
    }

    if (setjmp (png_jmpbuf (png)))
    {
	png_destroy_write_struct (&png, NULL);
	delete [] rows;
	return false;
    }

    png_set_write_fn (png, &file, stdioWriteFunc, NULL);

    png_set_IHDR (png, info,
		  size.width (), size.height (), 8,
		  PNG_COLOR_TYPE_RGB_ALPHA,
		  PNG_INTERLACE_NONE,
		  PNG_COMPRESSION_TYPE_DEFAULT,
		  PNG_FILTER_TYPE_DEFAULT);

    white.red   = 0xff;
    white.blue  = 0xff;
    white.green = 0xff;

    png_set_bKGD (png, info, &white);

    png_write_info (png, info);
    png_write_image (png, rows);
    png_write_end (png, info);

    png_destroy_write_struct (&png, &info);
    delete [] rows;

    return true;
}

CompString
PngScreen::fileNameWithExtension (CompString &path)
{
    unsigned int len = path.length ();

    if (len > 4 && path.substr (len - 4, 4) == ".png")
	return path;

    return path + ".png";
}

bool
PngScreen::imageToFile (CompString &path,
			CompString &format,
			CompSize   &size,
			int	   stride,
			void	   *data)
{
    bool          status = false;
    std::ofstream file;
    CompString    fileName = fileNameWithExtension (path);

    if (format == "png")
    {
	file.open (fileName.c_str ());
	if (file.is_open ())
	{
	    status = writePng ((unsigned char *) data, file, size, stride);
	    file.close ();
	}

	if (status)
	    return true;
    }

    status = screen->imageToFile (path, format, size, stride, data);

    if (!status)
    {
	file.open (fileName.c_str ());
	if (file.is_open ())
	{
	    status = writePng ((unsigned char *) data, file, size, stride);
	    file.close ();
	}
    }

    return status;
}

bool
PngScreen::fileToImage (CompString &name,
			CompSize   &size,
			int        &stride,
			void       *&data)
{
    bool          status = false;
    std::ifstream file;
    CompString    fileName = fileNameWithExtension (name);

    file.open (fileName.c_str ());
    if (file.is_open ())
    {
	status = readPng (file, size, data);
	file.close ();
    }

    if (status)
    {
	stride = size.width () * 4;
	return true;
    }

    return screen->fileToImage (name, size, stride, data);
}

bool
PngPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
	return false;

    return true;
}

