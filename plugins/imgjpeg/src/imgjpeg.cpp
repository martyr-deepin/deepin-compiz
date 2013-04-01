/*
 * beryl-plugins::jpeg.c - adds JPEG image support to beryl.
 * Copyright: (C) 2006 Nicholas Thomas
 *		       Danny Baumann (JPEG writing, option stuff)
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "imgjpeg.h"

COMPIZ_PLUGIN_20090315 (imgjpeg, JpegPluginVTable)

static bool
rgbToBGRA (const JSAMPLE *source,
	   void          *&data,
	   CompSize      &size,
	   int           alpha)
{
    int  h, w;
    char *dest;
    int  height = size.height ();
    int  width = size.width ();

    dest = (char *) malloc ((unsigned)(height * width * 4));
    if (!dest)
	return false;

    data = dest;

    for (h = 0; h < height; h++)
	for (w = 0; w < width; w++)
	{
	    int pos = h * width + w;
#if __BYTE_ORDER == __BIG_ENDIAN
	    dest[(pos * 4) + 3] = source[(pos * 3) + 2];    /* blue */
	    dest[(pos * 4) + 2] = source[(pos * 3) + 1];    /* green */
	    dest[(pos * 4) + 1] = source[(pos * 3) + 0];    /* red */
	    dest[(pos * 4) + 0] = alpha;
#else
	    dest[(pos * 4) + 0] = (char)source[(pos * 3) + 2];    /* blue */
	    dest[(pos * 4) + 1] = (char)source[(pos * 3) + 1];    /* green */
	    dest[(pos * 4) + 2] = (char)source[(pos * 3) + 0];    /* red */
	    dest[(pos * 4) + 3] = alpha;
#endif
	}

    return true;
}

static bool
rgbaToRGB (unsigned char *source,
	   JSAMPLE       **dest,
	   CompSize      &size,
	   int           stride)
{
    int     h, w;
    int     height = size.height ();
    int     width = size.width ();
    int     ps = stride / width;	/* pixel size */
    JSAMPLE *d;

    d = (JSAMPLE *) malloc ((unsigned)height * (unsigned)width * 3 *
			    sizeof (JSAMPLE));
    if (!d)
	return false;

    *dest = d;

    for (h = 0; h < height; h++)
	for (w = 0; w < width; w++)
	{
	    int pos = h * width + w;
#if __BYTE_ORDER == __BIG_ENDIAN
	    d[(pos * 3) + 0] = source[(pos * ps) + 3];	/* red */
    	    d[(pos * 3) + 1] = source[(pos * ps) + 2];	/* green */
    	    d[(pos * 3) + 2] = source[(pos * ps) + 1];	/* blue */
#else
	    d[(pos * 3) + 0] = source[(pos * ps) + 0];	/* red */
    	    d[(pos * 3) + 1] = source[(pos * ps) + 1];	/* green */
    	    d[(pos * 3) + 2] = source[(pos * ps) + 2];	/* blue */
#endif
	}

    return true;
}

static void
jpegErrorExit (j_common_ptr cinfo)
{
    char                buffer[JMSG_LENGTH_MAX];
    struct jpegErrorMgr *err = (struct jpegErrorMgr *) cinfo->err;

    /* Format the message */
    (*cinfo->err->format_message) (cinfo, buffer);

    printf ("%s\n", buffer);

    /* Return control to the setjmp point */
    longjmp (err->setjmp_buffer, 1);
}

bool
JpegScreen::readJPEG (FILE     *file,
		      CompSize &size,
		      void     *&data)
{
    struct jpeg_decompress_struct cinfo;
    struct jpegErrorMgr           jerr;
    JSAMPLE                       *buf;
    JSAMPROW                      *rows;
    bool                          result;

    if (!file)
	return false;

    cinfo.err = jpeg_std_error (&jerr.pub);
    jerr.pub.error_exit = jpegErrorExit;

    if (setjmp (jerr.setjmp_buffer))
    {
	/* this is called on decompression errors */
	jpeg_destroy_decompress (&cinfo);
	return false;
    }

    jpeg_create_decompress (&cinfo);

    jpeg_stdio_src (&cinfo, file);

    jpeg_read_header (&cinfo, true);

    cinfo.out_color_space = JCS_RGB;

    jpeg_start_decompress (&cinfo);

    size.setHeight ((int)cinfo.output_height);
    size.setWidth ((int)cinfo.output_width);

    buf = (JSAMPLE *) calloc (cinfo.output_height * cinfo.output_width *
			      (unsigned)cinfo.output_components,
			      sizeof (JSAMPLE));
    if (!buf)
    {
	jpeg_finish_decompress (&cinfo);
	jpeg_destroy_decompress (&cinfo);
	return false;
    }

    rows = (JSAMPROW *) malloc (cinfo.output_height * sizeof (JSAMPROW));
    if (!rows)
    {
	free (buf);
	jpeg_finish_decompress (&cinfo);
	jpeg_destroy_decompress (&cinfo);
	return false;
    }

    for (unsigned int i = 0; i < cinfo.output_height; i++)
	rows[i] = &buf[i * cinfo.output_width *
		       (unsigned)cinfo.output_components];

    while (cinfo.output_scanline < cinfo.output_height)
	jpeg_read_scanlines (&cinfo, &rows[cinfo.output_scanline],
			     cinfo.output_height - cinfo.output_scanline);

    jpeg_finish_decompress (&cinfo);
    jpeg_destroy_decompress (&cinfo);

    /* convert the rgb data into BGRA format */
    result = rgbToBGRA (buf, data, size, 255);

    free (rows);
    free (buf);
    return result;
}

bool
JpegScreen::writeJPEG (unsigned char *buffer,
		       FILE          *file,
		       CompSize      &size,
		       int           stride)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr       jerr;
    JSAMPROW                    row_pointer[1];
    JSAMPLE                     *data;

    /* convert the rgb data into BGRA format */
    if (!rgbaToRGB (buffer, &data, size, stride))
	return false;

    cinfo.err = jpeg_std_error (&jerr);
    jpeg_create_compress (&cinfo);

    jpeg_stdio_dest (&cinfo, file);

    cinfo.image_width      = (unsigned) size.width ();
    cinfo.image_height     = (unsigned) size.height ();
    cinfo.input_components = 3;
    cinfo.in_color_space   = JCS_RGB;

    jpeg_set_defaults (&cinfo);
    jpeg_set_quality (&cinfo, optionGetQuality (), true);
    jpeg_start_compress (&cinfo, true);

    while (cinfo.next_scanline < cinfo.image_height)
    {
	row_pointer[0] =
	    &data[(cinfo.image_height - cinfo.next_scanline - 1) *
		  (unsigned) size.width () * 3];
	jpeg_write_scanlines (&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress (&cinfo);
    jpeg_destroy_compress (&cinfo);

    free (data);

    return true;
}

CompString
JpegScreen::fileNameWithExtension (CompString &path)
{
    unsigned int len = path.length ();

    if ((len > 5 && path.substr (len - 5, 5) == ".jpeg") ||
    	(len > 4 && path.substr (len - 4, 4) == ".jpg"))
	return path;

    return path + ".jpeg";
}

bool
JpegScreen::imageToFile (CompString &path,
			 CompString &format,
			 CompSize   &size,
			 int	   stride,
			 void	   *data)
{
    bool       status = false;
    FILE       *file;
    CompString fileName = fileNameWithExtension (path);

    if (format == "jpeg" || format == "jpg" ||
    	!(status = screen->imageToFile (path, format, size, stride, data)))
    {
    	file = fopen (fileName.c_str (), "wb");
    	if (file)
	{
	    status = writeJPEG ((unsigned char *) data, file, size, stride);
	    fclose (file);
	}
    }

    return status;
}

bool
JpegScreen::fileToImage (CompString &name,
			 CompSize   &size,
			 int        &stride,
			 void       *&data)
{
    bool       status = false;
    FILE       *file;
    CompString fileName = fileNameWithExtension (name);

    file = fopen (fileName.c_str (), "rb");
    if (file)
    {
	status = readJPEG (file, size, data);
	fclose (file);
    }

    if (status)
    {
	stride = size.width () * 4;
	return true;
    }

    /* Isn't a JPEG - pass to the next in the chain. */
    return screen->fileToImage (name, size, stride, data);
}

JpegScreen::JpegScreen (CompScreen *screen) :
    PluginClassHandler<JpegScreen, CompScreen> (screen)
{
    ScreenInterface::setHandler (screen, true);
}

bool
JpegPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
	return false;

    return true;
}

