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

#ifndef COMPIZ_IMGPGN_H
#define COMPIZ_IMGPGN_H

#include <core/screen.h>
#include <core/string.h>
#include <core/pluginclasshandler.h>

#include <png.h>

#include <iosfwd>

extern const unsigned short PNG_SIG_SIZE;

class PngScreen :
    public ScreenInterface,
    public PluginClassHandler<PngScreen, CompScreen>
{
    public:
	PngScreen (CompScreen *screen);
	~PngScreen ();

	bool fileToImage (CompString &path, CompSize &size,
			  int &stride, void *&data);
	bool imageToFile (CompString &path, CompString &format,
			  CompSize &size, int stride, void *data);

    private:
	CompString fileNameWithExtension (CompString &path);

	bool readPngData (png_struct *png, png_info *info,
			  void *&data, CompSize &size);
	bool readPng (std::ifstream &file, CompSize &size, void *& data);
	bool writePng (unsigned char *buffer, std::ostream &file,
		       CompSize &size, int stride);
};

class PngPluginVTable :
    public CompPlugin::VTableForScreen<PngScreen>
{
    public:
	bool init ();
};

#endif
