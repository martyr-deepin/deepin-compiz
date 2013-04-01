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

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#include <core/core.h>
#include <core/pluginclasshandler.h>

#include <X11/Xarch.h>
#include <jpeglib.h>

#include "imgjpeg_options.h"



struct jpegErrorMgr
{
    struct  jpeg_error_mgr pub;	/* "public" fields */
    jmp_buf setjmp_buffer;	/* for return to caller */
};

class JpegScreen :
    public ScreenInterface,
    public PluginClassHandler<JpegScreen, CompScreen>,
    public ImgjpegOptions
{
    public:
	JpegScreen (CompScreen *screen);

	bool fileToImage (CompString &path, CompSize &size,
			  int &stride, void *&data);
	bool imageToFile (CompString &path, CompString &format,
			  CompSize &size, int stride, void *data);

    private:
	CompString fileNameWithExtension (CompString &path);

	bool readJPEG (FILE *file, CompSize &size, void *&data);
	bool writeJPEG (unsigned char *buffer, FILE *file,
			CompSize &size, int stride);
};

class JpegPluginVTable :
    public CompPlugin::VTableForScreen<JpegScreen>
{
    public:
	bool init ();
};

